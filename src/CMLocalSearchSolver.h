#include <string>
#include <map>

#include "Solution.h"
#include "LocalSearchSolver.h"

#ifndef _CMLOCALSEARCH_H
#define _CMLOCALSEARCH_H

namespace N
{
    class CMLocalSearchSolver : public LocalSearchSolver
    {
    private:
        std::vector<std::vector<int>> candidate_nodes;
        // For fast access of node indices
        std::map<int, int> node_lookup;

    public:
        CMLocalSearchSolver(std::string instance_filename,
                            double fraction_nodes,
                            Solution initial_solution,
                            int n_candidates);
        void run_candidates(std::string neigh_method, std::string search_method);

        void construct_candidate_nodes(int n_candidates);
        void construct_node_idxs_lookup();

        void find_best_neighbor_edges_from_candidates(int *out_delta, int *first_edge_idx,
                                                      int *second_edge_idx);
        void find_best_neighbor_nodes_from_candidates(int *out_delta, int *first_node_idx,
                                                      int *second_node);

        void apply_move(std::string *move_type, int arg1, int arg2);
        int get_solution_index(int node);

        void update_node_lookup_inter(int removed_idx, int new_node);
        void update_node_lookup_intra_edges(int edge1_idx, int edge2_idx);
    };

}
#endif