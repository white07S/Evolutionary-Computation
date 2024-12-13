#include "ProblemSolver.h"
#include "DistanceMatrixCreator.h"
#include "Solution.h"
#include "RandomSolution.h"
#include "NearestNeighbor.h"
#include "GreedyCycle.h"
#include "RegretGreedyCycle.h"
#include "Utils.h"

#include <iostream>
#include <algorithm>
#include <typeinfo>
#include <random>
#include <time.h>

using namespace std;
using namespace N;

ProblemSolver::ProblemSolver(string instance_filename, double fraction_nodes)
{
    // Class init

    DistanceMatrixCreator dmc;

    dmc.create(instance_filename);

    this->dist_mat = dmc.get_distance_matrix();
    this->costs = dmc.get_costs();
    this->total_nodes = this->costs.size();

    // Extract instance name
    string delimiter = "/";
    string token = instance_filename.substr(instance_filename.find(delimiter) + 1);
    delimiter = ".";
    token = token.substr(0, token.find(delimiter));
    this->instance_name = token;

    // srand(time(NULL));

    // n_nodes specifies how many nodes
    // should be covered in the solotion
    this->n_nodes = int(this->total_nodes * fraction_nodes);

    // cout << instance_name << endl;
}
vector<vector<int>> ProblemSolver::get_distance_matrix() { return this->dist_mat; }
std::vector<int> ProblemSolver::get_costs() {return this->costs; }

void ProblemSolver::generate_solutions(string method)
{

    // For temporal storage of solution scores
    std::vector<Solution *> solutions;
    std::vector<int> evaluations;

    // For temporal storage of a single score
    Solution *temp_sol;
    int temp_eval;

    for (int i = 0; i < this->total_nodes; i++)
    {

        if (method == "RANDOM")
        {
            temp_sol = random_solution(this->total_nodes, this->n_nodes);
            temp_eval = random_solution_score(temp_sol);
        }

        if (method == "NEAREST_NEIGHBOR")
        {
            temp_sol = nearest_neighbor_solution(this->n_nodes, i);
            temp_eval = nearest_neighbor_solution_score(temp_sol);
        }

        if (method == "GREEDY_CYCLE")
        {
            temp_sol = greedy_cycle_solution(this->n_nodes, i);
            temp_eval = greedy_cycle_solution_score(temp_sol);
        }
        if (method == "REGRET2_GREEDY_CYCLE")
        {
            float regret_proportion = 1.0;
            temp_sol = regret2_greedy_cycle_solution(this->n_nodes, i, regret_proportion);
            temp_eval = regret2_greedy_cycle_solution_score(temp_sol);
        }

        solutions.push_back(temp_sol);
        evaluations.push_back(temp_eval);
    }
    // Get best solution index
    vector<int>::iterator it = min_element(begin(evaluations), end(evaluations));
    int best_sol_idx = distance(begin(evaluations), it);
    temp_sol = solutions[best_sol_idx];
    // Export best solution
    string filename = "lab2/solutions/" + this->instance_name + "/" + method + ".csv";
    temp_sol->write_to_csv(filename);

    cout << method << endl;
    print_solution_stats(&evaluations);
}

RandomSolution *ProblemSolver::random_solution(int total_nodes, int n_nodes)
{
    // Create new random solution
    RandomSolution *rand_sol = new RandomSolution();

    // Generate new solution
    rand_sol->generate(total_nodes, n_nodes);

    // return temp_eval;
    return rand_sol;
}

int ProblemSolver::random_solution_score(Solution *rand_sol)
{
    // For temporal storage
    int temp_eval;

    // Evaluate solution
    temp_eval = rand_sol->evaluate(&this->dist_mat, &this->costs);

    return temp_eval;
}

NearestNeighbor *ProblemSolver::nearest_neighbor_solution(int n_nodes, int start_node)
{
    // Create new random solution
    NearestNeighbor *nearest_neighbor_sol = new NearestNeighbor();

    // Generate new solution
    nearest_neighbor_sol->generate(this->dist_mat, this->costs, start_node, n_nodes);

    // return temp_eval;
    return nearest_neighbor_sol;
}

int ProblemSolver::nearest_neighbor_solution_score(Solution *nearest_neighbor_sol)
{
    // For temporal storage
    int temp_eval;

    // Evaluate solution
    temp_eval = nearest_neighbor_sol->evaluate(&this->dist_mat, &this->costs);

    return temp_eval;
}

GreedyCycle *ProblemSolver::greedy_cycle_solution(int n_nodes, int start_node)
{
    // Create new random solution
    GreedyCycle *greedy_cycle_sol = new GreedyCycle();

    // Generate new solution
    greedy_cycle_sol->generate(this->dist_mat, this->costs, start_node, n_nodes);

    // return temp_eval;
    return greedy_cycle_sol;
}

int ProblemSolver::greedy_cycle_solution_score(Solution *greedy_cycle_sol)
{
    // For temporal storage
    int temp_eval;

    // Evaluate solution
    temp_eval = greedy_cycle_sol->evaluate(&this->dist_mat, &this->costs);

    return temp_eval;
}

RegretGreedyCycle *ProblemSolver::regret2_greedy_cycle_solution(int n_nodes, int start_node, float regret_proportion)
{
    // Create new random solution
    RegretGreedyCycle *greedy_cycle_sol = new RegretGreedyCycle();

    // Generate new solution
    greedy_cycle_sol->generate(this->dist_mat, this->costs, regret_proportion, start_node, n_nodes);

    // return temp_eval;
    return greedy_cycle_sol;
}

int ProblemSolver::regret2_greedy_cycle_solution_score(Solution *greedy_cycle_sol)
{
    // For temporal storage
    int temp_eval;

    // Evaluate solution
    temp_eval = greedy_cycle_sol->evaluate(&this->dist_mat, &this->costs);

    return temp_eval;
}

void ProblemSolver::print_solution_stats(std::vector<int> *evaluations)
{

    // Find minimum value
    int min, max;
    double average;
    calculate_stats(evaluations, &min, &average, &max);

    cout << "MIN " << min << " AVG " << average << " MAX " << max << endl;
}

int ProblemSolver::get_total_nodes()
{
    return this->total_nodes;
}

int ProblemSolver::get_n_nodes()
{
    return this->n_nodes;
}