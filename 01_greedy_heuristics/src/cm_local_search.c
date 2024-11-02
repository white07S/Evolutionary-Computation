#include "cm_local_search.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>

// Function prototypes
static Result CM_LocalSearch_solve(Algo *algo, const int **distances, int num_nodes, const int *costs, int num_solutions);

static void compute_candidate_edges(const int **distances, const int *costs, int num_nodes, int candidate_list_size, int **candidate_edges);

static int is_candidate_edge(int node_u, int node_v, int **candidate_edges, int candidate_list_size);

static int delta_two_edges_exchange_candidate(const int* solution, int solution_size, const int** distances, int i, int j, int **candidate_edges, int candidate_list_size);

static int delta_inter_route_exchange_candidate(const int* solution, int solution_size, const int** distances, const int* costs, int i, int node_j, int **candidate_edges, int candidate_list_size);

static void reverse_segment(int* solution, int start, int end, int solution_size);

// Remove the declaration of shuffle_array since it's declared in utils.h
// static void shuffle_array(int* array, int n);

// Comparison function for qsort
static int compare_node_values(const void* a, const void* b);

// Function to create a CM_LocalSearch algorithm
CM_LocalSearch* create_CM_LocalSearch(int candidate_list_size)
{
    CM_LocalSearch* cm_ls = (CM_LocalSearch*)malloc(sizeof(CM_LocalSearch));
    if (!cm_ls)
    {
        fprintf(stderr, "Error: Memory allocation failed for CM_LocalSearch\n");
        return NULL;
    }
    char* name = (char*)malloc(100 * sizeof(char));
    if (!name)
    {
        fprintf(stderr, "Error: Memory allocation failed for CM_LocalSearch name\n");
        free(cm_ls);
        return NULL;
    }
    snprintf(name, 100, "CM_LocalSearch_CandidateListSize_%d", candidate_list_size);

    cm_ls->base.name = name;
    cm_ls->base.solve = CM_LocalSearch_solve;
    cm_ls->candidate_list_size = candidate_list_size;
    return cm_ls;
}

// Implement CM_LocalSearch_solve
static Result CM_LocalSearch_solve(Algo *algo, const int **distances, int num_nodes, const int *costs, int num_solutions)
{
    CM_LocalSearch* cm_ls = (CM_LocalSearch*)algo;
    int solution_size = (num_nodes + 1) / 2; // Round up to select 50% of the nodes

    int total_iterations = num_solutions;
    int bestCost = INT_MAX;
    int worstCost = INT_MIN;
    long long totalCost = 0;

    int* bestSolution = NULL;
    int bestSolutionSize = 0;
    int* worstSolution = NULL;
    int worstSolutionSize = 0;

    // Precompute candidate edges
    int candidate_list_size = cm_ls->candidate_list_size;
    int **candidate_edges = (int**)malloc(num_nodes * sizeof(int*));
    if (!candidate_edges)
    {
        fprintf(stderr, "Error: Memory allocation failed for candidate_edges\n");
        Result res = {INT_MAX, INT_MIN, 0.0, NULL, 0, NULL, 0};
        return res;
    }
    for (int i = 0; i < num_nodes; i++)
    {
        candidate_edges[i] = (int*)malloc(candidate_list_size * sizeof(int));
        if (!candidate_edges[i])
        {
            fprintf(stderr, "Error: Memory allocation failed for candidate_edges[%d]\n", i);
            for (int k = 0; k < i; k++)
                free(candidate_edges[k]);
            free(candidate_edges);
            Result res = {INT_MAX, INT_MIN, 0.0, NULL, 0, NULL, 0};
            return res;
        }
    }

    // Compute candidate edges
    compute_candidate_edges(distances, costs, num_nodes, candidate_list_size, candidate_edges);

    // For num_solutions iterations
    for (int iter = 0; iter < num_solutions; iter++)
    {
        // Generate random starting solution
        int* current_solution = (int*)malloc(solution_size * sizeof(int));
        if (!current_solution)
        {
            fprintf(stderr, "Error: Memory allocation failed in CM_LocalSearch_solve\n");
            for (int i = 0; i < num_nodes; i++)
                free(candidate_edges[i]);
            free(candidate_edges);
            Result res = {INT_MAX, INT_MIN, 0.0, NULL, 0, NULL, 0};
            return res;
        }
        // Random starting solution
        int* all_nodes = (int*)malloc(num_nodes * sizeof(int));
        if (!all_nodes)
        {
            fprintf(stderr, "Error: Memory allocation failed in CM_LocalSearch_solve\n");
            free(current_solution);
            for (int i = 0; i < num_nodes; i++)
                free(candidate_edges[i]);
            free(candidate_edges);
            Result res = {INT_MAX, INT_MIN, 0.0, NULL, 0, NULL, 0};
            return res;
        }
        for (int i = 0; i < num_nodes; i++)
        {
            all_nodes[i] = i;
        }
        // Use shuffle_array from utils.h
        shuffle_array(all_nodes, num_nodes);
        memcpy(current_solution, all_nodes, solution_size * sizeof(int));
        free(all_nodes);

        // Prepare a boolean array for fast checking if a node is in the solution
        char* in_solution = (char*)calloc(num_nodes, sizeof(char));
        if (!in_solution)
        {
            fprintf(stderr, "Error: Memory allocation failed in CM_LocalSearch_solve (in_solution)\n");
            free(current_solution);
            for (int i = 0; i < num_nodes; i++)
                free(candidate_edges[i]);
            free(candidate_edges);
            Result res = {INT_MAX, INT_MIN, 0.0, NULL, 0, NULL, 0};
            return res;
        }
        for (int i = 0; i < solution_size; i++)
        {
            in_solution[current_solution[i]] = 1;
        }

        // Perform steepest local search with candidate moves
        int current_cost = calculate_cost(current_solution, solution_size, distances, costs);
        int improvement = 1;
        while (improvement)
        {
            improvement = 0;
            int best_delta = 0;
            int move_i = -1, move_j = -1;
            int move_type = -1; // 0 for intra-route, 1 for inter-route

            // Evaluate all possible candidate moves and select the best
            int delta = 0;
            // Intra-route moves (two-edges exchange)
            for (int i = 0; i < solution_size; i++)
            {
                for (int j = i + 2; j < solution_size + (i > 0 ? 0 : -1); j++)
                {
                    int jj = j % solution_size;
                    if (i == jj)
                        continue;
                    delta = delta_two_edges_exchange_candidate(current_solution, solution_size, distances, i, jj, candidate_edges, candidate_list_size);
                    if (delta < best_delta)
                    {
                        best_delta = delta;
                        move_i = i;
                        move_j = jj;
                        move_type = 0; // Intra-route
                    }
                }
            }
            // Inter-route moves
            for (int i = 0; i < solution_size; i++)
            {
                int node_i = current_solution[i];
                for (int k = 0; k < candidate_list_size; k++)
                {
                    int node_j = candidate_edges[node_i][k];
                    if (!in_solution[node_j])
                    {
                        delta = delta_inter_route_exchange_candidate(current_solution, solution_size, distances, costs, i, node_j, candidate_edges, candidate_list_size);
                        if (delta < best_delta)
                        {
                            best_delta = delta;
                            move_i = i;
                            move_j = node_j;
                            move_type = 1; // Inter-route
                        }
                    }
                }
            }

            if (best_delta < 0)
            {
                // Apply the best move
                if (move_type == 0)
                {
                    // Two-edges exchange (2-opt)
                    reverse_segment(current_solution, (move_i + 1) % solution_size, move_j, solution_size);
                }
                else if (move_type == 1)
                {
                    // Inter-route move
                    int old_node = current_solution[move_i];
                    in_solution[old_node] = 0;
                    current_solution[move_i] = move_j;
                    in_solution[move_j] = 1;
                }
                current_cost += best_delta;
                improvement = 1;
            }
        }

        // Update best, worst, total cost
        totalCost += current_cost;
        if (current_cost < bestCost)
        {
            bestCost = current_cost;
            if (bestSolution)
                free(bestSolution);
            bestSolution = (int*)malloc(solution_size * sizeof(int));
            if (bestSolution)
            {
                memcpy(bestSolution, current_solution, solution_size * sizeof(int));
                bestSolutionSize = solution_size;
            }
        }
        if (current_cost > worstCost)
        {
            worstCost = current_cost;
            if (worstSolution)
                free(worstSolution);
            worstSolution = (int*)malloc(solution_size * sizeof(int));
            if (worstSolution)
            {
                memcpy(worstSolution, current_solution, solution_size * sizeof(int));
                worstSolutionSize = solution_size;
            }
        }
        free(current_solution);
        free(in_solution);
    }

    double averageCost = (total_iterations > 0) ? ((double)totalCost / total_iterations) : 0.0;

    // Free candidate_edges
    for (int i = 0; i < num_nodes; i++)
        free(candidate_edges[i]);
    free(candidate_edges);

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

// Move compare_node_values outside of compute_candidate_edges
static int compare_node_values(const void* a, const void* b)
{
    typedef struct {
        int node;
        int value;
    } NodeValue;

    NodeValue* nv1 = (NodeValue*)a;
    NodeValue* nv2 = (NodeValue*)b;
    return nv1->value - nv2->value;
}

static void compute_candidate_edges(const int **distances, const int *costs, int num_nodes, int candidate_list_size, int **candidate_edges)
{
    // For each node, compute its candidate list
    for (int i = 0; i < num_nodes; i++)
    {
        // Create an array of (node_index, value) pairs
        typedef struct {
            int node;
            int value;
        } NodeValue;

        NodeValue* node_values = (NodeValue*)malloc(num_nodes * sizeof(NodeValue));
        if (!node_values)
        {
            fprintf(stderr, "Error: Memory allocation failed in compute_candidate_edges\n");
            exit(EXIT_FAILURE);
        }
        for (int j = 0; j < num_nodes; j++)
        {
            if (j == i)
            {
                node_values[j].node = j;
                node_values[j].value = INT_MAX; // Set a high value to exclude self
            }
            else
            {
                node_values[j].node = j;
                node_values[j].value = distances[i][j] + costs[j];
            }
        }

        // Use qsort and compare_node_values function
        qsort(node_values, num_nodes, sizeof(NodeValue), compare_node_values);

        for (int k = 0; k < candidate_list_size; k++)
        {
            candidate_edges[i][k] = node_values[k].node;
        }
        free(node_values);
    }
}

static int is_candidate_edge(int node_u, int node_v, int **candidate_edges, int candidate_list_size)
{
    // Check if node_v is in candidate_edges[node_u]
    for (int k = 0; k < candidate_list_size; k++)
    {
        if (candidate_edges[node_u][k] == node_v)
            return 1;
    }
    // Also check if node_u is in candidate_edges[node_v]
    for (int k = 0; k < candidate_list_size; k++)
    {
        if (candidate_edges[node_v][k] == node_u)
            return 1;
    }
    return 0;
}

static int delta_two_edges_exchange_candidate(const int* solution, int solution_size, const int** distances, int i, int j, int **candidate_edges, int candidate_list_size)
{
    int size = solution_size;

    int node_i = solution[i];
    int node_ip1 = solution[(i + 1) % size];
    int node_j = solution[j];
    int node_jp1 = solution[(j + 1) % size];

    if (i == j)
    {
        return 0;
    }

    // Avoid swapping the same edge or adjacent edges that would create loops
    if ((i + 1) % size == j || (j + 1) % size == i)
    {
        return 0;
    }

    // Check if move introduces at least one candidate edge
    int introduces_candidate_edge = 0;
    if (is_candidate_edge(node_i, node_j, candidate_edges, candidate_list_size) ||
        is_candidate_edge(node_ip1, node_jp1, candidate_edges, candidate_list_size))
    {
        introduces_candidate_edge = 1;
    }

    if (!introduces_candidate_edge)
    {
        return 0; // Do not consider this move
    }

    int delta = 0;

    delta -= distances[node_i][node_ip1];
    delta -= distances[node_j][node_jp1];
    delta += distances[node_i][node_j];
    delta += distances[node_ip1][node_jp1];

    return delta;
}

static int delta_inter_route_exchange_candidate(const int* solution, int solution_size, const int** distances, const int* costs, int i, int node_j, int **candidate_edges, int candidate_list_size)
{
    int size = solution_size;
    int node_i = solution[i];
    int prev_i = (i == 0) ? size - 1 : i - 1;
    int next_i = (i + 1) % size;

    // Edges involved are:
    // Remove edges: (prev_i, node_i), (node_i, next_i)
    // Add edges: (prev_i, node_j), (node_j, next_i)

    // Check if move introduces at least one candidate edge
    int introduces_candidate_edge = 0;
    if (is_candidate_edge(solution[prev_i], node_j, candidate_edges, candidate_list_size) ||
        is_candidate_edge(node_j, solution[next_i], candidate_edges, candidate_list_size))
    {
        introduces_candidate_edge = 1;
    }

    if (!introduces_candidate_edge)
    {
        return 0; // Do not consider this move
    }

    int delta = 0;

    delta -= distances[solution[prev_i]][node_i];
    delta -= distances[node_i][solution[next_i]];
    delta += distances[solution[prev_i]][node_j];
    delta += distances[node_j][solution[next_i]];

    delta -= costs[node_i];
    delta += costs[node_j];

    return delta;
}

static void reverse_segment(int* solution, int start, int end, int solution_size)
{
    int size = solution_size;
    int i = start;
    int j = end;

    while (i != j && (i + size - 1) % size != j)
    {
        int temp = solution[i];
        solution[i] = solution[j];
        solution[j] = temp;

        i = (i + 1) % size;
        if (j == 0)
            j = size - 1;
        else
            j = j - 1;
    }
}
