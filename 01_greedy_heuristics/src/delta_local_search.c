#include "delta_local_search.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>

// Define Move structure
typedef struct
{
    int i;
    int j;
    int type; // 0 for intra-route (2-opt), 1 for inter-route
    int delta;
    int edge_u1, edge_u2; // Edge u1-u2 to be removed
    int edge_v1, edge_v2; // Edge v1-v2 to be removed
    int reversed; // 1 if edges are reversed, 0 otherwise
} Move;

// Define Priority Queue structure
typedef struct
{
    Move* moves; // Array of moves
    int size;    // Current number of moves
    int capacity; // Capacity of the moves array
} PriorityQueue;

// Function prototypes
static Result DeltaLocalSearch_solve(Algo *algo, const int **distances, int num_nodes, const int *costs, int num_solutions);
static int delta_two_edges_exchange(const int* solution, int solution_size, const int** distances, int i, int j);
static int delta_inter_route_exchange(const int* solution, int solution_size, const int** distances, const int* costs, int i, int node_j);
static void reverse_segment(int* solution, int start, int end, int solution_size);

static void apply_move(int* solution, int solution_size, Move* move, int* predecessor, int* successor);
static int update_move(Move* move, const int* predecessor, const int* successor);
static void evaluate_all_moves_and_add_to_LM(int* current_solution, int solution_size, const int** distances, const int* costs, const char* in_solution, int* predecessor, int* successor, int num_nodes, PriorityQueue* pq);

static void init_pq(PriorityQueue* pq, int capacity);
static void free_pq(PriorityQueue* pq);
static void insert_move_pq(PriorityQueue* pq, Move* move);
static Move* extract_min_move_pq(PriorityQueue* pq);

DeltaLocalSearch* create_DeltaLocalSearch(int method_index)
{
    DeltaLocalSearch* dls = (DeltaLocalSearch*)malloc(sizeof(DeltaLocalSearch));
    if (!dls)
    {
        fprintf(stderr, "Error: Memory allocation failed for DeltaLocalSearch\n");
        return NULL;
    }

    char* name = (char*)malloc(100 * sizeof(char));
    if (!name)
    {
        fprintf(stderr, "Error: Memory allocation failed for DeltaLocalSearch name\n");
        free(dls);
        return NULL;
    }
    snprintf(name, 100, "DeltaLocalSearch");

    dls->base.name = name;
    dls->base.solve = DeltaLocalSearch_solve;
    dls->method_index = method_index;
    return dls;
}

static Result DeltaLocalSearch_solve(Algo *algo, const int **distances, int num_nodes, const int *costs, int num_solutions)
{
    DeltaLocalSearch* dls = (DeltaLocalSearch*)algo;
    int solution_size = (num_nodes + 1) / 2; // Round up to select 50% of the nodes

    int total_iterations = num_solutions;
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
        // Generate random starting solution
        int* current_solution = (int*)malloc(solution_size * sizeof(int));
        if (!current_solution)
        {
            fprintf(stderr, "Error: Memory allocation failed in DeltaLocalSearch_solve\n");
            Result res = {INT_MAX, INT_MIN, 0.0, NULL, 0, NULL, 0};
            return res;
        }
        // Random starting solution
        int* all_nodes = (int*)malloc(num_nodes * sizeof(int));
        if (!all_nodes)
        {
            fprintf(stderr, "Error: Memory allocation failed in DeltaLocalSearch_solve\n");
            free(current_solution);
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
            fprintf(stderr, "Error: Memory allocation failed in DeltaLocalSearch_solve (in_solution)\n");
            free(current_solution);
            Result res = {INT_MAX, INT_MIN, 0.0, NULL, 0, NULL, 0};
            return res;
        }
        for (int i = 0; i < solution_size; i++)
        {
            in_solution[current_solution[i]] = 1;
        }

        // Initialize predecessor and successor arrays
        int* successor = (int*)malloc(num_nodes * sizeof(int));
        int* predecessor = (int*)malloc(num_nodes * sizeof(int));
        if (!successor || !predecessor)
        {
            fprintf(stderr, "Error: Memory allocation failed in DeltaLocalSearch_solve (successor/predecessor)\n");
            free(current_solution);
            free(in_solution);
            free(successor);
            free(predecessor);
            Result res = {INT_MAX, INT_MIN, 0.0, NULL, 0, NULL, 0};
            return res;
        }
        // Initialize successor and predecessor arrays
        for (int i = 0; i < solution_size; i++)
        {
            int node = current_solution[i];
            int next_node = current_solution[(i + 1) % solution_size];
            int prev_node = current_solution[(i - 1 + solution_size) % solution_size];
            successor[node] = next_node;
            predecessor[node] = prev_node;
        }

        // Initialize priority queue LM
        PriorityQueue LM;
        init_pq(&LM, 1000); // Initial capacity

        // Perform steepest local search with LM
        int current_cost = calculate_cost(current_solution, solution_size, distances, costs);

        int found_improving_move = 0;

        do
        {
            found_improving_move = 0;

            // Evaluate all possible moves and add improving moves to LM
            evaluate_all_moves_and_add_to_LM(current_solution, solution_size, distances, costs, in_solution, predecessor, successor, num_nodes, &LM);

            // Process LM
            while (LM.size > 0)
            {
                Move* move_ptr = extract_min_move_pq(&LM);
                if (!move_ptr)
                {
                    // Priority queue is empty
                    break;
                }

                Move* move = move_ptr;

                int update_status = update_move(move, predecessor, successor);

                if (update_status == -1)
                {
                    // Move is invalid, skip
                    free(move_ptr);
                    continue;
                }
                else if (update_status == 0)
                {
                    // Edges reversed or mixed orientation, skip
                    free(move_ptr);
                    continue;
                }
                else if (update_status == 1)
                {
                    // Apply the move
                    apply_move(current_solution, solution_size, move, predecessor, successor);
                    current_cost += move->delta;

                    // Update in_solution array if necessary
                    if (move->type == 1)
                    {
                        int old_node = move->edge_u2; // node_i
                        int new_node = move->j;
                        in_solution[old_node] = 0;
                        in_solution[new_node] = 1;
                    }

                    free(move_ptr);

                    found_improving_move = 1;
                    break;
                }
            }

            // If no improving move was found after checking the whole LM, exit loop
            if (!found_improving_move)
            {
                break;
            }

            // Clear LM before next iteration
            free_pq(&LM);
            init_pq(&LM, 1000);

        } while (found_improving_move);

        // Free LM
        free_pq(&LM);

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

        // Free resources
        free(current_solution);
        free(in_solution);
        free(successor);
        free(predecessor);
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

static void init_pq(PriorityQueue* pq, int capacity)
{
    pq->moves = (Move*)malloc(capacity * sizeof(Move));
    if (!pq->moves)
    {
        fprintf(stderr, "Error: Memory allocation failed for PriorityQueue\n");
        pq->size = 0;
        pq->capacity = 0;
        return;
    }
    pq->size = 0;
    pq->capacity = capacity;
}

static void free_pq(PriorityQueue* pq)
{
    if (pq->moves)
    {
        free(pq->moves);
        pq->moves = NULL;
    }
    pq->size = 0;
    pq->capacity = 0;
}

static void insert_move_pq(PriorityQueue* pq, Move* move)
{
    // Ensure capacity
    if (pq->size >= pq->capacity)
    {
        // Resize the array
        pq->capacity *= 2;
        pq->moves = (Move*)realloc(pq->moves, pq->capacity * sizeof(Move));
        if (!pq->moves)
        {
            fprintf(stderr, "Error: Memory allocation failed in insert_move_pq\n");
            return;
        }
    }

    // Insert the move at the end
    pq->moves[pq->size] = *move;
    pq->size++;

    // Heapify up
    int i = pq->size - 1;
    while (i > 0)
    {
        int parent = (i - 1) / 2;
        if (pq->moves[i].delta < pq->moves[parent].delta)
        {
            // Swap
            Move temp = pq->moves[i];
            pq->moves[i] = pq->moves[parent];
            pq->moves[parent] = temp;
            i = parent;
        }
        else
        {
            break;
        }
    }
}

static Move* extract_min_move_pq(PriorityQueue* pq)
{
    if (pq->size == 0)
    {
        return NULL;
    }

    Move* min_move = (Move*)malloc(sizeof(Move));
    if (!min_move)
    {
        fprintf(stderr, "Error: Memory allocation failed in extract_min_move_pq\n");
        return NULL;
    }

    *min_move = pq->moves[0];

    // Move last element to root and heapify down
    pq->moves[0] = pq->moves[pq->size - 1];
    pq->size--;

    int i = 0;
    while (i < pq->size)
    {
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        int smallest = i;

        if (left < pq->size && pq->moves[left].delta < pq->moves[smallest].delta)
        {
            smallest = left;
        }
        if (right < pq->size && pq->moves[right].delta < pq->moves[smallest].delta)
        {
            smallest = right;
        }
        if (smallest != i)
        {
            // Swap
            Move temp = pq->moves[i];
            pq->moves[i] = pq->moves[smallest];
            pq->moves[smallest] = temp;
            i = smallest;
        }
        else
        {
            break;
        }
    }

    return min_move;
}

static void evaluate_all_moves_and_add_to_LM(int* current_solution, int solution_size, const int** distances, const int* costs, const char* in_solution, int* predecessor, int* successor, int num_nodes, PriorityQueue* pq)
{
    int delta;
    Move move;

    // Intra-route moves (2-opt)
    for (int i = 0; i < solution_size; i++)
    {
        for (int j = i + 2; j < solution_size + (i > 0 ? 0 : -1); j++)
        {
            int jj = j % solution_size;
            if (i == jj)
                continue;

            delta = delta_two_edges_exchange(current_solution, solution_size, distances, i, jj);

            if (delta < 0)
            {
                move.i = i;
                move.j = jj;
                move.type = 0;
                move.delta = delta;
                move.reversed = 0;

                // Save the edges involved
                move.edge_u1 = current_solution[i];
                move.edge_u2 = current_solution[(i + 1) % solution_size];
                move.edge_v1 = current_solution[jj];
                move.edge_v2 = current_solution[(jj + 1) % solution_size];

                // Insert move into priority queue
                insert_move_pq(pq, &move);
            }
        }
    }

    // Inter-route moves
    for (int i = 0; i < solution_size; i++)
    {
        for (int node_j = 0; node_j < num_nodes; node_j++)
        {
            if (!in_solution[node_j])
            {
                delta = delta_inter_route_exchange(current_solution, solution_size, distances, costs, i, node_j);

                if (delta < 0)
                {
                    move.i = i;
                    move.j = node_j;
                    move.type = 1;
                    move.delta = delta;
                    move.reversed = 0;

                    // Save the edges involved
                    int node_i = current_solution[i];
                    int prev_i_node = predecessor[node_i];
                    int next_i_node = successor[node_i];

                    move.edge_u1 = prev_i_node;
                    move.edge_u2 = node_i;
                    move.edge_v1 = node_i;
                    move.edge_v2 = next_i_node;

                    // Insert move into priority queue
                    insert_move_pq(pq, &move);
                }
            }
        }
    }
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

static int delta_inter_route_exchange(const int* solution, int solution_size, const int** distances, const int* costs, int i, int node_j)
{
    int size = solution_size;
    int node_i = solution[i];
    int prev_i = solution[(i - 1 + size) % size];
    int next_i = solution[(i + 1) % size];

    int delta = 0;

    delta -= distances[prev_i][node_i];
    delta -= distances[node_i][next_i];
    delta += distances[prev_i][node_j];
    delta += distances[node_j][next_i];

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

static void apply_move(int* solution, int solution_size, Move* move, int* predecessor, int* successor)
{
    if (move->type == 0)
    {
        // Intra-route move (2-opt)
        int size = solution_size;
        int i = move->i;
        int j = move->j;

        // Reverse the segment between (i+1) and j
        reverse_segment(solution, (i + 1) % size, j, solution_size);

        // Update predecessor and successor arrays
        for (int idx = 0; idx < size; idx++)
        {
            int node = solution[idx];
            int next_node = solution[(idx + 1) % size];
            int prev_node = solution[(idx - 1 + size) % size];
            successor[node] = next_node;
            predecessor[node] = prev_node;
        }
    }
    else if (move->type == 1)
    {
        // Inter-route move
        int old_node = solution[move->i];
        int new_node = move->j;

        int prev_node = predecessor[old_node];
        int next_node = successor[old_node];

        // Update solution
        solution[move->i] = new_node;

        // Update predecessor and successor arrays
        successor[prev_node] = new_node;
        predecessor[new_node] = prev_node;

        successor[new_node] = next_node;
        predecessor[next_node] = new_node;

        // Remove old_node from predecessor and successor arrays
        predecessor[old_node] = -1;
        successor[old_node] = -1;
    }
}

static int update_move(Move* move, const int* predecessor, const int* successor)
{
    int edge_u1 = move->edge_u1;
    int edge_u2 = move->edge_u2;
    int edge_v1 = move->edge_v1;
    int edge_v2 = move->edge_v2;

    int edge_u1_succ = successor[edge_u1];
    int edge_v1_succ = successor[edge_v1];

    int edge_u1_pred = predecessor[edge_u1];
    int edge_v1_pred = predecessor[edge_v1];

    int edges_exist = 1;

    // Check if edges exist
    if ((edge_u1_succ == edge_u2) || (edge_u1_pred == edge_u2))
    {
        // Edge (edge_u1, edge_u2) exists
    }
    else
    {
        edges_exist = 0;
    }

    if ((edge_v1_succ == edge_v2) || (edge_v1_pred == edge_v2))
    {
        // Edge (edge_v1, edge_v2) exists
    }
    else
    {
        edges_exist = 0;
    }

    if (!edges_exist)
    {
        // At least one edge no longer exists
        return -1; // Remove move from LM
    }

    // Determine edge orientations
    int edge_u1_orientation = (edge_u1_succ == edge_u2) ? 1 : -1; // 1 for forward, -1 for reversed
    int edge_v1_orientation = (edge_v1_succ == edge_v2) ? 1 : -1;

    if (edge_u1_orientation == edge_v1_orientation)
    {
        // Edges have same orientation
        return 1; // Apply the move
    }
    else
    {
        // Edges have mixed orientations
        return 0; // Leave move in LM but do not apply now
    }
}
