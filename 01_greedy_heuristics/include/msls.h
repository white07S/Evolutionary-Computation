#ifndef MSLS_H
#define MSLS_H

#include "algorithms.h"

/**
 * @brief Structure for Multiple Start Local Search (MSLS) algorithm.
 * Inherits from the base Algo structure.
 */
typedef struct {
    Algo base;              // Base algorithm structure
    int num_iterations;    // Number of local search iterations
} MSLS;

/**
 * @brief Creates an instance of the MSLS algorithm.
 *
 * @param num_iterations Number of local search iterations to perform.
 * @return Pointer to the created MSLS instance.
 */
MSLS* create_MSLS(int num_iterations);

#endif // MSLS_H
