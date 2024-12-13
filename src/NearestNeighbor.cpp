#include "NearestNeighbor.h"
#include "Utils.h"

#include <iostream>
#include <algorithm>
#include <random>
#include <set>
#include <limits>

using namespace std;
using namespace N;

void NearestNeighbor::generate(vector<vector<int>> dist_mat, vector<int> costs, int start_node, int n_nodes)
{
    int tmp_node = start_node;
    int node_count = 1;

    this->add_node(tmp_node);

    while (this->get_number_of_nodes() < n_nodes)
    {
        tmp_node = most_beneficial_node(dist_mat[tmp_node], costs, this->get_nodes());
        this->add_node(tmp_node);
    }
}