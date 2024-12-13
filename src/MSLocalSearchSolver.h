#include <string>
#include <queue>

#include "Solution.h"
#include "LocalSearchSolver.h"

#ifndef _MSLOCALSEARCH_H
#define _MSLOCALSEARCH_H

namespace N
{
    class MSLocalSearchSolver : public LocalSearchSolver
    {
    private:
        Solution i_solution;
        double f_nodes;
        std::string i_filename;

    public:
        MSLocalSearchSolver(std::string instance_filename,
                            double fraction_nodes,
                            Solution initial_solution);
        void set_best_solution(Solution new_best);
        void reset();
        void run();
    };

}

#endif