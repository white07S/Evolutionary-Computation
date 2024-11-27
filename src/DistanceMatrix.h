#ifndef DISTANCE_MATRIX_H
#define DISTANCE_MATRIX_H

#include <vector>
#include <string>

namespace LS {

    class DistanceMatrix {
    private:
        std::vector<std::vector<int>> distanceMatrix;
        std::vector<int> costs;

    public:
        void create(const std::string& filename);
        void readCoordinates(const std::string& filename,
                             std::vector<int>& xs,
                             std::vector<int>& ys);
        const std::vector<std::vector<int>>& getDistanceMatrix() const;
        const std::vector<int>& getCosts() const;
        void printDistanceMatrix() const;
    };

}

#endif // DISTANCE_MATRIX_H
