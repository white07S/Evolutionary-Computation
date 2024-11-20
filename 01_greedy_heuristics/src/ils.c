#include "ils.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <time.h>

// Include necessary headers for local search
#include "local_search.h"
#include "utils.h"
#include <linux/time.h>


/**
 * @brief Performs a perturbation on the given solution by performing k random 2-opt moves.
 *
 * @param solution Current solution array.
 * @param solution_size Size of the solution.
 * @param k Number of perturbation moves.
 */
static void perturb_solution(int* solution, int solution_size, int k)
{
    for (int i = 0; i < k; i++)
    {
        // Select two distinct random indices for 2-opt
        int idx1 = rand() % solution_size;
        int idx2 = rand() % solution_size;
        if (idx1 == idx2)
            continue;
        if (idx1 > idx2)
        {
            int temp = idx1;
            idx1 = idx2;
            idx2 = temp;
        }

        // Reverse the segment between idx1 and idx2
        while (idx1 < idx2)
        {
            int temp = solution[idx1];
            solution[idx1] = solution[idx2];
            solution[idx2] = temp;
            idx1++;
            idx2--;
        }
    }
}

/**
 * @brief Gets the current time in milliseconds.
 *
 * @return Current time in milliseconds.
 */
static long long current_time_ms()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)(ts.tv_sec) * 1000 + (ts.tv_nsec) / 1000000;
}

// Forward declaration of the solve function
static Result ILS_solve(Algo *algo, const int **distances, int num_nodes, const int *costs, int num_solutions);

/**
 * @brief Creates an instance of the ILS algorithm.
 *
 * @param max_time_ms Maximum running time in milliseconds.
 * @param perturbation_strength Number of moves to perturb the solution.
 * @return Pointer to the created ILS instance.
 */
ILS* create_ILS(int max_time_ms, int perturbation_strength)
{
    ILS* ils = (ILS*)malloc(sizeof(ILS));
    if (!ils)
    {
        fprintf(stderr, "Error: Memory allocation failed for ILS\n");
        return NULL;
    }

    char* name = (char*)malloc(100 * sizeof(char));
    if (!name)
    {
        fprintf(stderr, "Error: Memory allocation failed for ILS name\n");
        free(ils);
        return NULL;
    }
    snprintf(name, 100, "IteratedLocalSearch");

    ils->base.name = name;
    ils->base.solve = ILS_solve;
    ils->max_time_ms = max_time_ms;
    ils->perturbation_strength = perturbation_strength;

    return ils;
}

/**
 * @brief Performs the Iterated Local Search.
 * Alternates between local search and perturbation until the time limit is reached.
 *
 * @param algo Pointer to the ILS algorithm instance.
 * @param distances 2D array of distances between nodes.
 * @param num_nodes Total number of nodes.
 * @param costs Array of node costs.
 * @param num_solutions Number of ILS iterations (not used, controlled by time).
 * @return Result structure containing the best, worst, and average costs.
 */
static Result ILS_solve(Algo *algo, const int **distances, int num_nodes, const int *costs, int num_solutions)
{
    ILS* ils = (ILS*)algo;
    int solution_size = (num_nodes + 1) / 2; // Selecting approximately 50% of the nodes

    int bestCost = INT_MAX;
    int worstCost = INT_MIN;
    long long totalCost = 0;
    int iterations = 0;

    int* bestSolution = NULL;
    int bestSolutionSize = 0;
    int* worstSolution = NULL;
    int worstSolutionSize = 0;

    // Initialize timer
    long long start_time = current_time_ms();
    long long end_time = start_time + ils->max_time_ms;

    // Generate initial random solution
    int* current_solution = (int*)malloc(solution_size * sizeof(int));
    if (!current_solution)
    {
        fprintf(stderr, "Error: Memory allocation failed in ILS_solve\n");
        Result res = {INT_MAX, INT_MIN, 0.0, NULL, 0, NULL, 0};
        return res;
    }

    int* all_nodes = (int*)malloc(num_nodes * sizeof(int));
    if (!all_nodes)
    {
        fprintf(stderr, "Error: Memory allocation failed in ILS_solve\n");
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

    // Perform initial local search
    Result local_res = perform_local_search(current_solution, solution_size, distances, costs, num_nodes);
    iterations++;

    // Update best and worst
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

    free(local_res.bestSolution);
    free(local_res.worstSolution);

    // Iteratively perform perturbation and local search
    while (current_time_ms() < end_time)
    {
        // Perturb the current solution
        perturb_solution(current_solution, solution_size, ils->perturbation_strength);

        // Perform local search on the perturbed solution
        Result perturbed_res = perform_local_search(current_solution, solution_size, distances, costs, num_nodes);
        iterations++;

        // Update best and worst
        totalCost += perturbed_res.bestCost;
        if (perturbed_res.bestCost < bestCost)
        {
            bestCost = perturbed_res.bestCost;
            if (bestSolution)
                free(bestSolution);
            bestSolution = (int*)malloc(solution_size * sizeof(int));
            if (bestSolution)
            {
                memcpy(bestSolution, perturbed_res.bestSolution, solution_size * sizeof(int));
                bestSolutionSize = solution_size;
            }
        }
        if (perturbed_res.bestCost > worstCost)
        {
            worstCost = perturbed_res.bestCost;
            if (worstSolution)
                free(worstSolution);
            worstSolution = (int*)malloc(solution_size * sizeof(int));
            if (worstSolution)
            {
                memcpy(worstSolution, perturbed_res.worstSolution, solution_size * sizeof(int));
                worstSolutionSize = solution_size;
            }
        }

        free(perturbed_res.bestSolution);
        free(perturbed_res.worstSolution);
    }

    double averageCost = (iterations > 0) ? ((double)totalCost / iterations) : 0.0;

    Result res;
    res.bestCost = bestCost;
    res.worstCost = worstCost;
    res.averageCost = averageCost;
    res.bestSolution = bestSolution;
    res.bestSolutionSize = bestSolutionSize;
    res.worstSolution = worstSolution;
    res.worstSolutionSize = worstSolutionSize;

    free(current_solution);

    return res;
}
