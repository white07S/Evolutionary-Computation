#include "ILocalSearchSolver.h"
#include "LocalSearchSolver.h"
#include "Solution.h"
#include "Utils.h"

#include <string>
#include <set>
#include <iostream>
#include <limits>
#include <numeric>
#include <algorithm>
#include <utility>
#include <random>
#include <vector>

using namespace std;
using namespace N;

ILocalSearchSolver::ILocalSearchSolver(string instance_filename,
                                       double fraction_nodes,
                                       Solution initial_solution)
    : LocalSearchSolver(instance_filename, fraction_nodes, initial_solution)
{
    this->i_filename = instance_filename;
    this->f_nodes = fraction_nodes;
    this->i_solution = initial_solution;
}

void ILocalSearchSolver::set_best_solution(Solution new_best)
{
    this->best_solution = new_best;
    this->best_solution.set_nodes(new_best.get_nodes());
    this->best_solution.set_selected(new_best.get_selected());
}

double ILocalSearchSolver::get_avg_iter()
{
    double avg = mean(&this->iter_count);
    this->iter_count.clear();
    return avg;
}

void ILocalSearchSolver::run(double time)
{
    LocalSearchSolver solver = LocalSearchSolver(this->i_filename,
                                                 this->f_nodes,
                                                 this->i_solution);

    RandomSolution new_initial_solution = RandomSolution();
    new_initial_solution.generate(200, 100);
    // Generate initial solution
    solver.set_initial_solution_copy(new_initial_solution);
    auto start = std::chrono::steady_clock::now();
    // Run local search on the initial solution
    solver.run_basic("TWO_EDGES", "STEEPEST");

    // Set the best found solution as best for ILS
    this->best_sol_evaluation = solver.get_best_solution_eval();
    this->set_best_solution(solver.get_best_full_solution());

    cout << this->best_solution.evaluate(&this->dist_mat, &this->costs) << endl;
    int counter = 0;
    while (true)
    {
        auto end = std::chrono::steady_clock::now();
        double so_far = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        if (so_far > time)
        {
            break;
        }
        counter += 1;
        // Perturb current best solution
        solver.perturb_best_solution(4);

        solver.run_basic("TWO_EDGES", "STEEPEST");
        int solver_best_eval = solver.get_best_solution_eval();
        if (solver_best_eval < this->best_sol_evaluation)
        {
            this->set_best_solution(solver.get_best_full_solution());
            this->best_sol_evaluation = solver_best_eval;
        }
    }
    cout << counter << endl;
    this->iter_count.push_back(counter);
    cout << "Best found in run of ILS: " << this->best_sol_evaluation << endl;
}
