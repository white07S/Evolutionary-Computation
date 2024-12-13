#include "MSLocalSearchSolver.h"
#include "ILocalSearchSolver.h"
#include "RandomSolution.h"
#include "Utils.h"
#include "GreedyCycle.h"

#include <vector>
#include <iostream>
#include <chrono>
#include <random>
#include <ctime>
#include <string>
#include <fstream>
#include <filesystem>

using namespace std;
using namespace N;

vector<string> instances = {"data/TSPA.csv",
                            "data/TSPB.csv"};

bool ensure_directories(const std::string &dir) {
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    if (ec) {
        cerr << "Failed to create directory: " << dir << ". Error: " << ec.message() << endl;
        return false;
    }
    return true;
}

void write_solution_to_txt(const string& filepath, const vector<int>& solution) {
    ofstream out(filepath);
    if (!out.is_open()) {
        cerr << "Error: Unable to open file " << filepath << " for writing!" << endl;
        return;
    }
    for (auto val : solution) {
        out << val << "\n";
    }
    out.close();
}

void run_experiment()
{
    srand((unsigned)time(NULL));
    for (auto instance : instances)
    {
        cout << instance.substr(5, 4) << endl;
        int rep = 1;
        RandomSolution initial_solution;
        initial_solution.generate(200, 100);

        MSLocalSearchSolver mslss(instance, 0.5, initial_solution);
        vector<int> best_evaluations_ms;
        vector<double> generation_times_ms;
        for (int i = 0; i < rep; i++)
        {
            mslss.reset();
            int generation_time = measure_generation_time(&mslss, &MSLocalSearchSolver::run);
            generation_times_ms.push_back(generation_time);
            int eval = mslss.get_best_solution_eval();
            best_evaluations_ms.push_back(eval);
        }

        string problemName = instance.substr(5, 4); // e.g. TSPA
        string dir = "lab6/solutions/" + problemName; 
        // Ensure directories exist
        if (!ensure_directories(dir)) {
            // If we cannot create the directory, continue to next instance
            continue;
        }

        // Append a slash if needed
        if (dir.back() != '/' && dir.back() != '\\') {
            dir.push_back('/');
        }

        string filename = "MSLS.txt";
        auto best_solution_ms = mslss.get_best_solution();
        write_solution_to_txt(dir + filename, best_solution_ms);

        int min_e_ms, max_e_ms;
        double min_t_ms, avg_t_ms, max_t_ms, avg_e_ms;
        calculate_stats(&best_evaluations_ms, &min_e_ms, &avg_e_ms, &max_e_ms);
        cout << "EVAL MSLS " << avg_e_ms << " (" << min_e_ms << "-" << max_e_ms << ")" << endl;
        calculate_stats(&generation_times_ms, &min_t_ms, &avg_t_ms, &max_t_ms);
        cout << "TIME MSLS " << avg_t_ms / 1000 << " (" << min_t_ms / 1000 << "-" << max_t_ms / 1000 << ")" << endl;

        ILocalSearchSolver ilss(instance, 0.5, initial_solution);
        vector<int> best_evaluations_i;
        vector<double> generation_times_i;
        for (int i = 0; i < rep; i++)
        {
            int generation_time = measure_generation_time(avg_t_ms, &ilss, &ILocalSearchSolver::run);
            generation_times_i.push_back(generation_time);
            int eval = ilss.get_best_solution_eval();
            best_evaluations_i.push_back(eval);
        }
        filename = "ILS.txt";
        auto best_solution_i = ilss.get_best_solution();
        write_solution_to_txt(dir + filename, best_solution_i);

        int min_e_i, max_e_i;
        double min_t_i, avg_t_i, max_t_i, avg_e_i;
        calculate_stats(&best_evaluations_i, &min_e_i, &avg_e_i, &max_e_i);
        cout << "EVAL ILS " << avg_e_i << " (" << min_e_i << "-" << max_e_i << ")" << endl;
        calculate_stats(&generation_times_i, &min_t_i, &avg_t_i, &max_t_i);
        cout << "TIME ILS " << avg_t_i / 1000 << " (" << min_t_i / 1000 << "-" << max_t_i / 1000 << ")" << endl;
        double iterations = ilss.get_avg_iter();
        cout << "Average iter " << iterations << endl;
    }
}

int main()
{
    run_experiment();
    return 0;
}
