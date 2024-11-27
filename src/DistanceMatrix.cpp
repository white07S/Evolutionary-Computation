#include "DistanceMatrix.h"
#include "Utils.h"

#include <cmath>
#include <iostream>

namespace LS {

    void DistanceMatrix::readCoordinates(const std::string& filename,
                                         std::vector<int>& xs,
                                         std::vector<int>& ys)
    {
        std::vector<std::vector<std::string>> rows;
        // Read rows as string
        Utils::readCSV(filename, ';', rows);

        // Reserve space if possible
        xs.reserve(rows.size());
        ys.reserve(rows.size());
        costs.reserve(rows.size());

        // Populate xs, ys, costs with integer values
        for (const auto& row : rows)
        {
            if (row.size() >= 3)
            {
                try {
                    xs.emplace_back(std::stoi(row[0]));
                    ys.emplace_back(std::stoi(row[1]));
                    costs.emplace_back(std::stoi(row[2]));
                }
                catch (const std::invalid_argument& e) {
                    std::cerr << "Invalid data in CSV row: ";
                    for (const auto& item : row) std::cerr << item << " ";
                    std::cerr << std::endl;
                }
            }
        }
    }

    void DistanceMatrix::create(const std::string& filename)
    {
        // For storing xs, ys, and costs
        std::vector<int> xs, ys;

        readCoordinates(filename, xs, ys);

        size_t size = xs.size();
        distanceMatrix.reserve(size);
        for (size_t i = 0; i < size; ++i)
        {
            std::vector<int> temp;
            temp.reserve(size);
            for (size_t j = 0; j < size; ++j)
            {
                // Calculate Euclidean Distance
                double ed = Utils::euclideanDistance(xs[i], ys[i], xs[j], ys[j]);
                // Store the rounded value
                temp.emplace_back(static_cast<int>(std::round(ed)));
            }
            distanceMatrix.emplace_back(std::move(temp));
        }
    }

    const std::vector<std::vector<int>>& DistanceMatrix::getDistanceMatrix() const
    {
        return distanceMatrix;
    }

    const std::vector<int>& DistanceMatrix::getCosts() const
    {
        return costs;
    }

    void DistanceMatrix::printDistanceMatrix() const
    {
        // Print the Distance Matrix
        for (size_t i = 0; i < distanceMatrix.size(); ++i)
        {
            for (size_t j = 0; j < distanceMatrix[i].size(); ++j)
            {
                std::cout << distanceMatrix[i][j] << " ";
            }
            std::cout << std::endl;
        }
    }

}
