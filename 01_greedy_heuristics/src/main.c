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
#include <time.h>

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

// Function to generate an initial solution
// This example generates a random solution by selecting 'solution_size' unique nodes
int* generate_initial_solution(int num_nodes, int solution_size) {
    if (solution_size > num_nodes) {
        fprintf(stderr, "Error: Solution size cannot exceed the number of nodes.\n");
        return NULL;
    }

    int* all_nodes = (int*)malloc(num_nodes * sizeof(int));
    if (!all_nodes) {
        fprintf(stderr, "Error: Memory allocation failed for all_nodes.\n");
        return NULL;
    }

    for (int i = 0; i < num_nodes; i++) {
        all_nodes[i] = i;
    }

    // Shuffle the array
    shuffle_array(all_nodes, num_nodes);

    // Select the first 'solution_size' nodes as the initial solution
    int* initial_solution = (int*)malloc(solution_size * sizeof(int));
    if (!initial_solution) {
        fprintf(stderr, "Error: Memory allocation failed for initial_solution.\n");
        free(all_nodes);
        return NULL;
    }

    memcpy(initial_solution, all_nodes, solution_size * sizeof(int));
    free(all_nodes);

    return initial_solution;
}

int main(int argc, char* argv[]) {
    // Initialize random seed
    srand((unsigned int)time(NULL));

    // Define the number of algorithms excluding DeltaLocalSearch
    int num_other_algorithms = 2; // MSLS and ILS
    Algo* algorithms[num_other_algorithms];

    // Initialize Multiple Start Local Search
    algorithms[0] = (Algo*)create_MSLS(200); // 200 iterations as per requirement

    // Initialize Iterated Local Search
    // Assuming average running time of MSLS is approximated as 200 iterations
    // Set max_time_ms accordingly, e.g., 200 * average time per iteration
    // For simplicity, set a fixed time or adjust as needed
    algorithms[1] = (Algo*)create_ILS(10000, 5); // 10 seconds and perturbation strength of 5

    // List of files to process
    const char* files[] = {"data/TSPA.csv", "data/TSPB.csv"};
    int num_files = sizeof(files) / sizeof(files[0]);

    // Number of solutions to generate per method
    int num_solutions_delta = 200;    // For DeltaLocalSearch
    int num_solutions_msls = 200;     // For MSLS
    // ILS uses time-based stopping condition

    // Loop over files
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

        // Generate initial solution
        int solution_size = (num_nodes + 1) / 2; // Example: selecting 50% of the nodes
        int* initial_solution = generate_initial_solution(num_nodes, solution_size);
        if (!initial_solution) {
            fprintf(stderr, "Error: Failed to generate initial solution for file %s\n", files[f]);
            free(costs);
            free_data(data, num_nodes);
            free_distances(distances, num_nodes);
            continue;
        }

        // Create a temporary DeltaLocalSearch instance with the initial solution
        DeltaLocalSearch* dls = create_DeltaLocalSearch(0, initial_solution, solution_size);
        if (!dls) {
            fprintf(stderr, "Error: Failed to create DeltaLocalSearch for file %s\n", files[f]);
            free(initial_solution);
            free(costs);
            free_data(data, num_nodes);
            free_distances(distances, num_nodes);
            continue;
        }

        // Add DeltaLocalSearch to the list of algorithms for this iteration
        // Since algorithms array was initially defined for other algorithms,
        // we'll temporarily expand it to include DeltaLocalSearch
        int current_num_algorithms = num_other_algorithms + 1;
        Algo* current_algorithms[current_num_algorithms];
        current_algorithms[0] = (Algo*)dls; // DeltaLocalSearch
        current_algorithms[1] = algorithms[0]; // MSLS
        current_algorithms[2] = algorithms[1]; // ILS

        // Print algorithm and file information
        for(int a = 0; a < current_num_algorithms; a++) {
            // Print algorithm and file information
            printf("# Algorithm: %s\n", current_algorithms[a]->name);
            printf("## File: %s\n", files[f]);

            // Determine number of solutions based on algorithm
            int num_solutions = 0;
            if(strcmp(current_algorithms[a]->name, "MultipleStartLocalSearch") == 0) {
                num_solutions = num_solutions_msls;
            }
            else if(strcmp(current_algorithms[a]->name, "IteratedLocalSearch") == 0) {
                // ILS uses time-based stopping condition, num_solutions can be set to 0 or ignored
                num_solutions = 0;
            }
            else { // Assuming it's DeltaLocalSearch
                num_solutions = num_solutions_delta;
            }

            // Start timing before the solve function
            struct timespec start_time, end_time;
            if(clock_gettime(CLOCK_MONOTONIC, &start_time) != 0) {
                perror("clock_gettime");
                // Handle error, possibly continue without timing
            }

            // Solve using the current algorithm
            Result res = current_algorithms[a]->solve(current_algorithms[a], (const int**)distances, num_nodes, costs, num_solutions);

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

            snprintf(result_filename, sizeof(result_filename), "%s_%s_result.txt", current_algorithms[a]->name, base_name);
            write_Result_to_file(res, result_filename, (const int**)distances, costs);
            printf("Time taken by %s on %s: %.3f ms (%.3f seconds)\n",
                   current_algorithms[a]->name, files[f], elapsed_ms, elapsed_sec);
            printf("----------------------------------------\n");

            // Free allocated memory for the result
            free_Result(res);
        }

        // Free the temporary DeltaLocalSearch instance
        free(dls->base.name);
        free(dls);

        // Free initial_solution as it's copied inside DeltaLocalSearch
        free(initial_solution);

        // Free allocated memory for this file
        free(costs);
        free_data(data, num_nodes);
        free_distances(distances, num_nodes);
    }

    // Free other algorithm instances
    for(int i = 0; i < num_other_algorithms; i++) {
        free((void*)algorithms[i]->name);
        free(algorithms[i]);
    }

    return 0;
}
