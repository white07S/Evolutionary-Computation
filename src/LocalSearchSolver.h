#ifndef LOCAL_SEARCH_SOLVER_H
#define LOCAL_SEARCH_SOLVER_H

#include <vector>
#include <string>
#include <random>

#include "BaseSolver.h"
#include "Solution.h"

namespace LS {

    class LocalSearchSolver : public BaseSolver {
    protected:
        Solution bestSolution;
        int bestSolutionEvaluation;
        std::vector<int> iterator1;
        std::vector<int> iterator2;
        std::vector<int> iteratorLong;
        std::mt19937 rng;

    public:
        LocalSearchSolver(const std::string& instanceFilename, double fractionNodes, const Solution& initialSolution);

        void reset();
        void setInitialSolution(const Solution& newInitialSolution);
        void setInitialSolutionCopy(const Solution& newInitialSolution);
        void writeBestToCSV(const std::string& filename);
        int getBestSolutionEval() const;
        std::vector<int> getBestSolution() const;
        void greedyCycleRepair(std::vector<int>& tmpSol);
        Solution getBestFullSolution() const;
        Solution* getBestSolutionPtr();

        void perturbBestSolution(int n);
        void destroyAndRepairBestSolution();
        void destroyAndRepairBestSolutionV2();

        void runBasic(const std::string& neighborhoodMethod, const std::string& searchMethod);

        void findBestInterNeighbor(int& bestEval, int& exchangedNode, int& newNode, const std::string& searchMethod);
        void findBestIntraNeighborNodes(int& bestEval, int& firstNodeIdx, int& secondNodeIdx, const std::string& searchMethod);
        void findBestIntraNeighborEdges(int& outDelta, int& firstEdgeIdx, int& secondEdgeIdx, const std::string& searchMethod);

        void applyMove(const std::string& moveType, int arg1, int arg2);
    };

}

#endif // LOCAL_SEARCH_SOLVER_H
