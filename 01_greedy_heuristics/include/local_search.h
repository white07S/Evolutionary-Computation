#ifndef LOCAL_SEARCH_H
#define LOCAL_SEARCH_H

#include "utils.h"
#include "algorithms.h"

// Define LocalSearch struct
typedef struct
{
    Algo base;
    int local_search_type;      // 0 for steepest, 1 for greedy
    int intra_route_move_type;  // 0 for two-nodes exchange, 1 for two-edges exchange
    int starting_solution_type; // 0 for random starting solution, 1 for greedy heuristic
    int method_index;           // For identification purposes
} LocalSearch;

// Function to create a LocalSearch algorithm
LocalSearch* create_LocalSearch(int local_search_type, int intra_route_move_type, int starting_solution_type, int method_index);

Result perform_local_search(int* current_solution, int solution_size, const int** distances, const int* costs, int num_nodes);


#endif // LOCAL_SEARCH_H
