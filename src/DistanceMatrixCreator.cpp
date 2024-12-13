#include "DistanceMatrixCreator.h"
#include "Utils.h"

#include <cmath> // power and sqrt
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using namespace N;

void DistanceMatrixCreator::read_coordinates(std::string filename,
                                             vector<int> *xs,
                                             vector<int> *ys)
{

    vector<vector<string>> rows;
    // Read rows as string
    read_csv(filename, ';', &rows);

    // Populate xs, ys, costs with integer values
    for (int i = 0; i < rows.size(); ++i)
    {
        if (rows[i].size() > 0)
        {
            (*xs).push_back(stoi(rows[i][0]));
            (*ys).push_back(stoi(rows[i][1]));
            costs.push_back(stoi(rows[i][2]));
        }
    }
}

void DistanceMatrixCreator::create(string filename)
{
    // For storing xs, ys, and costs
    vector<int> xs, ys;

    read_coordinates(filename, &xs, &ys);

    for (int i = 0; i < xs.size(); i++)
    {
        vector<int> temp;
        for (int j = 0; j < xs.size(); j++)
        {
            // Calculate Euclidean Distance
            double ed = euclidean_distance(xs[i], ys[i], xs[j], ys[j]);
            // Store the rounded value
            temp.push_back(round(ed));
        }
        distance_matrix.push_back(temp);
    }
}

vector<vector<int>> DistanceMatrixCreator::get_distance_matrix() { return distance_matrix; }
std::vector<int> DistanceMatrixCreator::get_costs() {return costs; }

void DistanceMatrixCreator::print_distance_matrix()
{
    // Print the Distance Matrix
    for (int i = 0; i < distance_matrix.size(); ++i)
    {
        for (int j = 0; j < distance_matrix[i].size(); ++j)
        {
            cout << distance_matrix[i][j] << " ";
            cout << i << " " << j << endl;
        }
        cout << endl;
    }
}