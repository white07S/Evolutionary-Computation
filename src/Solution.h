#include <vector>
#include <string>
#include <set>

#ifndef _SOLUTION_H
#define _SOLUTION_H

namespace N
{

    class Solution
    {

    protected:
        std::vector<int> nodes;
        // Count how many nodes are in the solution
        int n_nodes;
        // Sets for faster tracking
        std::set<int> selected;

    public:
        void add_node(int node);
        void remove_node(int idx);
        void remove_nodes(int idx, int amount);
        bool contains(int node);

        std::vector<int> get_nodes();
        void set_nodes(std::vector<int> nodes);

        int get_number_of_nodes();
        int calculate_number_of_nodes();

        std::set<int> get_selected();
        void set_selected(std::set<int> new_selected);
        void update_selected();

        int get_node_at_idx(int node_idx);
        int get_next_node_idx(int node_idx);
        int get_prev_node_idx(int node_idx);
        int find_node_idx(int node);
        void exchange_node_at_idx(int node_idx, int new_node);
        void exchange_2_nodes(int node1_idx, int node2_idx);
        bool are_consecutive(int node1_idx, int node2_idx);
        void exchange_2_edges(int edge1_idx, int edge2_idx);

        int evaluate(std::vector<std::vector<int>> *dist_mat,
                     std::vector<int> *costs);

        int most_beneficial_node(std::vector<int> all_distances,
                                 std::vector<int> all_costs, std::vector<int> excluded_distances);

        int calculate_delta_inter_route(std::vector<std::vector<int>> *dist_mat,
                                        std::vector<int> *costs,
                                        int exchange_idx, int new_node);
        int calculate_delta_intra_route_nodes(std::vector<std::vector<int>> *dist_mat,
                                              int first_idx, int second_idx);
        int calculate_delta_inter_route_nodes_candidates(std::vector<std::vector<int>> *dist_mat,
                                                         std::vector<int> *costs,
                                                         int first_idx,
                                                         int cand_node,
                                                         int *removed_idx,
                                                         std::string direction);
        int calculate_delta_intra_route_edges(std::vector<std::vector<int>> *dist_mat,
                                              int first_edge_idx, int second_edge_idx);
        void subtract_distance_from_delta(int *delta, std::vector<std::vector<int>> *dist_mat,
                                          int first_node, int second_node);
        void add_distance_to_delta(int *delta, std::vector<std::vector<int>> *dist_mat,
                                   int first_node, int second_node);

        void print();
        void write_to_csv(std::string filename);
    };

}

#endif