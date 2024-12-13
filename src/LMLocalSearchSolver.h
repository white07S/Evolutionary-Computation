#include <string>
#include <queue>

#include "Solution.h"
#include "LocalSearchSolver.h"

#ifndef _LMLOCALSEARCH_H
#define _LMLOCALSEARCH_H

namespace N
{
    class LMLocalSearchSolver : public LocalSearchSolver
    {
    private:
        std::priority_queue<std::vector<int>, std::vector<std::vector<int>>, std::greater<>> LM;
        std::vector<std::vector<int>> temp_stored_moves;

    public:
        LMLocalSearchSolver(std::string instance_filename,
                            double fraction_nodes,
                            Solution initial_solution);
        void run();
        bool is_applicable(std::vector<int> *move,
                           bool *keep, int *edge1_idx, int *edge2_idx);
        void apply_app_move(std::vector<int> *move,
                            int arg1, int arg2);

        void init_LM_intra_edges();
        void add_improving_edge_exchanges(int edge_idx);

        void init_LM_inter_nodes();
        void add_improving_node_exchanges(int node_idx);

        void add_temp_moves();

        void reset();
        void print_LM();
    };

}

#endif