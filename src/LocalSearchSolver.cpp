#include "LocalSearchSolver.h"
#include "Solution.h"
#include "Utils.h"

#include <string>
#include <cstdlib>
#include <set>
#include <iostream>
#include <limits>
#include <random>
#include <numeric>
#include <algorithm>

using namespace std;
using namespace N;

LocalSearchSolver::LocalSearchSolver(string instance_filename, double fraction_nodes, Solution initial_solution)
    : ProblemSolver(instance_filename, fraction_nodes)
{
    this->best_solution = initial_solution;
    this->best_solution.set_nodes(initial_solution.get_nodes());
    this->best_solution.set_selected(initial_solution.get_selected());
    this->best_sol_evaluation = this->best_solution.evaluate(&this->dist_mat, &this->costs);

    vector<int> iterator1(this->best_solution.get_number_of_nodes());
    iota(iterator1.begin(), iterator1.end(), 0);
    this->iterator1 = iterator1;

    vector<int> iterator2(this->best_solution.get_number_of_nodes());
    iota(iterator2.begin(), iterator2.end(), 0);
    this->iterator2 = iterator2;

    vector<int> iterator_long(this->total_nodes);
    iota(iterator_long.begin(), iterator_long.end(), 0);
    this->iterator_long = iterator_long;
}

void LocalSearchSolver::set_initial_solution(Solution *new_initial_solution)
{

    this->best_solution = *new_initial_solution;
    this->best_sol_evaluation = this->best_solution.evaluate(&this->dist_mat, &this->costs);
}

void LocalSearchSolver::set_initial_solution_copy(Solution new_initial_solution)
{
    this->best_solution = new_initial_solution;
    this->best_solution.set_nodes(new_initial_solution.get_nodes());
    this->best_solution.set_selected(new_initial_solution.get_selected());
    this->best_sol_evaluation = this->best_solution.evaluate(&this->dist_mat, &this->costs);
}

void LocalSearchSolver::run_basic(string neigh_method, string search_method)
{
    int current_best_delta = -1;
    int best_inter_delta, best_intra_nodes_delta, best_intra_edges_delta;

    int arg1, arg2, temp_arg1, temp_arg2;
    string move_type;

    typedef void (LocalSearchSolver::*VoidFunctionFourParams)(int *, int *, int *, string);
    vector<VoidFunctionFourParams> neigh_methods;
    neigh_methods = {&LocalSearchSolver::find_best_inter_neighbor,
                     &LocalSearchSolver::find_best_intra_neighbor_nodes,
                     &LocalSearchSolver::find_best_intra_neighbor_edges};

    vector<string> move_types = {"inter", "intra_nodes", "intra_edges"};

    vector<int> neigh_methods_idxs = {0};

    int method_idx = 0;
    if (neigh_method == "TWO_NODES")
    {
        method_idx = 1;
    }
    if (neigh_method == "TWO_EDGES")
    {
        method_idx = 2;
    }

    if (rand() % 2 == 1)
    {
        neigh_methods_idxs.push_back(method_idx);
    }
    else
    {
        neigh_methods_idxs.insert(neigh_methods_idxs.begin(), method_idx);
    }

    while (current_best_delta < 0)
    {

        current_best_delta = 0;

        for (auto &i : neigh_methods_idxs)
        {
            (this->*neigh_methods[i])(&best_inter_delta, &temp_arg1, &temp_arg2, search_method);
            if (best_inter_delta < current_best_delta)
            {
                arg1 = temp_arg1;
                arg2 = temp_arg2;
                move_type = move_types[i];
                current_best_delta = best_inter_delta;
                if (search_method == "GREEDY")
                {
                    break;
                }
            }
        }

        if (current_best_delta >= 0)
        {
            // We don't want to alter the solution
            // if the new current delta is not less than 0
            break;
        }
        this->best_sol_evaluation += current_best_delta;
        apply_move(move_type, &arg1, &arg2);
        // cout << "Move type " << move_type << " Delta " << current_best_delta << endl;
        // cout << arg1 << " " << arg2 << endl;
        // cout << this->best_sol_evaluation << endl;
    }
    // int eval = best_solution.evaluate(&this->dist_mat, &this->costs);
    // cout << "Actual evaluation: " << eval << endl;
    // this->best_solution.print();
}

void LocalSearchSolver::find_best_intra_neighbor_nodes(int *out_delta, int *first_node_idx, int *second_node_idx, string search_method)
{
    int nodes_number = this->best_solution.get_number_of_nodes();
    int min_delta = 0;
    int min_node1_idx = -1;
    int min_node2_idx = -1;
    int delta;

    if (search_method == "GREEDY")
    {
        shuffle(this->iterator1.begin(), this->iterator1.end(), this->rd);
        shuffle(this->iterator2.begin(), this->iterator2.end(), this->rd);
    }

    for (auto &node1_idx : this->iterator1)
    {
        for (auto &node2_idx : this->iterator2)
        {
            if (node1_idx < node2_idx)
            {
                delta = this->best_solution.calculate_delta_intra_route_nodes(&this->dist_mat,
                                                                              node1_idx, node2_idx);
                if (delta < min_delta)
                {
                    min_delta = delta;
                    min_node1_idx = node1_idx;
                    min_node2_idx = node2_idx;

                    if (search_method == "GREEDY")
                    {
                        *out_delta = min_delta;
                        *first_node_idx = min_node1_idx;
                        *second_node_idx = min_node2_idx;
                        return;
                    }
                }
            }
        }
    }
    *out_delta = min_delta;
    *first_node_idx = min_node1_idx;
    *second_node_idx = min_node2_idx;
}

void LocalSearchSolver::find_best_intra_neighbor_edges(int *out_delta, int *first_edge_idx, int *second_edge_idx, string search_method)
{
    // We assume edge 0 to connect nodes[0] with nodes[1]
    // Last edge is between last node and the first node
    int min_delta = 0;
    int min_edge1_idx = -1;
    int min_edge2_idx = -1;
    int delta;

    if (search_method == "GREEDY")
    {
        shuffle(this->iterator1.begin(), this->iterator1.end(), this->rd);
        shuffle(this->iterator2.begin(), this->iterator2.end(), this->rd);
    }

    for (auto &edge1_idx : this->iterator1)
    {
        for (auto &edge2_idx : this->iterator2)
        {
            if (abs(edge1_idx - edge2_idx) > 1)
            {
                delta = this->best_solution.calculate_delta_intra_route_edges(&this->dist_mat,
                                                                              edge1_idx, edge2_idx);
                if (delta < min_delta)
                {
                    min_delta = delta;
                    min_edge1_idx = edge1_idx;
                    min_edge2_idx = edge2_idx;

                    if (search_method == "GREEDY")
                    {
                        *out_delta = min_delta;
                        *first_edge_idx = min_edge1_idx;
                        *second_edge_idx = min_edge2_idx;
                        return;
                    }
                }
            }
        }
    }
    *out_delta = min_delta;
    *first_edge_idx = min_edge1_idx;
    *second_edge_idx = min_edge2_idx;
}

void LocalSearchSolver::find_best_inter_neighbor(int *out_delta, int *exchanged_node, int *new_node, string search_method)
{
    // Finds best neighbor by exchanging some selected node
    // with a not selected node
    int delta;
    int min_delta = 0;
    int min_exchanged_idx = -1;
    int min_new_node = -1;

    // random order on indexes for greedy?
    if (search_method == "GREEDY")
    {
        shuffle(this->iterator1.begin(), this->iterator1.end(), this->rd);
        shuffle(this->iterator_long.begin(), this->iterator_long.end(), this->rd);
    }
    for (auto &j : iterator_long)
    {
        if (!this->best_solution.contains(j))
        {
            for (auto &i : iterator1)
            {
                delta = this->best_solution.calculate_delta_inter_route(&this->dist_mat,
                                                                        &this->costs, i, j);

                if (delta < min_delta)
                {
                    // The neighbor solution is better
                    min_delta = delta;
                    min_exchanged_idx = i;
                    min_new_node = j;

                    if (search_method == "GREEDY")
                    {
                        *out_delta = min_delta;
                        *exchanged_node = min_exchanged_idx;
                        *new_node = min_new_node;
                        return;
                    }
                }
            }
        }
    }
    *out_delta = min_delta;
    *exchanged_node = min_exchanged_idx;
    *new_node = min_new_node;
}

void LocalSearchSolver::apply_move(string move_type, int *arg1, int *arg2)
{

    if (move_type == "inter")
    {
        this->best_solution.exchange_node_at_idx(*arg1, *arg2);
    }
    else if (move_type == "intra_nodes")
    {
        this->best_solution.exchange_2_nodes(*arg1, *arg2);
    }
    else if (move_type == "intra_edges")
    {
        this->best_solution.exchange_2_edges(*arg1, *arg2);
    }
}
void LocalSearchSolver::perturb_best_solution(int n)
{
    for (int i = 0; i < n; ++i)
    {
        int edge1 = 0;
        int edge2 = 0;
        while (abs(edge1 - edge2) < 2)
        {
            edge1 = rand() % 100;
            edge2 = rand() % 100;

            if (edge1 > edge2)
            {
                swap(edge1, edge2);
            }
        }
        int delta = this->best_solution.calculate_delta_intra_route_edges(&this->dist_mat,
                                                                          edge1, edge2);
        this->best_sol_evaluation += delta;
        this->best_solution.exchange_2_edges(edge1, edge2);
    }
}

void LocalSearchSolver::destroy_and_repair_best_solution()
{
    // destroy

    int destroy_sequences_amount = (rand() % 4) + 2;
    int length = this->best_solution.get_number_of_nodes() / (4 * destroy_sequences_amount);

    for (int idx = 0; idx < destroy_sequences_amount; idx++)
    {
        int index_f = rand() % (this->best_solution.get_number_of_nodes() - length + 1);
        this->best_solution.remove_nodes(index_f, length);
        // cout<<index_f<<endl;
        // this->best_solution.print();
    }
    // repair
    vector<int> tmp_sol;
    this->greedy_cycle_repair(&tmp_sol);

    this->best_solution.set_nodes(tmp_sol);
    this->best_solution.update_selected();
    this->best_sol_evaluation = this->best_solution.evaluate(&this->dist_mat, &this->costs);
}

void LocalSearchSolver::destroy_and_repair_best_solution_v2()
{
    // destroy
    int destroy_sequences_amount = (rand() % 3) + 2;
    int length = this->best_solution.get_number_of_nodes() / (4 * destroy_sequences_amount);

    for (int idx = 0; idx < destroy_sequences_amount; idx++)
    {
        int index_f = rand() % (this->best_solution.get_number_of_nodes() - length + 1);
        this->best_solution.remove_nodes(index_f, length);
        // cout<<index_f<<endl;
        // this->best_solution.print();
    }
    // repair
    vector<int> tmp_sol;
    this->greedy_cycle_repair(&tmp_sol);

    this->best_solution.set_nodes(tmp_sol);
    this->best_solution.update_selected();
    this->best_sol_evaluation = this->best_solution.evaluate(&this->dist_mat, &this->costs);
}

void LocalSearchSolver::greedy_cycle_repair(vector<int> *correct_order_nodes)
{

    vector<vector<int>> edges;
    vector<int> nodes = this->best_solution.get_nodes();
    for (int idx = 0; idx < this->best_solution.get_number_of_nodes() - 1; idx++)
    {
        edges.push_back(vector<int>{nodes[idx], nodes[idx + 1]});
    }
    edges.push_back(vector<int>{nodes[this->best_solution.get_number_of_nodes() - 1], nodes[0]});

    while (this->best_solution.get_number_of_nodes() < n_nodes)
    {
        int node_to_add_idx = -1;
        int edge_to_remove_idx = -1;
        int min_total_cost = numeric_limits<int>::max();

        for (int node_idx = 0; node_idx < this->dist_mat.size(); node_idx++)
        {

            if (!this->best_solution.contains(node_idx))
            {

                for (int edge_idx = 0; edge_idx < edges.size(); edge_idx++)
                {

                    int node1 = edges[edge_idx][0];
                    int node2 = edges[edge_idx][1];

                    int total_cost = this->dist_mat[node1][node_idx] +
                                     this->dist_mat[node2][node_idx] -
                                     this->dist_mat[node1][node2] + this->costs[node_idx];

                    if (total_cost < min_total_cost)
                    {

                        min_total_cost = total_cost;
                        edge_to_remove_idx = edge_idx;
                        node_to_add_idx = node_idx;
                    }
                }
            }
        }

        int node_to_connect_1 = edges[edge_to_remove_idx][0];
        int node_to_connect_2 = edges[edge_to_remove_idx][1];

        edges.erase(edges.begin() + edge_to_remove_idx);
        edges.push_back(vector<int>{node_to_connect_1, node_to_add_idx});
        edges.push_back(vector<int>{node_to_connect_2, node_to_add_idx});

        this->best_solution.add_node(node_to_add_idx);
    }

    (*correct_order_nodes).push_back(edges[0][0]);
    while (edges.size() > 1)
    {
        for (int i = 0; i < edges.size(); i++)
        {
            if (edges[i][0] == (*correct_order_nodes).back())
            {
                (*correct_order_nodes).push_back(edges[i][1]);
                edges.erase(edges.begin() + i);
                // continue;
            }
            else if (edges[i][1] == (*correct_order_nodes).back())
            {
                (*correct_order_nodes).push_back(edges[i][0]);
                edges.erase(edges.begin() + i);
                // continue;
            }
        }
    }
}

void LocalSearchSolver::write_best_to_csv(string filename)
{
    this->best_solution.write_to_csv(filename);
}

int LocalSearchSolver::get_best_solution_eval()
{
    return this->best_sol_evaluation;
}
vector<int> LocalSearchSolver::get_best_solution()
{
    return this->best_solution.get_nodes();
}
Solution LocalSearchSolver::get_best_full_solution()
{
    return this->best_solution;
}
Solution *LocalSearchSolver::get_best_solution_addr()
{
    return &this->best_solution;
}

void LocalSearchSolver::reset()
{
    // Set new random solution
    RandomSolution new_initial_solution = RandomSolution();
    new_initial_solution.generate(200, 100);
    this->best_solution= new_initial_solution;
    this->best_solution.set_nodes(new_initial_solution.get_nodes());
    this->best_solution.set_selected(new_initial_solution.get_selected());

    this->best_sol_evaluation = this->best_solution.evaluate(&this->dist_mat, &this->costs);
}
