#ifndef BASE_SOLVER_H
#define BASE_SOLVER_H

#include <vector>
#include <string>

#include "Solution.h"
#include "DistanceMatrix.h"

namespace LS {

    class BaseSolver {
    protected:
        std::vector<std::vector<int>> distanceMatrix;
        std::vector<int> costs;
        int totalNodes;
        int numNodes;
        std::string instanceName;

    public:
        BaseSolver(const std::string& instanceFilename, double fractionNodes);
        
        int getTotalNodes() const;
        int getNumNodes() const;
        const std::vector<std::vector<int>>& getDistanceMatrix() const;
        const std::vector<int>& getCosts() const;

        void printSolutionStats(const std::vector<int>& evaluations) const;
    };

}

#endif // BASE_SOLVER_H
