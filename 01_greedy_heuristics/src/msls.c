#include "msls.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>

// Include necessary headers for local search
#include "local_search.h"
#include "utils.h"

// Forward declaration of the solve function
static Result MSLS_solve(Algo *algo, const int **distances, int num_nodes, const int *costs, int num_solutions);

/**
 * @brief Creates an instance of the MSLS algorithm.
 *
 * @param num_iterations Number of local search iterations to perform.
 * @return Pointer to the created MSLS instance.
 */
MSLS* create_MSLS(int num_iterations)
{
    MSLS* msls = (MSLS*)malloc(sizeof(MSLS));
    if (!msls)
    {
        fprintf(stderr, "Error: Memory allocation failed for MSLS\n");
        return NULL;
    }

    char* name = (char*)malloc(100 * sizeof(char));
    if (!name)
    {
        fprintf(stderr, "Error: Memory allocation failed for MSLS name\n");
        free(msls);
        return NULL;
    }
    snprintf(name, 100, "MultipleStartLocalSearch");

    msls->base.name = name;
    msls->base.solve = MSLS_solve;
    msls->num_iterations = num_iterations;

    return msls;
}

/**
 * @brief Performs the Multiple Start Local Search.
 * Executes multiple local search iterations starting from random solutions.
 *
 * @param algo Pointer to the MSLS algorithm instance.
 * @param distances 2D array of distances between nodes.
 * @param num_nodes Total number of nodes.
 * @param costs Array of node costs.
 * @param num_solutions Number of local search iterations to perform.
 * @return Result structure containing best, worst, and average costs.
 */
static Result MSLS_solve(Algo *algo, const int **distances, int num_nodes, const int *costs, int num_solutions)
{
    MSLS* msls = (MSLS*)algo;
    int solution_size = (num_nodes + 1) / 2; // Selecting approximately 50% of the nodes

    int total_iterations = num_solutions;
    int bestCost = INT_MAX;
    int worstCost = INT_MIN;
    long long totalCost = 0;

    int* bestSolution = NULL;
    int bestSolutionSize = 0;
    int* worstSolution = NULL;
    int worstSolutionSize = 0;

    // Iterate for the specified number of solutions
    for (int iter = 0; iter < num_solutions*20; iter++)
    {
        // Generate a random starting solution
        int* current_solution = (int*)malloc(solution_size * sizeof(int));
        if (!current_solution)
        {
            fprintf(stderr, "Error: Memory allocation failed in MSLS_solve\n");
            Result res = {INT_MAX, INT_MIN, 0.0, NULL, 0, NULL, 0};
            return res;
        }

        int* all_nodes = (int*)malloc(num_nodes * sizeof(int));
        if (!all_nodes)
        {
            fprintf(stderr, "Error: Memory allocation failed in MSLS_solve\n");
            free(current_solution);
            Result res = {INT_MAX, INT_MIN, 0.0, NULL, 0, NULL, 0};
            return res;
        }

        for (int i = 0; i < num_nodes; i++)
        {
            all_nodes[i] = i;
        }

        shuffle_array(all_nodes, num_nodes);
        memcpy(current_solution, all_nodes, solution_size * sizeof(int));
        free(all_nodes);

        // Perform local search on the current solution
        Result local_res = perform_local_search(current_solution, solution_size, distances, costs, num_nodes);

        // Update best, worst, and total costs
        totalCost += local_res.bestCost;
        if (local_res.bestCost < bestCost)
        {
            bestCost = local_res.bestCost;
            if (bestSolution)
                free(bestSolution);
            bestSolution = (int*)malloc(solution_size * sizeof(int));
            if (bestSolution)
            {
                memcpy(bestSolution, local_res.bestSolution, solution_size * sizeof(int));
                bestSolutionSize = solution_size;
            }
        }
        if (local_res.bestCost > worstCost)
        {
            worstCost = local_res.bestCost;
            if (worstSolution)
                free(worstSolution);
            worstSolution = (int*)malloc(solution_size * sizeof(int));
            if (worstSolution)
            {
                memcpy(worstSolution, local_res.worstSolution, solution_size * sizeof(int));
                worstSolutionSize = solution_size;
            }
        }

        // Free resources allocated for this iteration
        free(current_solution);
        free(local_res.bestSolution);
        free(local_res.worstSolution);
    }

    double averageCost = (total_iterations > 0) ? ((double)totalCost / total_iterations) : 0.0;

    Result res;
    res.bestCost = bestCost;
    res.worstCost = worstCost;
    res.averageCost = averageCost;
    res.bestSolution = bestSolution;
    res.bestSolutionSize = bestSolutionSize;
    res.worstSolution = worstSolution;
    res.worstSolutionSize = worstSolutionSize;

    return res;
}
