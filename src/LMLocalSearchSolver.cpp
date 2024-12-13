#include "LMLocalSearchSolver.h"
#include "LocalSearchSolver.h"
#include "Solution.h"
#include "Utils.h"

#include <string>
#include <set>
#include <iostream>
#include <limits>
#include <numeric>
#include <algorithm>
#include <utility>

using namespace std;
using namespace N;

LMLocalSearchSolver::LMLocalSearchSolver(string instance_filename,
                                         double fraction_nodes,
                                         Solution initial_solution)
    : LocalSearchSolver(instance_filename, fraction_nodes, initial_solution)
{
    this->reset();
}

void LMLocalSearchSolver::reset()
{
    // Clear LM
    while (!this->LM.empty())
    {
        this->LM.pop();
    }

    this->temp_stored_moves.clear();

    // Set new random solution
    RandomSolution new_initial_solution = RandomSolution();
    new_initial_solution.generate(200, 100);
    this->best_solution = new_initial_solution;
    this->best_solution.set_nodes(new_initial_solution.get_nodes());
    this->best_solution.set_selected(new_initial_solution.get_selected());
    this->best_sol_evaluation = this->best_solution.evaluate(&this->dist_mat, &this->costs);

    this->init_LM_intra_edges();
    this->init_LM_inter_nodes();
}

void LMLocalSearchSolver::run()
{

    // cout << "LM SIZE: " << this->LM.size() << endl;
    while (!this->LM.empty())
    {
        vector<int> move = this->LM.top();
        this->LM.pop();

        // whether to keep the move
        bool keep = true;
        int edge1_idx, edge2_idx;
        if (is_applicable(&move, &keep, &edge1_idx, &edge2_idx))
        {
            // cout << "Applicable move:" << endl;
            // print_vector(move);
            add_temp_moves();
            // cout << this->temp_stored_moves.size() << endl;
            this->temp_stored_moves.clear();
            this->apply_app_move(&move, edge1_idx, edge2_idx);
        }
        if (keep)
        {
            this->temp_stored_moves.push_back(move);
        }
    }
}
/*
Searches all edges which could replace given edge
and adds improving moves to LM
*/
bool LMLocalSearchSolver::is_applicable(vector<int> *move,
                                        bool *keep, int *arg1, int *arg2)
{

    int node1 = (*move)[1];
    int node2 = (*move)[2];
    int node3 = (*move)[3];
    int node4 = (*move)[4];
    int move_type = (*move)[5];

    int node1_idx = this->best_solution.find_node_idx(node1);
    int node2_idx, node3_idx, node4_idx;
    int node1_idx_n, node1_idx_n_n , node1_idx_p, node1_idx_p_p ;
    int node3_idx_n, node3_idx_p;

    // Check move type
    if (move_type == 0)
    {
        // check node appearence
        if (!this->best_solution.contains(node1) || !this->best_solution.contains(node2) || !this->best_solution.contains(node3) || !this->best_solution.contains(node4))
        {
            *keep = false;
            return false;
        }

        // Check if there is an edge between node1 and node2
        node1_idx_n = this->best_solution.get_next_node_idx(node1_idx);
        node1_idx_p = this->best_solution.get_prev_node_idx(node1_idx);
        if (this->best_solution.get_node_at_idx(node1_idx_n) != node2 &&
            this->best_solution.get_node_at_idx(node1_idx_p) != node2)
        {
            *keep = false;
            return false;
        }
        
        // Check if there is an edge between node3 and node4
        node3_idx = this->best_solution.find_node_idx(node3);
        node3_idx_n = this->best_solution.get_next_node_idx(node3_idx);
        node3_idx_p = this->best_solution.get_prev_node_idx(node3_idx);
        if (this->best_solution.get_node_at_idx(node3_idx_n) != node4 &&
            this->best_solution.get_node_at_idx(node3_idx_p) != node4)
        {
            *keep = false;
            return false;
        }

        // Check if (node1 before node2) and (node3 before node4)
        if (this->best_solution.get_node_at_idx(node3_idx_n) == node4 && 
            this->best_solution.get_node_at_idx(node1_idx_n) == node2)
        {
            *arg1 = node1_idx;
            *arg2 = node3_idx;
            *keep = false;
            return true;
        }

        // Check if (node1 after node2) and (node3 after node4)
        if (this->best_solution.get_node_at_idx(node3_idx_p) == node4 && 
            this->best_solution.get_node_at_idx(node1_idx_p) == node2)
        {
            *arg1 = node1_idx_p;
            *arg2 = node3_idx_p;
            *keep = false;
            return true;
        }

        // //Check if there are edges in reverse order
        // if ((this->best_solution.get_node_at_idx(node3_idx_n) == node4 && this->best_solution.get_node_at_idx(node1_idx_p) == node2) ||
        //     ((this->best_solution.get_node_at_idx(node3_idx_p) == node4 && this->best_solution.get_node_at_idx(node1_idx_n) == node2)))
     
        *keep = true;
        return false;
        
    }
    else
    {

        *keep = false;

        // check node appearence
        if (!this->best_solution.contains(node1) || !this->best_solution.contains(node2) || !this->best_solution.contains(node3) || this->best_solution.contains(node4))
        {
            return false;
        }

        node1_idx_n = this->best_solution.get_next_node_idx(node1_idx);
        node1_idx_n_n = this->best_solution.get_next_node_idx(node1_idx_n);

        // Check if there is an edge between (node1 before node2) and (node2 before node3)
        if (this->best_solution.get_node_at_idx(node1_idx_n) == node2 &&
            this->best_solution.get_node_at_idx(node1_idx_n_n) == node3)
        {
            *arg1 = node1_idx;
            *arg2 = node1_idx_n;
            return true;
        }

        node1_idx_p = this->best_solution.get_prev_node_idx(node1_idx); 
        node1_idx_p_p = this->best_solution.get_prev_node_idx(node1_idx_p);

        // Check if there is an edge between (node1 after node2) and (node2 after node3)
        if (this->best_solution.get_node_at_idx(node1_idx_p) == node2 &&
            this->best_solution.get_node_at_idx(node1_idx_p_p) == node3)
        {
            *arg1 = node1_idx_p_p;
            *arg2 = node1_idx_p;
            return true;
        }
            
        return false;
    }
}
/*
Applies move type, which is specified by the last element of the move vector
    move[0] - delta
    move[5] - move type:
    0 - intra edges:
            move[1] - first node of the first edge
            move[2] - second node of the first edge
            move[3] - first node of the second edge
            move[4] - second node of the second edge
        arg1 - index of the first edge in the solution
        arg2 - index of the second edge in the solution
    1 - inter nodes:
            move[1] - previous node of the exchanged node
            move[2] - exchanged node
            move[3] - next node of the exchanged node
            move[4] - new node
        arg1 - index of the prevoius node of the exchanged node (first from triplet)
        arg2 - index of the exchanged node (third from triplet)
*/
void LMLocalSearchSolver::apply_app_move(vector<int> *move,
                                         int arg1, int arg2)
{

    int delta = (*move)[0];
    this->best_sol_evaluation += delta;
    // Check move type
    if ((*move)[5] == 0)
    {
        // intra edges
        apply_move("intra_edges", &arg1, &arg2);

        add_improving_node_exchanges(arg1);
        add_improving_node_exchanges(this->best_solution.get_next_node_idx(arg1));
        add_improving_node_exchanges(arg2);
        add_improving_node_exchanges(this->best_solution.get_next_node_idx(arg2));

        add_improving_edge_exchanges(arg1);
        add_improving_edge_exchanges(arg2);
    }
    else
    {
        int new_node = (*move)[4];
        apply_move("inter", &arg2, &new_node);
        // Here arg1 is the index of the first node from the triplet
        add_improving_node_exchanges(arg1);
        // Index of the last node from the triplet
        add_improving_node_exchanges(this->best_solution.get_next_node_idx(arg2));
        

        add_improving_edge_exchanges(arg1);
        add_improving_edge_exchanges(arg2);
    }
}

/*
Pushes to LM moves from a vector.
This is mainly intended for pushing again the currently
non applicable moves
*/
void LMLocalSearchSolver::add_temp_moves()
{
    for (auto move : this->temp_stored_moves)
    {
        this->LM.push(move);
    }
}

void LMLocalSearchSolver::init_LM_intra_edges()
{
    for (auto edge_idx : this->iterator1)
    {
        add_improving_edge_exchanges(edge_idx);
    }
}
void LMLocalSearchSolver::add_improving_edge_exchanges(int edge_idx)
{
    for (int edge2_idx = 0; edge2_idx < this->best_solution.get_number_of_nodes(); ++edge2_idx)
    {
        if (abs(edge_idx - edge2_idx) > 1)
        {

            int delta = this->best_solution.calculate_delta_intra_route_edges(
                &this->dist_mat, edge_idx, edge2_idx);

            if (delta < 0)
            {
                vector<int> move;
                move.push_back(delta);
                move.push_back(this->best_solution.get_node_at_idx(edge_idx));
                move.push_back(this->best_solution.get_node_at_idx(this->best_solution.get_next_node_idx(edge_idx)));
                move.push_back(this->best_solution.get_node_at_idx(edge2_idx));
                move.push_back(this->best_solution.get_node_at_idx(this->best_solution.get_next_node_idx(edge2_idx)));
                // Move type of intra edges is 0
                move.push_back(0);
                // cout << edge_idx_copy << " " << temp_edge2_idx << endl;
                // print_vector(move);
                this->LM.push(move);
            }
        }
    }
}

void LMLocalSearchSolver::init_LM_inter_nodes()
{
    for (auto node_idx : this->iterator1)
    {
        add_improving_node_exchanges(node_idx);
    }
}

void LMLocalSearchSolver::add_improving_node_exchanges(int node_idx)
{
    for (auto node : this->iterator_long)
    {
        if (!this->best_solution.contains(node))
        {
            int delta = this->best_solution.calculate_delta_inter_route(
                &this->dist_mat, &this->costs, node_idx, node);

            if (delta < 0)
            {
                vector<int> move;
                move.push_back(delta);
                move.push_back(this->best_solution.get_node_at_idx(
                    this->best_solution.get_prev_node_idx(node_idx)));
                move.push_back(this->best_solution.get_node_at_idx(node_idx));
                move.push_back(this->best_solution.get_node_at_idx(
                    this->best_solution.get_next_node_idx(node_idx)));
                move.push_back(node);
                // Move type of intra edges is 0
                move.push_back(1);
                // cout << node_idx << " " << node << endl;
                // cout << "MOVE: " << endl;
                // print_vector(move);
                this->LM.push(move);
            }
        }
    }
}

void LMLocalSearchSolver::print_LM()
{
    cout << "PRIORITY QUEUE OF DELTAS:" << endl;
    int q_size = this->LM.size();
    while (!this->LM.empty())
    {
        print_vector(this->LM.top());
        this->LM.pop();
    }
    cout << "LM size: " << q_size << endl;
}