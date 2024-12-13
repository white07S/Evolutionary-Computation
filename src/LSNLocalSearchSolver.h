#include <string>
#include <vector>

#include "Solution.h"
#include "LocalSearchSolver.h"

#ifndef _LSNLOCALSEARCH_H
#define _LSNLOCALSEARCH_H

namespace N
{
    class LSNLocalSearchSolver : public LocalSearchSolver
    {
    private:
        Solution i_solution;
        double f_nodes;
        std::string i_filename;
        std::vector<int> iter_count;

    public:
        LSNLocalSearchSolver(std::string instance_filename,
                             double fraction_nodes,
                             Solution initial_solution);
        void set_best_solution(Solution new_best);
        void run(double time, bool inner_local_search);
        double get_avg_iter();
    };

}
#endif