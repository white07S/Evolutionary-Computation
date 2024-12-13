#include "Solution.h"
#include "Utils.h"
#include <iostream>
#include <string>
#include <fstream>
#include <set>
#include <algorithm>
#include <iterator> // for inserter

using namespace std;
using namespace N;

void Solution::add_node(int node)
{
    // Add node to solution vector
    this->nodes.push_back(node);
    this->selected.insert(node);
    this->n_nodes += 1;
}
void Solution::remove_node(int idx)
{
    this->selected.erase(this->nodes[idx]);
    this->nodes.erase(this->nodes.begin() + idx);
    this->n_nodes -= 1;
}

void Solution::remove_nodes(int idx, int amount)
{
    for (int i = 0; i < amount; i++)
    {
        this->remove_node(idx);
    }
}

bool Solution::contains(int node)
{
    return this->selected.contains(node);
}

void Solution::set_nodes(vector<int> nodes)
{
    // Set nodes
    this->nodes.assign(nodes.begin(), nodes.end());
    this->n_nodes = nodes.size();
}

void Solution::update_selected()
{
    this->selected.clear();
    this->selected = set(this->nodes.begin(), this->nodes.end());
}

vector<int> Solution::get_nodes()
{
    return this->nodes;
}

int Solution::get_number_of_nodes()
{
    return this->n_nodes;
}
// More robust than get_nodes()
int Solution::calculate_number_of_nodes()
{
    return this->nodes.size();
}

set<int> Solution::get_selected()
{
    return this->selected;
}

void Solution::set_selected(set<int> new_selected)
{
    this->selected = new_selected;
}

int Solution::get_node_at_idx(int node_idx)
{
    return this->nodes[node_idx];
}

int Solution::get_next_node_idx(int node_idx)
{
    return (node_idx + 1) % this->n_nodes;
}

int Solution::get_prev_node_idx(int node_idx)
{
    return (node_idx + this->n_nodes - 1) % this->n_nodes;
}

int Solution::find_node_idx(int node)
{
    auto it = find(this->nodes.begin(), this->nodes.end(), node);
    if (it != this->nodes.end())
    {
        return it - this->nodes.begin();
    }
    return -1;
}

void Solution::exchange_node_at_idx(int node_idx, int new_node)
{
    // Update the selected set
    this->selected.insert(new_node);
    this->selected.erase(this->nodes[node_idx]);

    this->nodes[node_idx] = new_node;
}

void Solution::exchange_2_nodes(int node_idx1, int node_idx2)
{
    int tmp_node = this->nodes[node_idx1];
    this->nodes[node_idx1] = this->nodes[node_idx2];
    this->nodes[node_idx2] = tmp_node;
}

bool Solution::are_consecutive(int node1_idx, int node2_idx)
{
    int node1_next_idx = get_next_node_idx(node1_idx);
    int node1_prev_idx = get_prev_node_idx(node1_idx);
    if (node1_next_idx == node2_idx || node1_prev_idx == node2_idx)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void Solution::exchange_2_edges(int edge1_idx, int edge2_idx)
{
    if (edge2_idx < edge1_idx)
    {
        swap(edge1_idx, edge2_idx);
    }
    reverse(this->nodes.begin() + edge1_idx + 1, this->nodes.begin() + edge2_idx + 1);
}

int Solution::evaluate(vector<vector<int>> *dist_mat, vector<int> *costs)
{

    int temp_node_cost, temp_dist;

    int current = this->nodes[0];
    // Init total cost as the cost
    // of the first node
    int total_cost = (*costs)[current];

    // To track the next node
    int next;

    for (int i = 1; i < this->n_nodes; i++)
    {
        next = this->nodes[i];

        // Add node cost
        temp_node_cost = (*costs)[next];
        total_cost += temp_node_cost;

        // Add distance cost
        temp_dist = (*dist_mat)[current][next];
        total_cost += temp_dist;

        current = next;
    }

    total_cost += (*dist_mat)[current][this->nodes[0]];

    return total_cost;
}

int Solution::calculate_delta_inter_route(vector<vector<int>> *dist_mat, vector<int> *costs,
                                          int exchanged_idx, int new_node)
{
    // Calculate node cost change
    int delta = (*costs)[new_node] - (*costs)[this->nodes[exchanged_idx]];

    // Calculate distance change
    int prev_node_idx = this->get_prev_node_idx(exchanged_idx);
    int next_node_idx = this->get_next_node_idx(exchanged_idx);

    int prev_node = this->nodes[prev_node_idx];
    int next_node = this->nodes[next_node_idx];

    // Subtract distance to the node to be exchanged
    this->subtract_distance_from_delta(&delta, dist_mat, prev_node, this->nodes[exchanged_idx]);
    this->subtract_distance_from_delta(&delta, dist_mat, this->nodes[exchanged_idx], next_node);

    // Add distance to the new node
    this->add_distance_to_delta(&delta, dist_mat, prev_node, new_node);
    this->add_distance_to_delta(&delta, dist_mat, new_node, next_node);

    return delta;
}

int Solution::calculate_delta_intra_route_nodes(vector<std::vector<int>> *dist_mat,
                                                int first_idx,
                                                int second_idx)
{
    int delta = 0;

    int first_node = this->nodes[first_idx];
    int second_node = this->nodes[second_idx];

    if (first_node == second_node)
    {
        return 0;
    }

    // Get previous and next of the first node
    int first_prev_node_idx = this->get_prev_node_idx(first_idx);
    int first_next_node_idx = this->get_next_node_idx(first_idx);

    int first_prev_node = this->nodes[first_prev_node_idx];
    int first_next_node = this->nodes[first_next_node_idx];

    // Get previous and next of the second node
    int second_prev_node_idx = this->get_prev_node_idx(second_idx);
    int second_next_node_idx = this->get_next_node_idx(second_idx);

    int second_prev_node = this->nodes[second_prev_node_idx];
    int second_next_node = this->nodes[second_next_node_idx];

    if (second_next_node == first_node)
    {
        // Switch first node with second node
        // if second node preceeds the first one
        int tmp_node = first_node;
        first_node = second_node;
        second_node = tmp_node;
    }

    // Subtract distances between first-previous and second-next
    this->subtract_distance_from_delta(&delta, dist_mat, first_prev_node, first_node);
    this->subtract_distance_from_delta(&delta, dist_mat, second_node, second_next_node);

    // Add distances between firsy-prev - second and first - second-next
    this->add_distance_to_delta(&delta, dist_mat, first_prev_node, second_node);
    this->add_distance_to_delta(&delta, dist_mat, first_node, second_next_node);

    if (are_consecutive(first_idx, second_idx))
    {
        // This is enough to do for
        // consecutive nodes
        return delta;
    }
    // Subtract distances between first - first-next and second-prev - second
    this->subtract_distance_from_delta(&delta, dist_mat, first_node, first_next_node);
    this->subtract_distance_from_delta(&delta, dist_mat, second_prev_node, second_node);

    // Add distances between second - first-next and first - second-prev
    this->add_distance_to_delta(&delta, dist_mat, second_node, first_next_node);
    this->add_distance_to_delta(&delta, dist_mat, first_node, second_prev_node);

    return delta;
}

int Solution::calculate_delta_inter_route_nodes_candidates(vector<std::vector<int>> *dist_mat,
                                                           vector<int> *costs,
                                                           int first_node_idx,
                                                           int node_to_add,
                                                           int *removed_idx,
                                                           string direction)
{
    int n_first_node_idx;
    int n_n_first_node_idx;

    if (direction == "previous")
    { // get 2 previous indexes
        n_first_node_idx = this->get_prev_node_idx(first_node_idx);
        n_n_first_node_idx = this->get_prev_node_idx(n_first_node_idx);
    }
    else
    { // get 2 next indexes
        n_first_node_idx = this->get_next_node_idx(first_node_idx);
        n_n_first_node_idx = this->get_next_node_idx(n_first_node_idx);
    }
    // Assign index of the node which will be removed
    *removed_idx = n_first_node_idx;

    int first_node = this->nodes[first_node_idx];
    // get 2 next/previous nodes
    int n_first_node = this->nodes[n_first_node_idx];     // closest neighbor
    int n_n_first_node = this->nodes[n_n_first_node_idx]; // second closest neighbor

    int delta = 0;
    delta += (*costs)[node_to_add] - (*costs)[n_first_node];

    this->subtract_distance_from_delta(&delta, dist_mat, first_node, n_first_node);
    this->subtract_distance_from_delta(&delta, dist_mat, n_first_node, n_n_first_node);

    this->add_distance_to_delta(&delta, dist_mat, first_node, node_to_add);
    this->add_distance_to_delta(&delta, dist_mat, node_to_add, n_n_first_node);

    return delta;
}

int Solution::calculate_delta_intra_route_edges(std::vector<std::vector<int>> *dist_mat,
                                                int first_edge_idx, int second_edge_idx)
{
    // Get the nodes that are connected by edges
    int node1edge1 = this->nodes[first_edge_idx];
    int node2edge1 = this->nodes[get_next_node_idx(first_edge_idx)];
    int node1edge2 = this->nodes[second_edge_idx];
    int node2edge2 = this->nodes[get_next_node_idx(second_edge_idx)];

    int delta = 0;
    // Subtract distances of erased edges
    delta -= (*dist_mat)[node1edge1][node2edge1];
    delta -= (*dist_mat)[node1edge2][node2edge2];

    // Add distances of new edges
    delta += (*dist_mat)[node1edge1][node1edge2];
    delta += (*dist_mat)[node2edge1][node2edge2];

    return delta;
}

void Solution::subtract_distance_from_delta(int *delta, vector<vector<int>> *dist_mat,
                                            int first_node, int second_node)
{
    *delta -= (*dist_mat)[first_node][second_node];
}

void Solution::add_distance_to_delta(int *delta, vector<vector<int>> *dist_mat,
                                     int first_node, int second_node)
{

    *delta += (*dist_mat)[first_node][second_node];
}

void Solution::print()
{

    for (int i = 0; i < this->nodes.size(); i++)
    {
        cout << this->nodes[i] << " ";
    }
    cout << endl;
}

void Solution::write_to_csv(string filename)
{

    ofstream myfile;
    myfile.open(filename);

    for (int i = 0; i < this->nodes.size(); i++)
    {
        myfile << this->nodes[i] << endl;
    }
    myfile.close();
}

int Solution::most_beneficial_node(vector<int> all_distances, vector<int> all_costs, vector<int> excluded_nodes)
{

    int min_idx = -1;
    int min_total_cost = numeric_limits<int>::max();

    for (int i = 0; i < all_distances.size(); i++)
    {
        if (!contain(excluded_nodes, i))
        {

            int tmp_total_cost = all_distances[i] + all_costs[i];
            if (tmp_total_cost < min_total_cost)
            {
                min_idx = i;
                min_total_cost = tmp_total_cost;
            }
        }
    }
    return min_idx;
}