#ifndef DELTA_LOCAL_SEARCH_H
#define DELTA_LOCAL_SEARCH_H

#include "utils.h"
#include "algorithms.h"

// Define DeltaLocalSearch struct
typedef struct
{
    Algo base;
    int method_index; // For identification purposes
} DeltaLocalSearch;

// Function to create a DeltaLocalSearch algorithm
DeltaLocalSearch* create_DeltaLocalSearch(int method_index);

#endif // DELTA_LOCAL_SEARCH_H
