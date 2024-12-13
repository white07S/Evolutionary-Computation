#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <math.h>
#include <chrono>

#include "Utils.h"
#include "ProblemSolver.h"
#include "LocalSearchSolver.h"

using namespace std;

void read_csv(string filename,
              char delimiter,
              std::vector<std::vector<std::string>> *rows)
{
    // Create a file pointer
    ifstream file;

    // Open csv file
    file.open(filename);

    if (!file.is_open())
    {
        throw runtime_error("Could not open the file");
    }

    // For temporary storage
    // For contents of a single row
    vector<string> row;
    string line, word;

    while (file.good())
    {
        row.clear();

        // Read one row and store
        // and store it in line
        getline(file, line);

        // For breaking words
        stringstream s(line);

        while (getline(s, word, ';'))
        {
            // Collect items separated by delimiter
            row.push_back(word);
        }
        (*rows).push_back(row);
    }

    file.close();
}

double euclidean_distance(int x1, int y1, int x2, int y2)
{
    double euc_dist;
    // Calculate Euclidean Distance between (x1, y1) and (x2, y2)
    euc_dist = sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));

    return euc_dist;
}

void measure_generation_time(std::string method,
                             N::ProblemSolver *obj,
                             void (N::ProblemSolver::*func)(std::string))
{

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    (obj->*func)(method);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Generation time = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;
}