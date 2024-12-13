#include <vector>
#include <string>
#include <numeric>   // iota
#include <algorithm> // sort
#include <chrono>
#include <iostream>
#include <fstream>

#include "ProblemSolver.h"
#include "LocalSearchSolver.h"

#ifndef _UTILS_H
#define _UTILS_H

void read_csv(std::string filename,
              char delimiter,
              std::vector<std::vector<std::string>> *rows);

double euclidean_distance(int x1, int y1, int x2, int y2);

template <typename T>
T contain(std::vector<T> vec, T value)
{
    for (int i = 0; i < vec.size(); i++)
    {
        if (vec[i] == value)
        {
            return true;
        }
    }
    return false;
}

template <typename T>
double mean(std::vector<T> *vec)
{

    if ((*vec).empty())
    {
        return 0;
    }
    else
    {
        return std::reduce((*vec).begin(), (*vec).end(), 0.0) / (*vec).size();
    }
}

template <typename T>
void calculate_stats(std::vector<T> *vec, T *min, double *avg, T *max)
{
    // Find minimum value
    *min = *min_element((*vec).begin(), (*vec).end());

    // Find maximum value
    *max = *max_element((*vec).begin(), (*vec).end());

    *avg = mean(vec);
}

// Returns the indices that would sort an array.
// Descending order
template <typename T>
std::vector<size_t> argsort(const std::vector<T> &v)
{
    // get inital indexes
    std::vector<size_t> idx(v.size());
    iota(idx.begin(), idx.end(), 0);

    // Sort indexes based on values of vector
    std::stable_sort(idx.begin(), idx.end(),
                     [&v](size_t i1, size_t i2)
                     { return v[i1] > v[i2]; });

    return idx;
}

void measure_generation_time(std::string method,
                             N::ProblemSolver *obj,
                             void (N::ProblemSolver::*func)(std::string));

/**
 * Measures execution time of a local search run in microseconds.
 */
template <typename T>
double measure_generation_time(std::string neigh_method,
                               std::string search_method,
                               T *obj,
                               void (T::*func)(std::string,
                                               std::string))
{
    auto begin = std::chrono::steady_clock::now();
    (obj->*func)(neigh_method, search_method);
    auto end = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
}

/**
 * Measures execution time of a local search run in microseconds.
 */
template <typename T>
double measure_generation_time(double time,
                               T *obj,
                               void (T::*func)(double))
{
    auto begin = std::chrono::steady_clock::now();
    (obj->*func)(time);
    auto end = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
}

/**
 * Measures execution time of a large-scale neighborhood local search run in microseconds.
 */
template <typename T>
double measure_generation_time(double time,
                               bool inner_local_search,
                               T *obj,
                               void (T::*func)(double, bool))
{
    auto begin = std::chrono::steady_clock::now();
    (obj->*func)(time, inner_local_search);
    auto end = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
}

/**
 * Measures execution time of a local search run in microseconds.
 */
template <typename T>
double measure_generation_time(T *obj,
                               void (T::*func)())
{
    auto begin = std::chrono::steady_clock::now();
    (obj->*func)();
    auto end = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
}

template <typename T>
void print_vector(const std::vector<T> &vec)
{
    for (T i : vec)
    {
        std::cout << i << ' ';
    }
    std::cout << std::endl;
}

template <typename T>
void write_matrix_to_csv(std::vector<std::vector<T>> matrix, std::string filename)
{
    std::ofstream myfile;
    myfile.open(filename);

    for (int i = 0; i < matrix.size(); ++i)
    {
        for (int j = 0; j < matrix[i].size(); ++j)
        {
            myfile << matrix[i][j];
            if (j < matrix[i].size() - 1)
            {
                myfile << ",";
            }
        }
        myfile << std::endl;
    }
    myfile.close();
}

#endif