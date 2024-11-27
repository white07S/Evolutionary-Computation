#include "LSNLocalSearchSolver.h"
#include "RandomSolution.h"

#include <chrono>
#include <iostream>
#include <algorithm>

namespace LS {

    LSNLocalSearchSolver::LSNLocalSearchSolver(const std::string& instanceFilename, double fractionNodes, const Solution& initialSolution)
        : LocalSearchSolver(instanceFilename, fractionNodes, initialSolution),
          initialSolution(initialSolution),
          fractionNodes(fractionNodes),
          instanceFilename(instanceFilename)
    {
    }

    void LSNLocalSearchSolver::setBestSolution(const Solution& newBest)
    {
        bestSolution = newBest;
        bestSolution.setNodes(newBest.getNodes());
        bestSolution.setSelectedNodes(newBest.getSelectedNodes());
    }

    double LSNLocalSearchSolver::getAverageIterations()
    {
        double avg = Utils::mean(iterationCounts);
        iterationCounts.clear();
        return avg;
    }

    void LSNLocalSearchSolver::run(double timeLimitMicroseconds, bool innerLocalSearch)
    {
        LocalSearchSolver solver(instanceFilename, fractionNodes, initialSolution);

        RandomSolution newInitialSolution;
        newInitialSolution.generate(totalNodes, numNodes);
        solver.setInitialSolutionCopy(newInitialSolution);
        auto start = std::chrono::steady_clock::now();

        // Run local search on the initial solution
        solver.runBasic("TWO_EDGES", "STEEPEST");

        // Set the best found solution as best for LSNLS
        bestSolutionEvaluation = solver.getBestSolutionEval();
        setBestSolution(solver.getBestFullSolution());

        std::cout << bestSolution.evaluate(distanceMatrix, costs) << std::endl;

        int counter = 0;
        while (true)
        {
            auto now = std::chrono::steady_clock::now();
            double elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - start).count();
            if (elapsed > timeLimitMicroseconds)
            {
                break;
            }
            counter++;

            // Destroy and repair current best solution
            solver.destroyAndRepairBestSolution();

            if (innerLocalSearch)
            {
                solver.runBasic("TWO_EDGES", "STEEPEST");
            }

            int solverBestEval = solver.getBestSolutionEval();
            if (solverBestEval < bestSolutionEvaluation)
            {
                setBestSolution(solver.getBestFullSolution());
                bestSolutionEvaluation = solverBestEval;
            }
        }
        iterationCounts.emplace_back(counter);
        std::cout << "Best found in run of LSNLS: " << bestSolutionEvaluation << std::endl;
    }

}
