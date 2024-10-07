#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <time.h>

// Define the Result structure to store algorithm results
typedef struct
{
    int bestCost;
    int worstCost;
    double averageCost;
    int *bestSolution;
    int bestSolutionSize;
    int *worstSolution;
    int worstSolutionSize;
} Result;

// Function to calculate the cost of a given solution
int calculate_cost(const int *solution, int solution_size, const int **distances, const int *costs);

// Function to calculate cost breakdown into path length and node costs
void calculate_cost_breakdown(const int *solution, int solution_size, const int **distances, const int *costs, int *path_length, int *node_costs);

// Fisher-Yates shuffle algorithm to shuffle an array
void shuffle_array(int *array, int n);

// Function to read a CSV file with ';' delimiter
// Assumes each line has at least three integers: x, y, cost
int **read_file(const char *filename, int *num_nodes);

// Function to calculate Euclidean distances rounded to nearest integer
int **calcDistances(int **data, int num_nodes);

// Function to check if a solution is valid (nodes are unique and within bounds)
int is_valid_solution(const int *solution, int solution_size, int num_nodes);

// Function to write a solution vector to a file
void write_solution_to_file(const int *sol, int sol_size, const char *filename);

// Function to write Result to file
void write_Result_to_file(const Result res, const char *filename, const int **distances, const int *costs);

// Function to free data arrays
void free_data(int **data, int num_nodes);

// Function to free distances matrix
void free_distances(int **distances, int num_nodes);

#endif
