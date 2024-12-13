#include "LocalSearchSolver.h"

#include "RandomSolution.h"
#include "Utils.h"


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
    vector<string> search_methods = {"STEEPEST"};
    vector<string> neigh_methods = {"TWO_EDGES"};
    vector<string> init_solutions = {"RANDOM_INIT"};

    for (auto instance : instances)
    {
        RandomSolution initial_solution = RandomSolution();
        initial_solution.generate(200, 100);
        LocalSearchSolver lss = LocalSearchSolver(instance, 0.5, initial_solution);
        for (auto init_solution : init_solutions)
            {
            for (auto search_method : search_methods)
            {
                for (auto neigh_method : neigh_methods)
                {
                    vector<int> best_evaluations;
                    vector<double> generation_times;
                    for (int i = 0; i < 200; i++)
                    {
                        if(init_solution == "RANDOM_INIT"){
                            RandomSolution new_initial_solution = RandomSolution();
                            new_initial_solution.generate(200, 100);
                            lss.set_initial_solution(&new_initial_solution);
                        } else {
                            GreedyCycle *new_initial_solution = new GreedyCycle();
                            new_initial_solution->generate(lss.get_distance_matrix(), lss.get_costs() , i, 100);
                            lss.set_initial_solution(new_initial_solution);
                        }


                        int generation_time = measure_generation_time(neigh_method, search_method,
                                                                    &lss, &LocalSearchSolver::run_basic);
                        generation_times.push_back(generation_time);
                        int eval = lss.get_best_solution_eval();
                        best_evaluations.push_back(eval);

                    }
                    string dir = "lab6/solutions/" + instance.substr(5, 4) + "/";
                    string filename = init_solution + "_" + neigh_method + "_EXCHANGE_" + search_method + ".csv";
                    lss.write_best_to_csv(dir + filename);
                    cout << search_method << " " << neigh_method << " " << init_solution << endl;
                    int min_e, max_e;
                    double min_t, avg_t, max_t, avg_e;
                    calculate_stats(&best_evaluations, &min_e, &avg_e, &max_e);
                    cout << "EVAL " << avg_e << " (" << min_e << "-" << max_e << ")" << endl;
                    calculate_stats(&generation_times, &min_t, &avg_t, &max_t);
                    cout << "TIME " << avg_t << " (" << min_t << "-" << max_t << ")" << endl;
                }
            }
        }
    }
}

int main()
{

    run_experiment();

    return 0;
}
