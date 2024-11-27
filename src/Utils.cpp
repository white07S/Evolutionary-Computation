#include "Utils.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <stdexcept>

namespace LS {

    void Utils::readCSV(const std::string& filename,
                        char delimiter,
                        std::vector<std::vector<std::string>>& rows)
    {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open the file: " + filename);
        }

        std::string line, word;
        std::vector<std::string> row;

        while (std::getline(file, line))
        {
            row.clear();
            std::stringstream s(line);
            while (std::getline(s, word, delimiter))
            {
                row.emplace_back(word);
            }
            rows.emplace_back(row);
        }

        file.close();
    }

    double Utils::euclideanDistance(int x1, int y1, int x2, int y2)
    {
        return std::sqrt(std::pow(x1 - x2, 2) + std::pow(y1 - y2, 2));
    }

    void Utils::writeMatrixToCSV(const std::vector<std::vector<int>>& matrix, const std::string& filename)
    {
        std::ofstream myfile(filename);
        if (!myfile.is_open()) {
            throw std::runtime_error("Could not open file for writing: " + filename);
        }

        for (const auto& row : matrix)
        {
            for (size_t i = 0; i < row.size(); ++i)
            {
                myfile << row[i];
                if (i < row.size() - 1)
                {
                    myfile << ",";
                }
            }
            myfile << "\n";
        }

        myfile.close();
    }

}
