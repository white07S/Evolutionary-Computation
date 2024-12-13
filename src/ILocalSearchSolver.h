#include <string>
#include <queue>

#include "Solution.h"
#include "LocalSearchSolver.h"

#ifndef _ILOCALSEARCH_H
#define _ILOCALSEARCH_H

namespace N
{
    class ILocalSearchSolver : public LocalSearchSolver
    {
    private:
        Solution i_solution;
        double f_nodes;
        std::string i_filename;
        std::vector<int> iter_count;

    public:
        ILocalSearchSolver(std::string instance_filename,
                           double fraction_nodes,
                           Solution initial_solution);
        void perform_random_edge_exchanges(Solution *solution, int n);
        void set_best_solution(Solution new_best);
        void run(double time);
        double get_avg_iter();
    };

}

#endif