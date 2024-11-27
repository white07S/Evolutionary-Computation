#include "RandomSolution.h"
#include "Utils.h"

#include <random>
#include <set>
#include <ctime>
#include <iostream>

namespace LS {

    void RandomSolution::generate(int totalNodes, int desiredNumNodes)
    {
        // Clear existing nodes and reset
        nodes.clear();
        selectedNodes.clear();
        numNodes = 0;

        // Initialize random number generator
        std::mt19937 rngEngine(static_cast<unsigned int>(std::time(nullptr)));
        std::uniform_int_distribution<int> dist(0, totalNodes - 1);

        while (nodes.size() < static_cast<size_t>(desiredNumNodes))
        {
            int node = dist(rngEngine);
            // Use the inherited contains method to check if the node is already in the solution
            if (!contains(node))
            {
                addNode(node);
            }
        }
    }

}
