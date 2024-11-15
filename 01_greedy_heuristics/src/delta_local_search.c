#include "delta_local_search.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>

// Define Move structure
// typedef struct
// {
//     int i;
//     int j;
//     int type; // 0 for intra-route (2-opt), 1 for inter-route
//     int delta;
//     int edge_u1, edge_u2; // Edge u1-u2 to be removed
//     int edge_v1, edge_v2; // Edge v1-v2 to be removed
//     int reversed; // 1 if edges are reversed, 0 otherwise
// } Move;

// Function prototypes
static Result DeltaLocalSearch_solve(Algo *algo, const int **distances, int num_nodes, const int *costs, int num_solutions);
static int delta_two_edges_exchange(const int* solution, int solution_size, const int** distances, int i, int j);
static int delta_inter_route_exchange(const int* solution, int solution_size, const int** distances, const int* costs, int i, int node_j);
static void reverse_segment(int* solution, int start, int end, int solution_size);
static int is_in_solution(int node, const int* solution, int solution_size);
static void apply_move(int* solution, int solution_size, Move* move, int* predecessor, int* successor);
static int update_move(Move* move, const int* predecessor, const int* successor);

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

        // Initialize list of improving moves (LM)
        // Move* LM = NULL;
        // int LM_size = 0;
        // int LM_capacity = 0;
        SortedDoublyLinkedList LM;
        initList(&LM);

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

        // Perform steepest local search with LM
        int current_cost = calculate_cost(current_solution, solution_size, distances, costs);

        while (1)
        {
            int found_improving_move = 0;

            // First, check the LM
            // int lm_index = 0;
            Node* node = LM.head;
            // while (lm_index < LM_size)
            while (node != NULL)
            {
                // Move* move = &LM[lm_index];
                Move* move = &node->move;



                int update_status = update_move(move, predecessor, successor);

                if (update_status == -1)
                {
                    // Remove move from LM
                    // LM[lm_index] = LM[LM_size - 1];
                    // LM_size--;
                    Node* next_node = node->next;
                    deleteNode(&LM, node);
                    node = next_node;
                    continue;
                }
                else if (update_status == 0)
                {
                    // Edges reversed or mixed orientation, leave move in LM
                    // lm_index++;
                    node = node->next;
                    continue;
                }
                else if (update_status == 1)
                {
                    // Apply the move
                    apply_move(current_solution, solution_size, move, predecessor, successor);
                    current_cost += move->delta;

                    // Update in_solution array
                    if (move->type == 1)
                    {
                        int old_node = move->edge_u2; // node_i
                        int new_node = move->j;
                        in_solution[old_node] = 0;
                        in_solution[new_node] = 1;
                    }

                    // Remove move from LM
                    // LM[lm_index] = LM[LM_size - 1];
                    // LM_size--;

                    Node* next_node = node->next;
                    deleteNode(&LM, node);
                    node = next_node;
                    found_improving_move = 1;
                    break;
                }
            }

            if (found_improving_move)
            {
                continue; // Go back to LM
            }

            // If no improving move in LM, evaluate new moves

            // Evaluate all possible moves
            int best_delta = 0;
            Move new_best_move;
            int delta = 0;

            
            // Intra-route moves (2-opt)
            for (int i = 0; i < solution_size; i++)
            {
                for (int j = i + 2; j < solution_size + (i > 0 ? 0 : -1); j++)
                {
                    int jj = j % solution_size;
                    if (i == jj)
                        continue;

                    delta = delta_two_edges_exchange(current_solution, solution_size, distances, i, jj);

                    // FIXME: each move with delta < 0 should be added to 
                    // the LM. The best move should be kept track of, so that
                    // it can be applied after evaluating all the moves 
                    if (delta < best_delta)
                    {
                        best_delta = delta;
                        new_best_move.i = i;
                        new_best_move.j = jj;
                        new_best_move.type = 0;
                        new_best_move.delta = delta;
                        new_best_move.reversed = 0;

                        // Save the edges involved
                        new_best_move.edge_u1 = current_solution[i];
                        new_best_move.edge_u2 = current_solution[(i + 1) % solution_size];
                        new_best_move.edge_v1 = current_solution[jj];
                        new_best_move.edge_v2 = current_solution[(jj + 1) % solution_size];
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

                        // FIXME: each move with delta < 0 should be added to 
                        // the LM. The best move should be kept track of, so that
                        // it can be applied after evaluating all the moves 
                        if (delta < best_delta)
                        {
                            best_delta = delta;
                            new_best_move.i = i;
                            new_best_move.j = node_j;
                            new_best_move.type = 1;
                            new_best_move.delta = delta;
                            new_best_move.reversed = 0;

                            // Save the edges involved
                            int prev_i_node = predecessor[current_solution[i]];
                            int next_i_node = successor[current_solution[i]];

                            new_best_move.edge_u1 = prev_i_node;
                            new_best_move.edge_u2 = current_solution[i];
                            new_best_move.edge_v1 = current_solution[i];
                            new_best_move.edge_v2 = next_i_node;
                        }
                    }
                }
            }

            if (best_delta < 0)
            {
                // Apply the best move
                apply_move(current_solution, solution_size, &new_best_move, predecessor, successor);
                current_cost += best_delta;

                // Update in_solution array
                if (new_best_move.type == 1)
                {
                    int old_node = new_best_move.edge_u2; // node_i
                    int new_node = new_best_move.j;
                    in_solution[old_node] = 0;
                    in_solution[new_node] = 1;
                }

                // Add the move to LM
                // if (LM_size == LM_capacity)
                // {
                //     LM_capacity = LM_capacity == 0 ? 10 : LM_capacity * 2;
                //     LM = (Move*)realloc(LM, LM_capacity * sizeof(Move));
                //     if (!LM)
                //     {
                //         fprintf(stderr, "Error: Memory allocation failed for LM\n");
                //         free(current_solution);
                //         free(in_solution);
                //         free(successor);
                //         free(predecessor);
                //         Result res = {INT_MAX, INT_MIN, 0.0, NULL, 0, NULL, 0};
                //         return res;
                //     }
                // }
                // LM[LM_size++] = new_best_move;

                //FIXME: Here, the items should not be added to the LM but removed.
                insert(&LM, new_best_move);
            }
            else
            {
                // No improving move found
                break;
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

        // Free resources
        free(current_solution);
        free(in_solution);
        // free(LM);
        free(successor);
        free(predecessor);
        Node* node = LM.head;
        while (node != NULL) {
            Node* next_node = node->next;
            deleteNode(&LM, node);
            node = next_node;
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
        int node_i = solution[i];
        int node_ip1 = solution[(i + 1) % size];
        int node_j = solution[j];
        int node_jp1 = solution[(j + 1) % size];

        // Reverse the segment between (i+1) and j
        reverse_segment(solution, (i + 1) % size, j, solution_size);

        // Update predecessor and successor arrays
        int current_node = node_i;
        int next_node = solution[(i + 1) % size];
        successor[current_node] = next_node;
        predecessor[next_node] = current_node;

        int idx = (i + 1) % size;
        while (idx != (j + 1) % size)
        {
            current_node = solution[idx];
            next_node = solution[(idx + 1) % size];
            successor[current_node] = next_node;
            predecessor[next_node] = current_node;
            idx = (idx + 1) % size;
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
    int edges_same_direction = 0;
    int edges_reversed = 0;

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

    if (edge_u1_orientation == 1 && edge_v1_orientation == 1)
    {
        edges_same_direction = 1;
    }
    else if (edge_u1_orientation == -1 && edge_v1_orientation == -1)
    {
        edges_same_direction = 1;
        move->reversed = 1; // Both edges reversed
    }
    else
    {
        // Edges have mixed orientations
        return 0; // Leave move in LM but do not apply now
    }

    // Move can be applied
    return 1;
}


// functions for linked list implementation

// Function to create a new node with a Move
static Node* createNode(Move move) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->move = move;
    newNode->next = NULL;
    newNode->prev = NULL;
    return newNode;
}

// Function to initialize the doubly linked list
void initList(SortedDoublyLinkedList* list) {
    list->head = NULL;
}

// Function to insert a Move in sorted order (based on delta)
void insert(SortedDoublyLinkedList* list, Move move) {
    Node* newNode = createNode(move);
    if (list->head == NULL) {  // List is empty
        list->head = newNode;
        return;
    }

    // Insert in sorted order based on delta
    Node* current = list->head;
    Node* previous = NULL;

    // Traverse to find the correct insertion point
    while (current != NULL && current->move.delta < move.delta) {
        previous = current;
        current = current->next;
    }

    if (previous == NULL) {  // Insert at the beginning
        newNode->next = list->head;
        list->head->prev = newNode;
        list->head = newNode;
    } else if (current == NULL) {  // Insert at the end
        previous->next = newNode;
        newNode->prev = previous;
    } else {  // Insert in the middle
        previous->next = newNode;
        newNode->prev = previous;
        newNode->next = current;
        current->prev = newNode;
    }
}

// Function to get the smallest Move (first element in the list)
Move* getSmallest(SortedDoublyLinkedList* list) {
    if (list->head == NULL) {
        return NULL;
    }
    return &list->head->move;
}

// Function to delete the smallest Move
void deleteSmallest(SortedDoublyLinkedList* list) {
    if (list->head == NULL) {
        return;
    }

    Node* temp = list->head;
    list->head = list->head->next;
    if (list->head != NULL) {
        list->head->prev = NULL;
    }

    free(temp);  // Free the memory of the removed node
}


// Function to delete a specific node from the list
void deleteNode(SortedDoublyLinkedList* list, Node* node) {
    if (node == NULL) {
        return;
    }

    // Adjust head if necessary
    if (node == list->head) {
        list->head = node->next;
        if (list->head != NULL) {
            list->head->prev = NULL;
        }
    } else {
        // Adjust the previous node's next pointer
        if (node->prev != NULL) {
            node->prev->next = node->next;
        }
    }

    // Adjust the next node's previous pointer
    if (node->next != NULL) {
        node->next->prev = node->prev;
    }

    // Free the node memory
    free(node);        // Free the node itself

}


// Function to print the list (for debugging purposes)
void printList(SortedDoublyLinkedList* list) {
    Node* current = list->head;
    printf("List: ");
    while (current != NULL) {
        printf("{delta: %d, i: %d, j: %d} ", current->move.delta, current->move.i, current->move.j);
        current = current->next;
    }
    printf("\n");
}


