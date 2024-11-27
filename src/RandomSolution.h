#ifndef RANDOM_SOLUTION_H
#define RANDOM_SOLUTION_H

#include "Solution.h"

namespace LS {

    class RandomSolution : public Solution {
    public:
        void generate(int totalNodes, int numNodes);
    };

}

#endif // RANDOM_SOLUTION_H
