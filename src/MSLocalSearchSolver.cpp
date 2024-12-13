#include "MSLocalSearchSolver.h"
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

using namespace std;
using namespace N;

MSLocalSearchSolver::MSLocalSearchSolver(string instance_filename,
                                         double fraction_nodes,
                                         Solution initial_solution)
    : LocalSearchSolver(instance_filename, fraction_nodes, initial_solution)
{
    this->i_filename = instance_filename;
    this->f_nodes = fraction_nodes;
}

void MSLocalSearchSolver::reset()
{
    // Set new random solution
    RandomSolution new_initial_solution = RandomSolution();
    new_initial_solution.generate(200, 100);
    this->i_solution = new_initial_solution;
    this->i_solution.set_nodes(new_initial_solution.get_nodes());
    this->i_solution.set_selected(new_initial_solution.get_selected());

    this->best_solution = new_initial_solution;
    this->best_sol_evaluation = 1000000;
}

void MSLocalSearchSolver::set_best_solution(Solution new_best)
{
    this->best_solution = new_best;
    this->best_solution.set_nodes(new_best.get_nodes());
    this->best_solution.set_selected(new_best.get_selected());
}

void MSLocalSearchSolver::run()
{
    LocalSearchSolver solver = LocalSearchSolver(this->i_filename,
                                                 this->f_nodes,
                                                 this->i_solution);
    for (int i = 0; i < 200; i++)
    {
        RandomSolution new_initial_solution = RandomSolution();
        new_initial_solution.generate(200, 100);
        solver.set_initial_solution_copy(new_initial_solution);

        solver.run_basic("TWO_EDGES", "STEEPEST");
        int solver_best_eval = solver.get_best_solution_eval();
        if (solver_best_eval < this->best_sol_evaluation)
        {
            this->set_best_solution(solver.get_best_full_solution());
            this->best_sol_evaluation = solver_best_eval;
        }
    }
    cout << "Best found in run of MSLS: " << this->best_sol_evaluation << endl;
}