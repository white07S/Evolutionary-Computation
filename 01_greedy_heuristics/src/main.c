#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "algorithms.h"
#include "utils.h"
#include "local_search.h"
#include "delta_local_search.h"
#include "msls.h"        // Include MSLS header
#include "ils.h"         // Include ILS header
#include <linux/time.h>

// Function to calculate elapsed time in milliseconds
double get_elapsed_time_ms(struct timespec start, struct timespec end) {
    double start_ms = start.tv_sec * 1000.0 + start.tv_nsec / 1.0e6;
    double end_ms = end.tv_sec * 1000.0 + end.tv_nsec / 1.0e6;
    return end_ms - start_ms;
}

// Function to calculate elapsed time in seconds
double get_elapsed_time_sec(struct timespec start, struct timespec end) {
    double start_sec = start.tv_sec + start.tv_nsec / 1.0e9;
    double end_sec = end.tv_sec + end.tv_nsec / 1.0e9;
    return end_sec - start_sec;
}

int main(int argc, char* argv[]) {
    // Initialize random seed
    srand((unsigned int)time(NULL));

    // Create algorithm instances
    // Adjust the number of algorithms and their initialization as needed
    int num_algorithms = 3; // Example: RandomSearch, MSLS, ILS
    Algo* algorithms[3];

    // Initialize DeltaLocalSearch as MSLS
    algorithms[0] = (Algo*)create_DeltaLocalSearch(0); // Existing DeltaLocalSearch
    // Initialize Multiple Start Local Search
    algorithms[1] = (Algo*)create_MSLS(200); // 200 iterations as per requirement
    // Initialize Iterated Local Search
    // Assuming average running time of MSLS is approximated as 200 iterations
    // Set max_time_ms accordingly, e.g., 200 * average time per iteration
    // For simplicity, set a fixed time or adjust as needed
    algorithms[2] = (Algo*)create_ILS(10000, 5); // 10 seconds and perturbation strength of 5

    // List of files to process
    const char* files[] = {"data/TSPA.csv", "data/TSPB.csv"};
    int num_files = sizeof(files) / sizeof(files[0]);

    // Number of solutions to generate per method
    int num_solutions_delta = 200;    // For DeltaLocalSearch
    int num_solutions_msls = 200;     // For MSLS
    // ILS uses time-based stopping condition

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

            // Determine number of solutions based on algorithm
            int num_solutions = 0;
            if(strcmp(algorithms[a]->name, "MultipleStartLocalSearch") == 0) {
                num_solutions = num_solutions_msls;
            }
            else if(strcmp(algorithms[a]->name, "IteratedLocalSearch") == 0) {
                // ILS uses time-based stopping condition, num_solutions can be set to 0 or ignored
                num_solutions = 0;
            }
            else {
                num_solutions = num_solutions_delta;
            }

            // Start timing before the solve function
            struct timespec start_time, end_time;
            if(clock_gettime(CLOCK_MONOTONIC, &start_time) != 0) {
                perror("clock_gettime");
                // Handle error, possibly continue without timing
            }

            // Solve using the current algorithm
            Result res = algorithms[a]->solve(algorithms[a], (const int**)distances, num_nodes, costs, num_solutions);

            // End timing after the solve function
            if(clock_gettime(CLOCK_MONOTONIC, &end_time) != 0) {
                perror("clock_gettime");
                // Handle error, possibly set elapsed_time to -1
            }

            // Calculate elapsed time in milliseconds and seconds
            double elapsed_ms = get_elapsed_time_ms(start_time, end_time);
            double elapsed_sec = get_elapsed_time_sec(start_time, end_time);

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
            printf("Time taken by %s on %s: %.3f ms (%.3f seconds)\n",
                   algorithms[a]->name, files[f], elapsed_ms, elapsed_sec);
            printf("----------------------------------------\n");

            // Free allocated memory for this file
            free_Result(res);
            free(costs);
            free_data(data, num_nodes);
            free_distances(distances, num_nodes);
        }
    }

    // Print the summary table
    // (Implementation depends on how you want to store and display results)

    // Free algorithm instances
    for(int i = 0; i < num_algorithms; i++) {
        free((void*)algorithms[i]->name);
        free(algorithms[i]);
    }

    return 0;
}
