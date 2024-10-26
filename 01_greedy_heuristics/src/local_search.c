#include "local_search.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>

// Define Move structure
typedef struct
{
    int i;
    int j;
    int type; // 0 for intra-route, 1 for inter-route
} Move;

// Function prototypes
static Result LocalSearch_solve(Algo *algo, const int **distances, int num_nodes, const int *costs, int num_solutions);
static int delta_two_nodes_exchange(const int* solution, int solution_size, const int** distances, int i, int j);
static int delta_two_edges_exchange(const int* solution, int solution_size, const int** distances, int i, int j);
static int delta_inter_route_exchange(const int* solution, int solution_size, const int** distances, const int* costs, int i, int node_j);
static void swap_nodes(int* solution, int i, int j);
static void reverse_segment(int* solution, int start, int end, int solution_size);
static int is_in_solution(int node, const int* solution, int solution_size);
static void shuffle_moves(Move* moves, int n);
static void generate_Greedy2Regret_solution(int start_node, const int **distances, int num_nodes, const int *costs, int solution_size, int *solution);

// Function to create a LocalSearch algorithm
LocalSearch* create_LocalSearch(int local_search_type, int intra_route_move_type, int starting_solution_type, int method_index)
{
    LocalSearch* ls = (LocalSearch*)malloc(sizeof(LocalSearch));
    if (!ls)
    {
        fprintf(stderr, "Error: Memory allocation failed for LocalSearch\n");
        return NULL;
    }

    char* ls_type_str = local_search_type == 0 ? "Steepest" : "Greedy";
    char* move_type_str = intra_route_move_type == 0 ? "TwoNodes" : "TwoEdges";
    char* start_type_str = starting_solution_type == 0 ? "RandomStart" : "GreedyStart";

    char* name = (char*)malloc(100 * sizeof(char));
    if (!name)
    {
        fprintf(stderr, "Error: Memory allocation failed for LocalSearch name\n");
        free(ls);
        return NULL;
    }
    snprintf(name, 100, "LocalSearch_%s_%s_%s", ls_type_str, move_type_str, start_type_str);

    ls->base.name = name;
    ls->base.solve = LocalSearch_solve;
    ls->local_search_type = local_search_type;
    ls->intra_route_move_type = intra_route_move_type;
    ls->starting_solution_type = starting_solution_type;
    ls->method_index = method_index;
    return ls;
}

// LocalSearch solve function
static Result LocalSearch_solve(Algo *algo, const int **distances, int num_nodes, const int *costs, int num_solutions)
{
    LocalSearch* ls = (LocalSearch*)algo;
    int solution_size = (num_nodes + 1) / 2; // Round up to select 50% of the nodes

    int bestCost = INT_MAX;
    int worstCost = INT_MIN;
    long long totalCost = 0;

    int* bestSolution = NULL;
    int bestSolutionSize = 0;
    int* worstSolution = NULL;
    int worstSolutionSize = 0;

    // For num_solutions iterations
    for (int iter = 0; iter < num_solutions; iter++)
    {
        // Generate initial solution
        int* current_solution = (int*)malloc(solution_size * sizeof(int));
        if (!current_solution)
        {
            fprintf(stderr, "Error: Memory allocation failed in LocalSearch_solve\n");
            Result res = {INT_MAX, INT_MIN, 0.0, NULL, 0, NULL, 0};
            return res;
        }

        if (ls->starting_solution_type == 0)
        {
            // Random starting solution
            int* all_nodes = (int*)malloc(num_nodes * sizeof(int));
            if (!all_nodes)
            {
                fprintf(stderr, "Error: Memory allocation failed in LocalSearch_solve\n");
                free(current_solution);
                Result res = {INT_MAX, INT_MIN, 0.0, NULL, 0, NULL, 0};
                return res;
            }
            for (int i = 0; i < num_nodes; i++)
            {
                all_nodes[i] = i;
            }
            // Shuffle and select first solution_size nodes
            shuffle_array(all_nodes, num_nodes);
            memcpy(current_solution, all_nodes, solution_size * sizeof(int));
            free(all_nodes);
        }
        else if (ls->starting_solution_type == 1)
        {
            // Greedy heuristic starting solution (Weighted Greedy 2-Regret)
            int start_node = rand() % num_nodes;
            generate_Greedy2Regret_solution(start_node, distances, num_nodes, costs, solution_size, current_solution);
        }

        // Perform local search on current_solution
        int current_cost = calculate_cost(current_solution, solution_size, distances, costs);

        int improvement = 1;
        while (improvement)
        {
            improvement = 0;
            int best_delta = 0;
            int move_i = -1, move_j = -1;
            int move_type = -1; // 0 for intra-route, 1 for inter-route

            if (ls->local_search_type == 0) // Steepest
            {
                // Evaluate all possible moves and select the best
                int delta = 0;
                if (ls->intra_route_move_type == 0)
                {
                    // Two-nodes exchange
                    for (int i = 0; i < solution_size - 1; i++)
                    {
                        for (int j = i + 1; j < solution_size; j++)
                        {
                            delta = delta_two_nodes_exchange(current_solution, solution_size, distances, i, j);
                            if (delta < best_delta)
                            {
                                best_delta = delta;
                                move_i = i;
                                move_j = j;
                                move_type = 0;
                            }
                        }
                    }
                }
                else if (ls->intra_route_move_type == 1)
                {
                    // Two-edges exchange (2-opt)
                    for (int i = 0; i < solution_size; i++)
                    {
                        for (int j = i + 2; j < solution_size + (i > 0 ? 0 : -1); j++)
                        {
                            int jj = j % solution_size;
                            if (i == jj)
                                continue;
                            delta = delta_two_edges_exchange(current_solution, solution_size, distances, i, jj);
                            if (delta < best_delta)
                            {
                                best_delta = delta;
                                move_i = i;
                                move_j = jj;
                                move_type = 0;
                            }
                        }
                    }
                }
                // Inter-route moves
                for (int i = 0; i < solution_size; i++)
                {
                    for (int node_j = 0; node_j < num_nodes; node_j++)
                    {
                        if (!is_in_solution(node_j, current_solution, solution_size))
                        {
                            delta = delta_inter_route_exchange(current_solution, solution_size, distances, costs, i, node_j);
                            if (delta < best_delta)
                            {
                                best_delta = delta;
                                move_i = i;
                                move_j = node_j;
                                move_type = 1;
                            }
                        }
                    }
                }

                if (best_delta < 0)
                {
                    // Apply the best move
                    if (move_type == 0)
                    {
                        if (ls->intra_route_move_type == 0)
                        {
                            // Two-nodes exchange
                            swap_nodes(current_solution, move_i, move_j);
                        }
                        else if (ls->intra_route_move_type == 1)
                        {
                            // Two-edges exchange (2-opt)
                            reverse_segment(current_solution, (move_i + 1) % solution_size, move_j, solution_size);
                        }
                    }
                    else if (move_type == 1)
                    {
                        // Inter-route move
                        current_solution[move_i] = move_j;
                    }
                    current_cost += best_delta;
                    improvement = 1;
                }
            }
            else if (ls->local_search_type == 1) // Greedy
            {
                // Generate list of moves and shuffle
                int num_moves = 0;
                Move* moves = NULL;
                int max_moves = (solution_size * (solution_size - 1)) / 2 + solution_size * (num_nodes - solution_size);

                moves = (Move*)malloc(max_moves * sizeof(Move));
                if (!moves)
                {
                    fprintf(stderr, "Error: Memory allocation failed in LocalSearch_solve\n");
                    free(current_solution);
                    Result res = {INT_MAX, INT_MIN, 0.0, NULL, 0, NULL, 0};
                    return res;
                }

                int k = 0;
                if (ls->intra_route_move_type == 0)
                {
                    // Two-nodes exchange
                    for (int i = 0; i < solution_size - 1; i++)
                    {
                        for (int j = i + 1; j < solution_size; j++)
                        {
                            moves[k].i = i;
                            moves[k].j = j;
                            moves[k].type = 0; // Intra-route
                            k++;
                        }
                    }
                }
                else if (ls->intra_route_move_type == 1)
                {
                    // Two-edges exchange (2-opt)
                    for (int i = 0; i < solution_size; i++)
                    {
                        for (int j = i + 2; j < solution_size + (i > 0 ? 0 : -1); j++)
                        {
                            int jj = j % solution_size;
                            if (i == jj)
                                continue;
                            moves[k].i = i;
                            moves[k].j = jj;
                            moves[k].type = 0; // Intra-route
                            k++;
                        }
                    }
                }
                // Inter-route moves
                for (int i = 0; i < solution_size; i++)
                {
                    for (int node_j = 0; node_j < num_nodes; node_j++)
                    {
                        if (!is_in_solution(node_j, current_solution, solution_size))
                        {
                            moves[k].i = i;
                            moves[k].j = node_j;
                            moves[k].type = 1; // Inter-route
                            k++;
                        }
                    }
                }
                num_moves = k;
                shuffle_moves(moves, num_moves);

                for (int m = 0; m < num_moves; m++)
                {
                    int delta = 0;
                    if (moves[m].type == 0)
                    {
                        // Intra-route
                        if (ls->intra_route_move_type == 0)
                        {
                            delta = delta_two_nodes_exchange(current_solution, solution_size, distances, moves[m].i, moves[m].j);
                        }
                        else if (ls->intra_route_move_type == 1)
                        {
                            delta = delta_two_edges_exchange(current_solution, solution_size, distances, moves[m].i, moves[m].j);
                        }
                    }
                    else if (moves[m].type == 1)
                    {
                        // Inter-route
                        delta = delta_inter_route_exchange(current_solution, solution_size, distances, costs, moves[m].i, moves[m].j);
                    }
                    if (delta < 0)
                    {
                        // Apply the move
                        if (moves[m].type == 0)
                        {
                            if (ls->intra_route_move_type == 0)
                            {
                                // Two-nodes exchange
                                swap_nodes(current_solution, moves[m].i, moves[m].j);
                            }
                            else if (ls->intra_route_move_type == 1)
                            {
                                // Two-edges exchange
                                reverse_segment(current_solution, (moves[m].i + 1) % solution_size, moves[m].j, solution_size);
                            }
                        }
                        else if (moves[m].type == 1)
                        {
                            // Inter-route
                            current_solution[moves[m].i] = moves[m].j;
                        }
                        current_cost += delta;
                        improvement = 1;
                        break;
                    }
                }
                free(moves);
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
    }

    double averageCost = (num_solutions > 0) ? ((double)totalCost / num_solutions) : 0.0;

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

// Helper functions implementation

static int delta_two_nodes_exchange(const int* solution, int solution_size, const int** distances, int i, int j)
{
    int delta = 0;
    int size = solution_size;

    int prev_i = (i == 0) ? size - 1 : i - 1;
    int next_i = (i + 1) % size;
    int prev_j = (j == 0) ? size - 1 : j - 1;
    int next_j = (j + 1) % size;

    int node_i = solution[i];
    int node_j = solution[j];

    if (j == next_i)
    {
        // Nodes are adjacent (i before j)
        delta -= distances[solution[prev_i]][node_i];
        delta -= distances[node_i][node_j];
        delta -= distances[node_j][solution[next_j]];
        delta += distances[solution[prev_i]][node_j];
        delta += distances[node_j][node_i];
        delta += distances[node_i][solution[next_j]];
    }
    else if (i == next_j)
    {
        // Nodes are adjacent (j before i)
        delta -= distances[solution[prev_j]][node_j];
        delta -= distances[node_j][node_i];
        delta -= distances[node_i][solution[next_i]];
        delta += distances[solution[prev_j]][node_i];
        delta += distances[node_i][node_j];
        delta += distances[node_j][solution[next_i]];
    }
    else
    {
        // Nodes are not adjacent
        delta -= distances[solution[prev_i]][node_i];
        delta -= distances[node_i][solution[next_i]];
        delta -= distances[solution[prev_j]][node_j];
        delta -= distances[node_j][solution[next_j]];
        delta += distances[solution[prev_i]][node_j];
        delta += distances[node_j][solution[next_i]];
        delta += distances[solution[prev_j]][node_i];
        delta += distances[node_i][solution[next_j]];
    }

    return delta;
}

static int delta_two_edges_exchange(const int* solution, int solution_size, const int** distances, int i, int j)
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

    int delta = 0;

    delta -= distances[node_i][node_ip1];
    delta -= distances[node_j][node_jp1];
    delta += distances[node_i][node_j];
    delta += distances[node_ip1][node_jp1];

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

static int delta_inter_route_exchange(const int* solution, int solution_size, const int** distances, const int* costs, int i, int node_j)
{
    int size = solution_size;
    int node_i = solution[i];
    int prev_i = (i == 0) ? size - 1 : i - 1;
    int next_i = (i + 1) % size;

    int delta = 0;

    delta -= distances[solution[prev_i]][node_i];
    delta -= distances[node_i][solution[next_i]];
    delta += distances[solution[prev_i]][node_j];
    delta += distances[node_j][solution[next_i]];

    delta -= costs[node_i];
    delta += costs[node_j];

    return delta;
}

static void swap_nodes(int* solution, int i, int j)
{
    int temp = solution[i];
    solution[i] = solution[j];
    solution[j] = temp;
}

static int is_in_solution(int node, const int* solution, int solution_size)
{
    for (int i = 0; i < solution_size; i++)
    {
        if (solution[i] == node)
            return 1;
    }
    return 0;
}

static void shuffle_moves(Move* moves, int n)
{
    for (int i = n - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);
        Move temp = moves[i];
        moves[i] = moves[j];
        moves[j] = temp;
    }
}

// Function to generate a single Greedy 2-Regret solution starting from a given node
// This version uses weights for regret and cost increase
static void generate_Greedy2Regret_solution(int start_node, const int **distances, int num_nodes, const int *costs, int solution_size, int *solution)
{
    // Set default weights
    double weight1 = 0.6; // Weight for regret
    double weight2 = 0.4; // Weight for cost increase

    int current_size = 0;
    char *visited = (char *)calloc(num_nodes, sizeof(char));
    if (!visited)
    {
        fprintf(stderr, "Error: Memory allocation failed in generate_Greedy2Regret_solution (visited)\n");
        exit(EXIT_FAILURE);
    }

    int *current_solution = solution;

    // Start with start_node
    current_solution[current_size++] = start_node;
    visited[start_node] = 1;

    // Find the farthest node to start forming a cycle
    int max_distance = -1;
    int farthest_node = -1;
    for (int j = 0; j < num_nodes; j++)
    {
        if (!visited[j])
        {
            int distance = distances[start_node][j];
            if (distance > max_distance)
            {
                max_distance = distance;
                farthest_node = j;
            }
        }
    }

    if (farthest_node == -1)
    {
        free(visited);
        return;
    }

    current_solution[current_size++] = farthest_node;
    visited[farthest_node] = 1;

    // Build the solution
    while (current_size < solution_size)
    {
        double max_score = -1e9;
        int candidate_node = -1;
        int candidate_position = -1;

        // Iterate over all unvisited nodes
        for (int k = 0; k < num_nodes; k++)
        {
            if (visited[k])
                continue;

            // For node k, compute insertion costs at all possible positions
            int smallest_cost = INT_MAX;
            int second_smallest_cost = INT_MAX;
            int best_pos = -1;

            for (int j = 0; j < current_size; j++)
            {
                int prev = current_solution[j];
                int next = current_solution[(j + 1) % current_size];
                int insertion_cost = distances[prev][k] + distances[k][next] - distances[prev][next] + costs[k];

                if (insertion_cost < smallest_cost)
                {
                    second_smallest_cost = smallest_cost;
                    smallest_cost = insertion_cost;
                    best_pos = (j + 1) % (current_size + 1);
                }
                else if (insertion_cost < second_smallest_cost)
                {
                    second_smallest_cost = insertion_cost;
                }
            }

            // Handle case where only one insertion position is possible
            if (second_smallest_cost == INT_MAX)
            {
                second_smallest_cost = smallest_cost;
            }

            // Compute regret
            int regret = second_smallest_cost - smallest_cost;

            // Compute score using weights
            double score = weight1 * regret - weight2 * smallest_cost;

            if (score > max_score)
            {
                max_score = score;
                candidate_node = k;
                candidate_position = best_pos;
            }
        }

        if (candidate_node == -1)
        {
            break;
        }

        // Insert candidate_node at candidate_position
        for (int m = current_size; m > candidate_position; m--)
        {
            current_solution[m] = current_solution[m - 1];
        }
        current_solution[candidate_position] = candidate_node;
        current_size++;
        visited[candidate_node] = 1;
    }

    free(visited);
}
