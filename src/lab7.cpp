#include "LSNLocalSearchSolver.h"
#include "Utils.h"
#include "RandomSolution.h"

#include <vector>
#include <iostream>
#include <string>
#include <limits>
#include <filesystem>
#include <fstream>
namespace LS {

    // List of instances
    std::vector<std::string> instances = {
        "data/TSPA.csv",
        "data/TSPB.csv"
    };

    void runExperiment(bool innerLocalSearch)
    {
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        const int repetitions = 20   ;
        // Time limits in microseconds
        std::vector<double> timeLimits = { 20173.5*1000 , 22698.6*1000 };

        int instanceIdx = 0;
        for (const auto& instance : instances)
        {
            std::cout << "Processing Instance: " << instance.substr(instance.find_last_of("/\\") + 1, 4) << std::endl;

            double timeLimitMicroseconds = timeLimits[instanceIdx];
            ++instanceIdx;

            RandomSolution initialSolution;
            initialSolution.generate(200, 100); // Adjust parameters as needed

            LSNLocalSearchSolver lsnlss(instance, 0.5, initialSolution);
            std::vector<int> bestEvaluations;
            std::vector<double> generationTimes;
            Solution bestSol = initialSolution;
            int bestEval = std::numeric_limits<int>::max();

            for (int i = 0; i < repetitions; ++i)
            {
                auto runStart = std::chrono::steady_clock::now();
                lsnlss.run(timeLimitMicroseconds, innerLocalSearch);
                auto runEnd = std::chrono::steady_clock::now();

                double generationTime = std::chrono::duration_cast<std::chrono::microseconds>(runEnd - runStart).count();
                generationTimes.emplace_back(generationTime);

                int eval = lsnlss.getBestSolutionEval();
                bestEvaluations.emplace_back(eval);

                if (eval < bestEval)
                {
                    bestEval = eval;
                    bestSol.setNodes(lsnlss.getBestSolution());
                }
            }

            // Ensure output directory exists
            std::string instanceName = instance.substr(instance.find_last_of("/\\") + 1, 4);
            std::string dir = "lab7/solutions/" + instanceName + "/";
            std::filesystem::create_directories(dir);

            std::string filename = innerLocalSearch ? "LSNLS_INNER_LOCAL_SEARCH.txt" : "LSNLS_NO_INNER_LOCAL_SEARCH.txt";
            int totalCost = bestSol.evaluate(lsnlss.getDistanceMatrix(), lsnlss.getCosts());

            // Open the text file for writing
            std::ofstream outfile(dir + filename);
            if (!outfile.is_open()) {
                throw std::runtime_error("Could not open file for writing: " + (dir + filename));
            }
            outfile << "Best Solution:";
            for (const auto& node : bestSol.getNodes()) {
                outfile << " " << node;
            }
            outfile << "\n";

            // Write "Total Cost: <cost>"
            outfile << "Total Cost: " << totalCost << "\n";

            // Close the file
            outfile.close();

            // bestSol.writeToCSV(dir + filename + ".txt");


            int minEval, maxEval;
            double avgEval;
            Utils::calculateStats(bestEvaluations, minEval, avgEval, maxEval);
            std::cout << "EVAL LSNLS " << avgEval << " (" << minEval << "-" << maxEval << ")" << std::endl;

            double minTime, avgTime, maxTime;
            Utils::calculateStats(generationTimes, minTime, avgTime, maxTime);
            // Convert to milliseconds for output
            std::cout << "TIME LSNLS " << avgTime / 1000 << " ms (" << minTime / 1000 << "-" << maxTime / 1000 << " ms)" << std::endl;

            double avgIterations = lsnlss.getAverageIterations();
            std::cout << "Average number of iterations: " << avgIterations << std::endl;
        }
    }

}

int main()
{
    std::cout << "RUNNING WITHOUT INNER LOCAL SEARCH" << std::endl;
    bool innerLocalSearch = false;
    LS::runExperiment(innerLocalSearch);

    std::cout << std::endl << "RUNNING WITH INNER LOCAL SEARCH" << std::endl;
    innerLocalSearch = true;
    LS::runExperiment(innerLocalSearch);

    return 0;
}
