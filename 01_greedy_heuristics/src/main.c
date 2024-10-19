#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "algorithms.h"
#include "utils.h"

int main(int argc, char* argv[]) {
    // Initialize random seed
    srand((unsigned int)time(NULL));

    // Create algorithm instances
    int num_algorithms = 6;
    Algo* algorithms[6];
    algorithms[0] = (Algo*)create_RandomSearch();
    algorithms[1] = (Algo*)create_NearestNeighboursEndInsert();
    algorithms[2] = (Algo*)create_NearestNeighboursAnywhereInsert();
    algorithms[3] = (Algo*)create_GreedyCycle();
    algorithms[4] = (Algo*)create_Greedy2Regret();
    algorithms[5] = (Algo*)create_Greedy2RegretWeighted();

    // Check if any algorithm failed to be created
    for(int a = 0; a < num_algorithms; a++) {
        if(algorithms[a] == NULL) {
            fprintf(stderr, "Error: Failed to create algorithm %d\n", a);
            // Optionally, handle error
        }
    }

    // List of files to process (only TSPA and TSPB)
    const char* files[] = {"data/TSPA.csv", "data/TSPB.csv"};
    int num_files = sizeof(files) / sizeof(files[0]);

    // Number of solutions to generate per starting node
    int num_solutions = 200;

    // Loop over algorithms and files
    for(int a = 0; a < num_algorithms; a++) {
        for(int f = 0; f < num_files; f++) {
            // Read data from file
            int num_nodes = 0;
            int** data = read_file(files[f], &num_nodes);
            if(!data || num_nodes == 0) {
                fprintf(stderr, "Error: No data found in file %s\n", files[f]);
                continue;
            }

            // Calculate distances
            int** distances = calcDistances(data, num_nodes);
            if(!distances) {
                fprintf(stderr, "Error: Failed to calculate distances for file %s\n", files[f]);
                free_data(data, num_nodes);
                continue;
            }

            // Extract costs
            int* costs = (int*)malloc(num_nodes * sizeof(int));
            if(!costs) {
                fprintf(stderr, "Error: Memory allocation failed for costs in file %s\n", files[f]);
                free_data(data, num_nodes);
                free_distances(distances, num_nodes);
                continue;
            }
            for(int i = 0; i < num_nodes; i++) {
                costs[i] = data[i][2];
            }

            // Print algorithm and file information
            printf("# Algorithm: %s\n", algorithms[a]->name);
            printf("## File: %s\n", files[f]);

            // Solve using the current algorithm
            Result res = algorithms[a]->solve(algorithms[a], (const int**)distances, num_nodes, costs, num_solutions);

            // Print and write results
            print_Result(res, (const int**)distances, costs);
            char result_filename[256];

            char base_name[256];
            const char* temp_base_filename = strrchr(files[f], '/');
            if (temp_base_filename) {
                temp_base_filename++;  // Skip the '/'
            } else {
                temp_base_filename = files[f];  // No '/' found, use the full file name
            }

            // Copy base filename to base_name
            strncpy(base_name, temp_base_filename, sizeof(base_name));
            base_name[sizeof(base_name)-1] = '\0'; // Ensure null termination

            // Remove file extension
            char* dot = strrchr(base_name, '.');
            if (dot) {
                *dot = '\0';
            }

            snprintf(result_filename, sizeof(result_filename), "%s_%s_result.txt", algorithms[a]->name, base_name);
            write_Result_to_file(res, result_filename, (const int**)distances, costs);
            printf("----------------------------------------\n");

            // Free allocated memory for this file
            free_Result(res);
            free(costs);
            free_data(data, num_nodes);
            free_distances(distances, num_nodes);
        }
    }

    // Free algorithm instances
    for(int i = 0; i < num_algorithms; i++) {
        free(algorithms[i]);
    }

    return 0;
}
