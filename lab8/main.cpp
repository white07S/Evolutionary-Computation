#include "RandomSolution.h"
#include "Utils.h"
#include "LocalSearchSolver.h"

#include <vector>
#include <iostream>
#include <chrono>
#include <random>
#include <time.h>
#include <string>

using namespace std;
using namespace N;

vector<string> instances = {"data/TSPA.csv",
                            "data/TSPB.csv"};

void run_experiment()
{
    srand(time(NULL));
    for (auto instance : instances)
    {
        cout << instance.substr(5, 4) << endl;
        int rep = 1000;
        RandomSolution initial_solution = RandomSolution();
        initial_solution.generate(200, 100);

        LocalSearchSolver lss = LocalSearchSolver(instance, 0.5, initial_solution);
        vector<vector<int>> generated_solutions;
        vector<int> best_evaluations_ms;
        vector<double> generation_times_ms;
        for (int i = 0; i < rep; i++)
        {
            lss.reset();
            int generation_time = measure_generation_time(
                "TWO_EDGES", "GREEDY",
                &lss, &LocalSearchSolver::run_basic);
            generation_times_ms.push_back(generation_time);
            int eval = lss.get_best_solution_eval();
            best_evaluations_ms.push_back(eval);
            vector<int> best_solution = lss.get_best_solution();
            // Append solution evaluation at the end
            best_solution.push_back(eval);
            generated_solutions.push_back(best_solution);
        }
        int min_e_ms, max_e_ms;
        double min_t_ms, avg_t_ms, max_t_ms, avg_e_ms;
        calculate_stats(&best_evaluations_ms, &min_e_ms, &avg_e_ms, &max_e_ms);
        cout << "EVAL MSLS " << avg_e_ms << " (" << min_e_ms << "-" << max_e_ms << ")" << endl;
        calculate_stats(&generation_times_ms, &min_t_ms, &avg_t_ms, &max_t_ms);
        cout << "TIME MSLS " << avg_t_ms / 1000 << " (" << min_t_ms / 1000 << "-" << max_t_ms / 1000 << ")" << endl;

        write_matrix_to_csv(generated_solutions, "lab8/" + instance.substr(5, 4) + "_solutions.csv");
    }
}

int main()
{
    run_experiment();

    return 0;
}