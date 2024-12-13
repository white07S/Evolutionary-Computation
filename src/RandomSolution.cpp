#include "RandomSolution.h"

#include <iostream>
#include <random>
#include <set>

using namespace std;
using namespace N;

void RandomSolution::generate(int total_nodes, int n_nodes)
{
    // Tempoary storage to check
    // if a node was already in the solution
    set<int> nodes;
    int node_id;

    while (nodes.size() < n_nodes)
    {
        node_id = rand() % total_nodes;

        if (!nodes.count(node_id))
        {
            nodes.insert(node_id);
            this->add_node(node_id);
        }
    }
}