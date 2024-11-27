#include "BaseSolver.h"
#include "DistanceMatrix.h"
#include "Utils.h"

#include <algorithm>
#include <numeric>
#include <iostream>

namespace LS {

    BaseSolver::BaseSolver(const std::string& instanceFilename, double fractionNodes)
    {
        DistanceMatrix distanceMatrixCreator;
        distanceMatrixCreator.create(instanceFilename);
        distanceMatrix = distanceMatrixCreator.getDistanceMatrix();
        costs = distanceMatrixCreator.getCosts();
        totalNodes = costs.size();

        // Extract instance name from filename
        size_t delimiterPos = instanceFilename.find_last_of("/\\");
        std::string filename = (delimiterPos != std::string::npos) ? 
                                instanceFilename.substr(delimiterPos + 1) : 
                                instanceFilename;
        size_t dotPos = filename.find_last_of('.');
        instanceName = (dotPos != std::string::npos) ? 
                        filename.substr(0, dotPos) : 
                        filename;

        // Determine number of nodes to cover
        numNodes = static_cast<int>(totalNodes * fractionNodes);
    }

    int BaseSolver::getTotalNodes() const
    {
        return totalNodes;
    }

    int BaseSolver::getNumNodes() const
    {
        return numNodes;
    }

    const std::vector<std::vector<int>>& BaseSolver::getDistanceMatrix() const
    {
        return distanceMatrix;
    }

    const std::vector<int>& BaseSolver::getCosts() const
    {
        return costs;
    }

    void BaseSolver::printSolutionStats(const std::vector<int>& evaluations) const
    {
        int minEval = *std::min_element(evaluations.begin(), evaluations.end());
        int maxEval = *std::max_element(evaluations.begin(), evaluations.end());
        double avgEval = Utils::mean(evaluations);

        std::cout << "MIN " << minEval << " AVG " << avgEval << " MAX " << maxEval << std::endl;
    }

}
