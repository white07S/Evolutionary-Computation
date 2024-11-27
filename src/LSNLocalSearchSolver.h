#ifndef LSN_LOCAL_SEARCH_SOLVER_H
#define LSN_LOCAL_SEARCH_SOLVER_H

#include "LocalSearchSolver.h"
#include "Solution.h"
#include "Utils.h"
namespace LS {

    class LSNLocalSearchSolver : public LocalSearchSolver {
    private:
        Solution initialSolution;
        double fractionNodes;
        std::string instanceFilename;
        std::vector<int> iterationCounts;

    public:
        LSNLocalSearchSolver(const std::string& instanceFilename, double fractionNodes, const Solution& initialSolution);

        void setBestSolution(const Solution& newBest);
        double getAverageIterations();

        void run(double timeLimitMicroseconds, bool innerLocalSearch);
    };

}

#endif // LSN_LOCAL_SEARCH_SOLVER_H
