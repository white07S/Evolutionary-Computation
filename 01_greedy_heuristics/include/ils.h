#ifndef ILS_H
#define ILS_H

#include "algorithms.h"

/**
 * @brief Structure for Iterated Local Search (ILS) algorithm.
 * Inherits from the base Algo structure.
 */
typedef struct {
    Algo base;              // Base algorithm structure
    int max_time_ms;       // Maximum running time in milliseconds
    int perturbation_strength; // Number of perturbation moves
} ILS;

/**
 * @brief Creates an instance of the ILS algorithm.
 *
 * @param max_time_ms Maximum running time in milliseconds.
 * @param perturbation_strength Number of moves to perturb the solution.
 * @return Pointer to the created ILS instance.
 */
ILS* create_ILS(int max_time_ms, int perturbation_strength);

#endif // ILS_H
