#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include "utils.h"

// Forward declaration of Algo structure
typedef struct Algo Algo;

// Define the Algo structure with a name and a solve function pointer
struct Algo
{
    const char *name;
    Result (*solve)(Algo *algo, const int **distances, int num_nodes, const int *costs, int num_solutions);
};

// Function declarations for algorithms

// RandomSearch Algorithm
typedef struct
{
    Algo base;
} RandomSearch;

RandomSearch *create_RandomSearch();

// NearestNeighboursEndInsert Algorithm
typedef struct
{
    Algo base;
} NearestNeighboursEndInsert;

NearestNeighboursEndInsert *create_NearestNeighboursEndInsert();

// NearestNeighboursAnywhereInsert Algorithm
typedef struct
{
    Algo base;
} NearestNeighboursAnywhereInsert;

NearestNeighboursAnywhereInsert *create_NearestNeighboursAnywhereInsert();

// GreedyCycle Algorithm
typedef struct
{
    Algo base;
} GreedyCycle;

GreedyCycle *create_GreedyCycle();

// Greedy2Regret Algorithm
typedef struct
{
    Algo base;
} Greedy2Regret;

Greedy2Regret *create_Greedy2Regret();

// Greedy2RegretWeighted Algorithm
typedef struct
{
    Algo base;
} Greedy2RegretWeighted;

Greedy2RegretWeighted *create_Greedy2RegretWeighted();

// Function to free a Result structure
void free_Result(Result res);

// Function to print the Result
void print_Result(const Result res, const int **distances, const int *costs);

#endif
