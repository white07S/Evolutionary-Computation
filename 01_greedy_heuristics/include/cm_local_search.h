#ifndef CM_LOCAL_SEARCH_H
#define CM_LOCAL_SEARCH_H

#include "utils.h"
#include "algorithms.h"

// Define CM_LocalSearch struct
typedef struct
{
    Algo base;
    int candidate_list_size; // Number of candidate edges per node
} CM_LocalSearch;

// Function to create a CM_LocalSearch algorithm
CM_LocalSearch* create_CM_LocalSearch(int candidate_list_size);

#endif // CM_LOCAL_SEARCH_H
