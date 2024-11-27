#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <string>
#include <algorithm>
#include <numeric>

namespace LS {

    class Utils {
    public:
        static void readCSV(const std::string& filename,
                            char delimiter,
                            std::vector<std::vector<std::string>>& rows);
        
        static double euclideanDistance(int x1, int y1, int x2, int y2);

        template <typename T>
        static bool contains(const std::vector<T>& vec, const T& value) {
            return std::find(vec.begin(), vec.end(), value) != vec.end();
        }

        template <typename T>
        static double mean(const std::vector<T>& vec) {
            if (vec.empty()) return 0.0;
            double sum = std::accumulate(vec.begin(), vec.end(), 0.0);
            return sum / vec.size();
        }

        template <typename T>
        static void calculateStats(const std::vector<T>& vec, T& min, double& avg, T& max) {
            if (vec.empty()) {
                min = T();
                avg = 0.0;
                max = T();
                return;
            }
            min = *std::min_element(vec.begin(), vec.end());
            max = *std::max_element(vec.begin(), vec.end());
            avg = mean(vec);
        }

        static void writeMatrixToCSV(const std::vector<std::vector<int>>& matrix, const std::string& filename);
    };

}

#endif // UTILS_H
