#include <vector>
#include <string>

#include "Solution.h"
#include "RandomSolution.h"
#include "NearestNeighbor.h"
#include "GreedyCycle.h"
#include "RegretGreedyCycle.h"

#ifndef _PROBLEMSOLVER_H
#define _PROBLEMSOLVER_H

namespace N
{
    class ProblemSolver
    {
    protected:
        std::vector<std::vector<int>> dist_mat;
        std::vector<int> costs;
        int total_nodes;
        int n_nodes;
        std::string instance_name;

    public:
        int get_total_nodes();
        int get_n_nodes();

        ProblemSolver(std::string instance_filename, double fraction_nodes);
        void generate_solutions(std::string method);

        RandomSolution *random_solution(int total_nodes, int n_nodes);
        int random_solution_score(Solution *rand_sol);

        NearestNeighbor *nearest_neighbor_solution(int n_nodes, int start_node);
        int nearest_neighbor_solution_score(Solution *nearest_neighbor_sol);

        GreedyCycle *greedy_cycle_solution(int n_nodes, int start_node);
        int greedy_cycle_solution_score(Solution *greedy_cycle_sol);

        RegretGreedyCycle *regret2_greedy_cycle_solution(int n_nodes, int start_node, float regret_proportion);
        int regret2_greedy_cycle_solution_score(Solution *greedy_cycle_sol);

        void print_solution_stats(std::vector<int> *evaluations);

        std::vector<std::vector<int>> get_distance_matrix();
        std::vector<int> get_costs();
    };

}

#endif