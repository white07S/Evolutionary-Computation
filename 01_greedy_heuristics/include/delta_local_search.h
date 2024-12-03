#ifndef DELTA_LOCAL_SEARCH_H
#define DELTA_LOCAL_SEARCH_H

#include "utils.h"
#include "algorithms.h"

// Define DeltaLocalSearch struct
// delta_local_search.h
typedef struct {
    Algo base;
    int method_index;
    int* initial_solution;       // Pointer to the initial solution
    int initial_solution_size;   // Size of the initial solution
} DeltaLocalSearch;

// Function to create a DeltaLocalSearch algorithm
DeltaLocalSearch* create_DeltaLocalSearch(int method_index, int* initial_solution, int initial_solution_size);

#endif // DELTA_LOCAL_SEARCH_H
