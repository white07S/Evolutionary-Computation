#include "CMLocalSearchSolver.h"
#include "LocalSearchSolver.h"
#include "Solution.h"

#include <string>
#include <set>
#include <iostream>
#include <limits>
#include <random>
#include <numeric>
#include <algorithm>
#include <map>

using namespace std;
using namespace N;

CMLocalSearchSolver::CMLocalSearchSolver(string instance_filename,
                                         double fraction_nodes,
                                         Solution initial_solution,
                                         int n_candidates)
    : LocalSearchSolver(instance_filename, fraction_nodes, initial_solution)
{
    this->construct_candidate_nodes(n_candidates);
    this->construct_node_idxs_lookup();
}

void CMLocalSearchSolver::run_candidates(string neigh_method, string search_method)
{
    int current_best_delta = -1;
    int best_inter_delta, best_intra_edges_delta;

    int arg1, arg2, temp_arg1, temp_arg2;
    string move_type;

    while (current_best_delta < 0)
    {
        current_best_delta = 0;
        this->find_best_neighbor_edges_from_candidates(&best_intra_edges_delta, &temp_arg1, &temp_arg2);
        if (best_intra_edges_delta < current_best_delta)
        {
            arg1 = temp_arg1;
            arg2 = temp_arg2;
            move_type = "intra_edges";
            current_best_delta = best_intra_edges_delta;
        }
        this->find_best_neighbor_nodes_from_candidates(&best_inter_delta, &temp_arg1, &temp_arg2);
        if (best_inter_delta < current_best_delta)
        {
            arg1 = temp_arg1;
            arg2 = temp_arg2;
            move_type = "inter_nodes";
            current_best_delta = best_inter_delta;
        }
        if (current_best_delta >= 0)
        {
            // We don't want to alter the solution
            // if the new current delta is not less than 0
            break;
        }
        // cout << "Move type " << move_type << " Delta " << current_best_delta << endl;
        // cout << arg1 << " " << arg2 << endl;
        this->apply_move(&move_type, arg1, arg2);
        this->best_sol_evaluation += current_best_delta;
        // cout << this->best_sol_evaluation << endl;
        //  this->best_solution.print();
    }
    // int eval = best_solution.evaluate(&this->dist_mat, &this->costs);
    // cout << "Actual evaluation: " << eval << endl;
    // this->best_solution.print();
}

void CMLocalSearchSolver::find_best_neighbor_edges_from_candidates(int *out_delta, int *first_edge_idx,
                                                                   int *second_edge_idx)
{
    // We assume edge 0 to connect nodes[0] with nodes[1]
    // Last edge is between last node and the first node
    int min_delta = 0;
    int min_edge1_idx = -1;
    int min_edge2_idx = -1;

    for (auto &edge1_idx : this->iterator1)
    {
        for (auto &cand_node : this->candidate_nodes[edge1_idx])
        {
            if (this->best_solution.contains(cand_node))
            {
                int cand_idx = this->get_solution_index(cand_node);
                if (edge1_idx < cand_idx - 1)
                {
                    int delta_next = this->best_solution.calculate_delta_intra_route_edges(
                        &this->dist_mat, edge1_idx, cand_idx);
                    if (delta_next < min_delta)
                    {
                        min_delta = delta_next;
                        min_edge1_idx = edge1_idx;
                        min_edge2_idx = cand_idx;
                    }
                }
                int edge1_prev_idx = this->best_solution.get_prev_node_idx(edge1_idx);
                int cand_prev_idx = this->best_solution.get_prev_node_idx(cand_idx);
                if (edge1_prev_idx < cand_prev_idx - 1)
                {
                    int delta_prev = this->best_solution.calculate_delta_intra_route_edges(
                        &this->dist_mat, edge1_prev_idx, cand_prev_idx);
                    if (delta_prev < min_delta)
                    {
                        min_delta = delta_prev;
                        min_edge1_idx = edge1_prev_idx;
                        min_edge2_idx = cand_prev_idx;
                    }
                }
            }
        }
    }
    *out_delta = min_delta;
    *first_edge_idx = min_edge1_idx;
    *second_edge_idx = min_edge2_idx;
}

void CMLocalSearchSolver::find_best_neighbor_nodes_from_candidates(int *out_delta, int *first_node_idx,
                                                                   int *second_node)
{
    int min_delta = 0;
    int min_node1_idx = -1;
    int min_node2 = -1;
    int delta;
    // Store idx of the node to be removed
    int removed_idx;

    for (auto &node1_idx : this->iterator1)
    {
        for (auto &cand_node : this->candidate_nodes[node1_idx])
        {
            if (!this->best_solution.contains(cand_node))
            {
                delta = this->best_solution.calculate_delta_inter_route_nodes_candidates(&this->dist_mat, &this->costs,
                                                                                         node1_idx, cand_node,
                                                                                         &removed_idx,
                                                                                         "previous");
                if (delta < min_delta)
                {
                    min_delta = delta;
                    min_node1_idx = removed_idx;
                    min_node2 = cand_node;
                }
                delta = this->best_solution.calculate_delta_inter_route_nodes_candidates(&this->dist_mat, &this->costs,
                                                                                         node1_idx, cand_node,
                                                                                         &removed_idx,
                                                                                         "next");
                if (delta < min_delta)
                {
                    min_delta = delta;
                    min_node1_idx = removed_idx;
                    min_node2 = cand_node;
                }
            }
        }
    }

    *out_delta = min_delta;
    *first_node_idx = min_node1_idx;
    *second_node = min_node2;
}

void CMLocalSearchSolver::apply_move(string *move_type, int arg1, int arg2)
{

    if (*move_type == "inter_nodes")
    {
        this->best_solution.exchange_node_at_idx(arg1, arg2);
        update_node_lookup_inter(arg1, arg2);
    }
    else if (*move_type == "intra_edges")
    {
        this->best_solution.exchange_2_edges(arg1, arg2);
        update_node_lookup_intra_edges(arg1, arg2);
    }
}

void CMLocalSearchSolver::update_node_lookup_inter(int removed_idx,
                                                   int new_node)
{
    int removed_node = this->best_solution.get_nodes()[removed_idx];
    // Remove node from lookup
    node_lookup.erase(removed_node);
    // Assign new node with the same index of the solution
    node_lookup[new_node] = removed_idx;
}

void CMLocalSearchSolver::update_node_lookup_intra_edges(int edge1_idx,
                                                         int edge2_idx)
{
    for (int i = edge1_idx + 1; i < edge2_idx + 1; ++i)
    {
        node_lookup[this->best_solution.get_nodes()[i]] = i;
    }
}

/*
Constructs a node lookup structure for fast verification
at what index the node is in the solution
*/
void CMLocalSearchSolver::construct_node_idxs_lookup()
{
    map<int, int> node_lookup;
    vector<int> nodes = this->best_solution.get_nodes();
    for (int i = 0; i < this->best_solution.get_number_of_nodes(); ++i)
    {
        node_lookup[nodes[i]] = i;
    }
    this->node_lookup = node_lookup;
}

int CMLocalSearchSolver::get_solution_index(int node)
{
    return this->node_lookup[node];
}

void CMLocalSearchSolver::construct_candidate_nodes(int n_candidates)
{
    vector<vector<int>> dist_mat = this->get_distance_matrix();
    vector<int> costs = this->get_costs();
    for (int node_idx = 0; node_idx < this->total_nodes; node_idx++)
    {
        vector<int> single_node_candidates(this->total_nodes);
        iota(single_node_candidates.begin(), single_node_candidates.end(), 0);
        single_node_candidates.erase(single_node_candidates.begin() + node_idx);

        sort(single_node_candidates.begin(), single_node_candidates.end(), [&](int a, int b)
             {
            int a_value = dist_mat[node_idx][a] + costs[a];
            int b_value = dist_mat[node_idx][b] + costs[b];
            return a_value<b_value; });

        single_node_candidates = {single_node_candidates.begin(),
                                  single_node_candidates.begin() + n_candidates};
        this->candidate_nodes.push_back(single_node_candidates);
    }
}