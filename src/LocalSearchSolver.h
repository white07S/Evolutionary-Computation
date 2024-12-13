#include <string>
#include <random>
#include <set>

#include "Solution.h"
#include "ProblemSolver.h"

#ifndef _LOCALSEARCH_H
#define _LOCALSEARCH_H

namespace N
{
    class LocalSearchSolver : public ProblemSolver
    {
    protected:
        Solution best_solution;
        int best_sol_evaluation;
        std::vector<int> iterator1;
        std::vector<int> iterator2;
        std::vector<int> iterator_long;
        std::random_device rd;

    public:
        LocalSearchSolver(std::string instance_filename, double fraction_nodes, Solution initial_solution);

        void reset();
        void set_initial_solution(Solution *new_initial_solution);
        void set_initial_solution_copy(Solution new_initial_solution);
        void write_best_to_csv(std::string filename);
        int get_best_solution_eval();
        std::vector<int> get_best_solution();
        void greedy_cycle_repair(std::vector<int> *tmp_sol);
        Solution get_best_full_solution();
        Solution *get_best_solution_addr();

        void perturb_best_solution(int n);
        void destroy_and_repair_best_solution();
        void destroy_and_repair_best_solution_v2();

        void run_basic(std::string neigh_method, std::string search_method);

        void find_best_inter_neighbor(int *best_eval, int *exchanged_node, int *new_node, std::string search_method);
        void find_best_intra_neighbor_nodes(int *best_eval, int *first_node_idx, int *second_node_idx, std::string search_method);
        void find_best_intra_neighbor_edges(int *out_delta, int *first_edge_idx, int *second_edge_idx, std::string search_method);

        void apply_move(std::string move_type, int *arg1, int *arg2);
    };
}

#endif