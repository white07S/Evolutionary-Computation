#include "algorithms.h"
#include "utils.h"
#include <limits.h>
#include <string.h>

// Function prototypes (forward declarations)
static Result RandomSearch_solve(Algo *algo, const int **distances, int num_nodes, const int *costs, int num_solutions);
static Result NearestNeighboursEndInsert_solve(Algo *algo, const int **distances, int num_nodes, const int *costs, int num_solutions);
static Result NearestNeighboursAnywhereInsert_solve(Algo *algo, const int **distances, int num_nodes, const int *costs, int num_solutions);
static Result GreedyCycle_solve(Algo *algo, const int **distances, int num_nodes, const int *costs, int num_solutions);
static Result Greedy2Regret_solve(Algo *algo, const int **distances, int num_nodes, const int *costs, int num_solutions);
static Result Greedy2RegretWeighted_solve(Algo *algo, const int **distances, int num_nodes, const int *costs, int num_solutions);

// ------------------ RandomSearch Algorithm ------------------

RandomSearch *create_RandomSearch()
{
    RandomSearch *rs = (RandomSearch *)malloc(sizeof(RandomSearch));
    if (!rs)
    {
        fprintf(stderr, "Error: Memory allocation failed for RandomSearch\n");
        return NULL;
    }
    rs->base.name = "RandomSearch";
    rs->base.solve = RandomSearch_solve;
    return rs;
}

static Result RandomSearch_solve(Algo *algo, const int **distances, int num_nodes, const int *costs, int num_solutions)
{
    int solution_size = (num_nodes + 1) / 2; // Corrected to round up

    // Initialize array of all nodes
    int *all_nodes = (int *)malloc(num_nodes * sizeof(int));
    if (!all_nodes)
    {
        fprintf(stderr, "Error: Memory allocation failed for all_nodes in RandomSearch\n");
        Result res = {INT_MAX, INT_MIN, 0.0, NULL, 0, NULL, 0};
        return res;
    }
    for (int i = 0; i < num_nodes; i++)
    {
        all_nodes[i] = i;
    }

    // Initialize best and worst
    int bestCost = INT_MAX;
    int worstCost = INT_MIN;
    long long totalCost = 0;

    int *bestSolution = NULL;
    int bestSolutionSize = 0;
    int *worstSolution = NULL;
    int worstSolutionSize = 0;

    // Allocate memory for current_solution
    int *current_solution = (int *)malloc(solution_size * sizeof(int));
    if (!current_solution)
    {
        fprintf(stderr, "Error: Memory allocation failed for current_solution in RandomSearch\n");
        free(all_nodes);
        Result res = {INT_MAX, INT_MIN, 0.0, NULL, 0, NULL, 0};
        return res;
    }

    for (int i = 0; i < num_solutions; i++)
    {
        // Shuffle all_nodes
        shuffle_array(all_nodes, num_nodes);
        // Take first solution_size elements
        memcpy(current_solution, all_nodes, solution_size * sizeof(int));

        // Check if valid solution
        if (!is_valid_solution(current_solution, solution_size, num_nodes))
        {
            fprintf(stderr, "Error: The generated solution is not valid\n");
        }

        // Calculate cost
        int current_cost = calculate_cost(current_solution, solution_size, distances, costs);
        totalCost += current_cost;

        // Update best
        if (current_cost < bestCost)
        {
            bestCost = current_cost;
            if (bestSolution)
                free(bestSolution);
            bestSolution = (int *)malloc(solution_size * sizeof(int));
            if (bestSolution)
            {
                memcpy(bestSolution, current_solution, solution_size * sizeof(int));
                bestSolutionSize = solution_size;
            }
        }

        // Update worst
        if (current_cost > worstCost)
        {
            worstCost = current_cost;
            if (worstSolution)
                free(worstSolution);
            worstSolution = (int *)malloc(solution_size * sizeof(int));
            if (worstSolution)
            {
                memcpy(worstSolution, current_solution, solution_size * sizeof(int));
                worstSolutionSize = solution_size;
            }
        }
    }

    // Calculate average cost
    double averageCost = (num_solutions > 0) ? ((double)totalCost / num_solutions) : 0.0;

    // Free allocated memory
    free(all_nodes);
    free(current_solution);

    // Prepare Result
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

// ------------------ NearestNeighboursEndInsert Algorithm ------------------

NearestNeighboursEndInsert *create_NearestNeighboursEndInsert()
{
    NearestNeighboursEndInsert *nn = (NearestNeighboursEndInsert *)malloc(sizeof(NearestNeighboursEndInsert));
    if (!nn)
    {
        fprintf(stderr, "Error: Memory allocation failed for NearestNeighboursEndInsert\n");
        return NULL;
    }
    nn->base.name = "NearestNeighboursEndInsert";
    nn->base.solve = NearestNeighboursEndInsert_solve;
    return nn;
}

static Result NearestNeighboursEndInsert_solve(Algo *algo, const int **distances, int num_nodes, const int *costs, int num_solutions)
{
    int solution_size = (num_nodes + 1) / 2;

    int total_iterations = num_nodes * num_solutions;

    int bestCost = INT_MAX;
    int worstCost = INT_MIN;
    long long totalCost = 0;

    int *bestSolution = NULL;
    int bestSolutionSize = 0;
    int *worstSolution = NULL;
    int worstSolutionSize = 0;

    // For each node as starting point
    for (int start_node = 0; start_node < num_nodes; start_node++)
    {
        // Generate num_solutions starting from this node
        for (int s = 0; s < num_solutions; s++)
        {
            // Initialize current solution
            int *current_solution = (int *)malloc(solution_size * sizeof(int));
            if (!current_solution)
            {
                fprintf(stderr, "Error: Memory allocation failed in NearestNeighboursEndInsert_solve\n");
                Result res = {INT_MAX, INT_MIN, 0.0, NULL, 0, NULL, 0};
                return res;
            }

            char *visited = (char *)calloc(num_nodes, sizeof(char));
            if (!visited)
            {
                fprintf(stderr, "Error: Memory allocation failed in NearestNeighboursEndInsert_solve\n");
                free(current_solution);
                Result res = {INT_MAX, INT_MIN, 0.0, NULL, 0, NULL, 0};
                return res;
            }

            int count = 0;
            current_solution[count++] = start_node;
            visited[start_node] = 1;

            while (count < solution_size)
            {
                int last_node = current_solution[count - 1];
                int nearest_nodes[num_nodes];
                int num_nearest = 0;
                int min_distance = INT_MAX;

                for (int j = 0; j < num_nodes; j++)
                {
                    if (!visited[j])
                    {
                        int distance = distances[last_node][j];
                        if (distance < min_distance)
                        {
                            min_distance = distance;
                            num_nearest = 0;
                            nearest_nodes[num_nearest++] = j;
                        }
                        else if (distance == min_distance)
                        {
                            nearest_nodes[num_nearest++] = j;
                        }
                    }
                }

                if (num_nearest == 0)
                {
                    break;
                }

                // Randomly select one of the nearest nodes
                int rand_index = rand() % num_nearest;
                int nearest_node = nearest_nodes[rand_index];

                current_solution[count++] = nearest_node;
                visited[nearest_node] = 1;
            }

            // Check if valid solution
            if (!is_valid_solution(current_solution, count, num_nodes))
            {
                fprintf(stderr, "Error: The generated solution is not valid\n");
            }

            // Calculate cost
            int current_cost = calculate_cost(current_solution, count, distances, costs);
            totalCost += current_cost;

            // Update best
            if (current_cost < bestCost)
            {
                bestCost = current_cost;
                if (bestSolution)
                    free(bestSolution);
                bestSolution = (int *)malloc(count * sizeof(int));
                if (bestSolution)
                {
                    memcpy(bestSolution, current_solution, count * sizeof(int));
                    bestSolutionSize = count;
                }
            }

            // Update worst
            if (current_cost > worstCost)
            {
                worstCost = current_cost;
                if (worstSolution)
                    free(worstSolution);
                worstSolution = (int *)malloc(count * sizeof(int));
                if (worstSolution)
                {
                    memcpy(worstSolution, current_solution, count * sizeof(int));
                    worstSolutionSize = count;
                }
            }

            free(visited);
            free(current_solution);
        }
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

// ------------------ NearestNeighboursAnywhereInsert Algorithm ------------------

NearestNeighboursAnywhereInsert *create_NearestNeighboursAnywhereInsert()
{
    NearestNeighboursAnywhereInsert *nn = (NearestNeighboursAnywhereInsert *)malloc(sizeof(NearestNeighboursAnywhereInsert));
    if (!nn)
    {
        fprintf(stderr, "Error: Memory allocation failed for NearestNeighboursAnywhereInsert\n");
        return NULL;
    }
    nn->base.name = "NearestNeighboursAnywhereInsert";
    nn->base.solve = NearestNeighboursAnywhereInsert_solve;
    return nn;
}

static Result NearestNeighboursAnywhereInsert_solve(Algo *algo, const int **distances, int num_nodes, const int *costs, int num_solutions)
{
    int solution_size = (num_nodes + 1) / 2;

    int total_iterations = num_nodes * num_solutions;

    int bestCost = INT_MAX;
    int worstCost = INT_MIN;
    long long totalCost = 0;

    int *bestSolution = NULL;
    int bestSolutionSize = 0;
    int *worstSolution = NULL;
    int worstSolutionSize = 0;

    // For each node as starting point
    for (int start_node = 0; start_node < num_nodes; start_node++)
    {
        // Generate num_solutions starting from this node
        for (int s = 0; s < num_solutions; s++)
        {
            // Initialize current solution
            int *current_solution = (int *)malloc(solution_size * sizeof(int));
            if (!current_solution)
            {
                fprintf(stderr, "Error: Memory allocation failed in NearestNeighboursAnywhereInsert_solve\n");
                Result res = {INT_MAX, INT_MIN, 0.0, NULL, 0, NULL, 0};
                return res;
            }

            char *visited = (char *)calloc(num_nodes, sizeof(char));
            if (!visited)
            {
                fprintf(stderr, "Error: Memory allocation failed in NearestNeighboursAnywhereInsert_solve\n");
                free(current_solution);
                Result res = {INT_MAX, INT_MIN, 0.0, NULL, 0, NULL, 0};
                return res;
            }

            int count = 0;
            current_solution[count++] = start_node;
            visited[start_node] = 1;

            while (count < solution_size)
            {
                int best_nodes[num_nodes];
                int best_positions[num_nodes];
                int num_best = 0;
                int min_increase = INT_MAX;

                for (int j = 0; j < num_nodes; j++)
                {
                    if (!visited[j])
                    {
                        for (int pos = 0; pos <= count; pos++)
                        {
                            int prev_node = (pos == 0) ? current_solution[count - 1] : current_solution[pos - 1];
                            int next_node = (pos == count) ? current_solution[0] : current_solution[pos];
                            int increase = distances[prev_node][j] + distances[j][next_node] - distances[prev_node][next_node] + costs[j];

                            if (increase < min_increase)
                            {
                                min_increase = increase;
                                num_best = 0;
                                best_nodes[num_best] = j;
                                best_positions[num_best++] = pos;
                            }
                            else if (increase == min_increase)
                            {
                                best_nodes[num_best] = j;
                                best_positions[num_best++] = pos;
                            }
                        }
                    }
                }

                if (num_best == 0)
                {
                    break;
                }

                // Randomly select one of the best nodes and positions
                int rand_index = rand() % num_best;
                int best_node = best_nodes[rand_index];
                int best_position = best_positions[rand_index];

                // Insert best_node at best_position
                for (int m = count; m > best_position; m--)
                {
                    current_solution[m] = current_solution[m - 1];
                }
                current_solution[best_position] = best_node;
                count++;
                visited[best_node] = 1;
            }

            // Check if valid solution
            if (!is_valid_solution(current_solution, count, num_nodes))
            {
                fprintf(stderr, "Error: The generated solution is not valid\n");
            }

            int current_cost = calculate_cost(current_solution, count, distances, costs);
            totalCost += current_cost;

            // Update best
            if (current_cost < bestCost)
            {
                bestCost = current_cost;
                if (bestSolution)
                    free(bestSolution);
                bestSolution = (int *)malloc(count * sizeof(int));
                if (bestSolution)
                {
                    memcpy(bestSolution, current_solution, count * sizeof(int));
                    bestSolutionSize = count;
                }
            }

            // Update worst
            if (current_cost > worstCost)
            {
                worstCost = current_cost;
                if (worstSolution)
                    free(worstSolution);
                worstSolution = (int *)malloc(count * sizeof(int));
                if (worstSolution)
                {
                    memcpy(worstSolution, current_solution, count * sizeof(int));
                    worstSolutionSize = count;
                }
            }

            free(visited);
            free(current_solution);
        }
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

// ------------------ GreedyCycle Algorithm ------------------

GreedyCycle *create_GreedyCycle()
{
    GreedyCycle *gc = (GreedyCycle *)malloc(sizeof(GreedyCycle));
    if (!gc)
    {
        fprintf(stderr, "Error: Memory allocation failed for GreedyCycle\n");
        return NULL;
    }
    gc->base.name = "GreedyCycle";
    gc->base.solve = GreedyCycle_solve;
    return gc;
}

static Result GreedyCycle_solve(Algo *algo, const int **distances, int num_nodes, const int *costs, int num_solutions)
{
    int solution_size = (num_nodes + 1) / 2;

    int total_iterations = num_nodes * num_solutions;

    int bestCost = INT_MAX;
    int worstCost = INT_MIN;
    long long totalCost = 0;

    int *bestSolution = NULL;
    int bestSolutionSize = 0;
    int *worstSolution = NULL;
    int worstSolutionSize = 0;

    // For each node as starting point
    for (int start_node = 0; start_node < num_nodes; start_node++)
    {
        // Generate num_solutions starting from this node
        for (int s = 0; s < num_solutions; s++)
        {
            // Initialize current solution
            int *current_solution = (int *)malloc(solution_size * sizeof(int));
            if (!current_solution)
            {
                fprintf(stderr, "Error: Memory allocation failed in GreedyCycle_solve\n");
                Result res = {INT_MAX, INT_MIN, 0.0, NULL, 0, NULL, 0};
                return res;
            }

            char *visited = (char *)calloc(num_nodes, sizeof(char));
            if (!visited)
            {
                fprintf(stderr, "Error: Memory allocation failed in GreedyCycle_solve\n");
                free(current_solution);
                Result res = {INT_MAX, INT_MIN, 0.0, NULL, 0, NULL, 0};
                return res;
            }

            int count = 0;
            current_solution[count++] = start_node;
            visited[start_node] = 1;

            // Find the farthest node to start forming a cycle
            int farthest_nodes[num_nodes];
            int num_farthest = 0;
            int max_distance = -1;
            for (int j = 0; j < num_nodes; j++)
            {
                if (!visited[j])
                {
                    int distance = distances[start_node][j];
                    if (distance > max_distance)
                    {
                        max_distance = distance;
                        num_farthest = 0;
                        farthest_nodes[num_farthest++] = j;
                    }
                    else if (distance == max_distance)
                    {
                        farthest_nodes[num_farthest++] = j;
                    }
                }
            }

            if (num_farthest == 0)
            {
                free(visited);
                free(current_solution);
                continue;
            }

            // Randomly select one of the farthest nodes
            int rand_index = rand() % num_farthest;
            int farthest_node = farthest_nodes[rand_index];

            current_solution[count++] = farthest_node;
            visited[farthest_node] = 1;

            while (count < solution_size)
            {
                int best_nodes[num_nodes];
                int best_positions[num_nodes];
                int num_best = 0;
                int min_increase = INT_MAX;

                for (int j = 0; j < num_nodes; j++)
                {
                    if (!visited[j])
                    {
                        for (int pos = 0; pos < count; pos++)
                        {
                            int next_pos = (pos + 1) % count;
                            int prev_node = current_solution[pos];
                            int next_node = current_solution[next_pos];
                            int increase = distances[prev_node][j] + distances[j][next_node] - distances[prev_node][next_node] + costs[j];

                            if (increase < min_increase)
                            {
                                min_increase = increase;
                                num_best = 0;
                                best_nodes[num_best] = j;
                                best_positions[num_best++] = next_pos;
                            }
                            else if (increase == min_increase)
                            {
                                best_nodes[num_best] = j;
                                best_positions[num_best++] = next_pos;
                            }
                        }
                    }
                }

                if (num_best == 0)
                {
                    break;
                }

                // Randomly select one of the best nodes and positions
                int rand_index = rand() % num_best;
                int best_node = best_nodes[rand_index];
                int best_position = best_positions[rand_index];

                // Insert best_node at best_position
                for (int m = count; m > best_position; m--)
                {
                    current_solution[m] = current_solution[m - 1];
                }
                current_solution[best_position] = best_node;
                count++;
                visited[best_node] = 1;
            }

            // Check if valid solution
            if (!is_valid_solution(current_solution, count, num_nodes))
            {
                fprintf(stderr, "Error: The generated solution is not valid\n");
            }

            int current_cost = calculate_cost(current_solution, count, distances, costs);
            totalCost += current_cost;

            // Update best
            if (current_cost < bestCost)
            {
                bestCost = current_cost;
                if (bestSolution)
                    free(bestSolution);
                bestSolution = (int *)malloc(count * sizeof(int));
                if (bestSolution)
                {
                    memcpy(bestSolution, current_solution, count * sizeof(int));
                    bestSolutionSize = count;
                }
            }

            // Update worst
            if (current_cost > worstCost)
            {
                worstCost = current_cost;
                if (worstSolution)
                    free(worstSolution);
                worstSolution = (int *)malloc(count * sizeof(int));
                if (worstSolution)
                {
                    memcpy(worstSolution, current_solution, count * sizeof(int));
                    worstSolutionSize = count;
                }
            }

            free(visited);
            free(current_solution);
        }
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

// ------------------ Greedy2Regret Algorithm ------------------

Greedy2Regret *create_Greedy2Regret()
{
    Greedy2Regret *gr = (Greedy2Regret *)malloc(sizeof(Greedy2Regret));
    if (!gr)
    {
        fprintf(stderr, "Error: Memory allocation failed for Greedy2Regret\n");
        return NULL;
    }
    gr->base.name = "Greedy2Regret";
    gr->base.solve = Greedy2Regret_solve;
    return gr;
}

static Result Greedy2Regret_solve(Algo *algo, const int **distances, int num_nodes, const int *costs, int num_solutions)
{
    int solution_size = (num_nodes + 1) / 2;

    int total_iterations = num_nodes * num_solutions;

    int bestCost = INT_MAX;
    int worstCost = INT_MIN;
    long long totalCost = 0;

    int *bestSolution = NULL;
    int bestSolutionSize = 0;
    int *worstSolution = NULL;
    int worstSolutionSize = 0;

    // For each node as starting point
    for (int start_node = 0; start_node < num_nodes; start_node++)
    {
        // Generate num_solutions starting from this node
        for (int s = 0; s < num_solutions; s++)
        {
            int *current_solution = (int *)malloc((solution_size + 1) * sizeof(int)); // +1 for safe insertion
            if (!current_solution)
            {
                fprintf(stderr, "Error: Memory allocation failed in Greedy2Regret_solve\n");
                Result res = {INT_MAX, INT_MIN, 0.0, NULL, 0, NULL, 0};
                return res;
            }

            int current_size = 0;
            char *visited = (char *)calloc(num_nodes, sizeof(char));
            if (!visited)
            {
                fprintf(stderr, "Error: Memory allocation failed in Greedy2Regret_solve (visited)\n");
                free(current_solution);
                Result res = {INT_MAX, INT_MIN, 0.0, NULL, 0, NULL, 0};
                return res;
            }

            // Start with start_node
            current_solution[current_size++] = start_node;
            visited[start_node] = 1;

            // Find the farthest node to start forming a cycle
            int max_distance = -1;
            int farthest_nodes[num_nodes];
            int num_farthest = 0;
            for (int j = 0; j < num_nodes; j++)
            {
                if (!visited[j])
                {
                    int distance = distances[start_node][j];
                    if (distance > max_distance)
                    {
                        max_distance = distance;
                        num_farthest = 0;
                        farthest_nodes[num_farthest++] = j;
                    }
                    else if (distance == max_distance)
                    {
                        farthest_nodes[num_farthest++] = j;
                    }
                }
            }

            if (num_farthest == 0)
            {
                free(visited);
                free(current_solution);
                continue;
            }

            // Randomly select one of the farthest nodes
            int rand_index = rand() % num_farthest;
            int farthest_node = farthest_nodes[rand_index];

            current_solution[current_size++] = farthest_node;
            visited[farthest_node] = 1;

            // Build the solution
            while (current_size < solution_size)
            {
                int max_regret = INT_MIN;
                int candidate_nodes[num_nodes];
                int candidate_positions[num_nodes];
                int num_candidates = 0;

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

                    if (regret > max_regret)
                    {
                        max_regret = regret;
                        num_candidates = 0;
                        candidate_nodes[num_candidates] = k;
                        candidate_positions[num_candidates++] = best_pos;
                    }
                    else if (regret == max_regret)
                    {
                        candidate_nodes[num_candidates] = k;
                        candidate_positions[num_candidates++] = best_pos;
                    }
                }

                if (num_candidates == 0)
                {
                    break;
                }

                // Randomly select one of the candidates
                int rand_index = rand() % num_candidates;
                int insert_node = candidate_nodes[rand_index];
                int insert_index = candidate_positions[rand_index];

                // Insert insert_node at insert_index in current_solution
                for (int m = current_size; m > insert_index; m--)
                {
                    current_solution[m] = current_solution[m - 1];
                }
                current_solution[insert_index] = insert_node;
                current_size++;
                visited[insert_node] = 1;
            }

            // Check if valid solution
            if (!is_valid_solution(current_solution, current_size, num_nodes))
            {
                fprintf(stderr, "Error: The generated solution is not valid\n");
            }

            // Calculate cost
            int current_cost = calculate_cost(current_solution, current_size, distances, costs);
            totalCost += current_cost;

            // Update best
            if (current_cost < bestCost)
            {
                bestCost = current_cost;
                if (bestSolution)
                    free(bestSolution);
                bestSolution = (int *)malloc(current_size * sizeof(int));
                if (bestSolution)
                {
                    memcpy(bestSolution, current_solution, current_size * sizeof(int));
                    bestSolutionSize = current_size;
                }
            }

            // Update worst
            if (current_cost > worstCost)
            {
                worstCost = current_cost;
                if (worstSolution)
                    free(worstSolution);
                worstSolution = (int *)malloc(current_size * sizeof(int));
                if (worstSolution)
                {
                    memcpy(worstSolution, current_solution, current_size * sizeof(int));
                    worstSolutionSize = current_size;
                }
            }

            free(visited);
            free(current_solution);
        }
    }

    // Calculate average cost
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


// ------------------ Greedy2RegretWeighted Algorithm ------------------

Greedy2RegretWeighted *create_Greedy2RegretWeighted()
{
    Greedy2RegretWeighted *grw = (Greedy2RegretWeighted *)malloc(sizeof(Greedy2RegretWeighted));
    if (!grw)
    {
        fprintf(stderr, "Error: Memory allocation failed for Greedy2RegretWeighted\n");
        return NULL;
    }
    grw->base.name = "Greedy2RegretWeighted";
    grw->base.solve = Greedy2RegretWeighted_solve;
    return grw;
}

static Result Greedy2RegretWeighted_solve(Algo *algo, const int **distances, int num_nodes, const int *costs, int num_solutions)
{
    int solution_size = (num_nodes + 1) / 2;

    int total_iterations = num_nodes * num_solutions;

    int bestCost = INT_MAX;
    int worstCost = INT_MIN;
    long long totalCost = 0;

    int *bestSolution = NULL;
    int bestSolutionSize = 0;
    int *worstSolution = NULL;
    int worstSolutionSize = 0;

    double weight1 = 1.0; // weight for regret
    double weight2 = 1.0; // weight for cost increase

    // For each node as starting point
    for (int start_node = 0; start_node < num_nodes; start_node++)
    {
        // Generate num_solutions starting from this node
        for (int s = 0; s < num_solutions; s++)
        {
            int *current_solution = (int *)malloc((solution_size + 1) * sizeof(int)); // +1 for safe insertion
            if (!current_solution)
            {
                fprintf(stderr, "Error: Memory allocation failed in Greedy2RegretWeighted_solve\n");
                Result res = {INT_MAX, INT_MIN, 0.0, NULL, 0, NULL, 0};
                return res;
            }

            int current_size = 0;
            char *visited = (char *)calloc(num_nodes, sizeof(char));
            if (!visited)
            {
                fprintf(stderr, "Error: Memory allocation failed in Greedy2RegretWeighted_solve (visited)\n");
                free(current_solution);
                Result res = {INT_MAX, INT_MIN, 0.0, NULL, 0, NULL, 0};
                return res;
            }

            // Start with start_node
            current_solution[current_size++] = start_node;
            visited[start_node] = 1;

            // Find the farthest node to start forming a cycle
            int max_distance = -1;
            int farthest_nodes[num_nodes];
            int num_farthest = 0;
            for (int j = 0; j < num_nodes; j++)
            {
                if (!visited[j])
                {
                    int distance = distances[start_node][j];
                    if (distance > max_distance)
                    {
                        max_distance = distance;
                        num_farthest = 0;
                        farthest_nodes[num_farthest++] = j;
                    }
                    else if (distance == max_distance)
                    {
                        farthest_nodes[num_farthest++] = j;
                    }
                }
            }

            if (num_farthest == 0)
            {
                free(visited);
                free(current_solution);
                continue;
            }

            // Randomly select one of the farthest nodes
            int rand_index = rand() % num_farthest;
            int farthest_node = farthest_nodes[rand_index];

            current_solution[current_size++] = farthest_node;
            visited[farthest_node] = 1;

            // Build the solution
            while (current_size < solution_size)
            {
                double max_score = -1e9;
                int candidate_nodes[num_nodes];
                int candidate_positions[num_nodes];
                int num_candidates = 0;

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

                    // Compute score
                    double score = weight1 * regret - weight2 * smallest_cost;

                    if (score > max_score)
                    {
                        max_score = score;
                        num_candidates = 0;
                        candidate_nodes[num_candidates] = k;
                        candidate_positions[num_candidates++] = best_pos;
                    }
                    else if (score == max_score)
                    {
                        candidate_nodes[num_candidates] = k;
                        candidate_positions[num_candidates++] = best_pos;
                    }
                }

                if (num_candidates == 0)
                {
                    break;
                }

                // Randomly select one of the candidates
                int rand_index = rand() % num_candidates;
                int insert_node = candidate_nodes[rand_index];
                int insert_index = candidate_positions[rand_index];

                // Insert insert_node at insert_index in current_solution
                for (int m = current_size; m > insert_index; m--)
                {
                    current_solution[m] = current_solution[m - 1];
                }
                current_solution[insert_index] = insert_node;
                current_size++;
                visited[insert_node] = 1;
            }

            // Check if valid solution
            if (!is_valid_solution(current_solution, current_size, num_nodes))
            {
                fprintf(stderr, "Error: The generated solution is not valid\n");
            }

            // Calculate cost
            int current_cost = calculate_cost(current_solution, current_size, distances, costs);
            totalCost += current_cost;

            // Update best
            if (current_cost < bestCost)
            {
                bestCost = current_cost;
                if (bestSolution)
                    free(bestSolution);
                bestSolution = (int *)malloc(current_size * sizeof(int));
                if (bestSolution)
                {
                    memcpy(bestSolution, current_solution, current_size * sizeof(int));
                    bestSolutionSize = current_size;
                }
            }

            // Update worst
            if (current_cost > worstCost)
            {
                worstCost = current_cost;
                if (worstSolution)
                    free(worstSolution);
                worstSolution = (int *)malloc(current_size * sizeof(int));
                if (worstSolution)
                {
                    memcpy(worstSolution, current_solution, current_size * sizeof(int));
                    worstSolutionSize = current_size;
                }
            }

            free(visited);
            free(current_solution);
        }
    }

    // Calculate average cost
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

// ------------------ Utility Functions ------------------

// Function to free a Result structure
void free_Result(Result res)
{
    if (res.bestSolution)
        free(res.bestSolution);
    if (res.worstSolution)
        free(res.worstSolution);
}

// Function to print the Result with cost breakdown
void print_Result(const Result res, const int **distances, const int *costs)
{
    printf("Best cost: %d\n", res.bestCost);
    printf("Worst cost: %d\n", res.worstCost);
    printf("Average cost: %.2lf\n", res.averageCost);

    int path_length = 0;
    int node_costs = 0;
    calculate_cost_breakdown(res.bestSolution, res.bestSolutionSize, distances, costs, &path_length, &node_costs);

    printf("Best solution total cost breakdown:\n");
    printf(" - Path length: %d\n", path_length);
    printf(" - Node costs: %d\n", node_costs);

    // Verify that total cost matches
    int total_cost = path_length + node_costs;
    if (total_cost != res.bestCost)
    {
        printf("Warning: Total cost (%d) does not match reported best cost (%d).\n", total_cost, res.bestCost);
    }

    printf("Best solution: ");
    for (int i = 0; i < res.bestSolutionSize; i++)
    {
        printf("%d ", res.bestSolution[i]);
    }
    printf("\n");
}
