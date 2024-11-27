#ifndef SOLUTION_H
#define SOLUTION_H

#include <vector>
#include <string>
#include <set>
#include <limits>

namespace LS {

    class Solution {
    protected:
        std::vector<int> nodes;
        int numNodes;
        std::set<int> selectedNodes;

    public:
        Solution() : numNodes(0) {}

        void addNode(int node);
        void removeNode(int index);
        void removeNodes(int index, int amount);
        bool contains(int node) const;

        const std::vector<int>& getNodes() const;
        void setNodes(const std::vector<int>& newNodes);

        int getNumberOfNodes() const;
        int calculateNumberOfNodes() const;

        const std::set<int>& getSelectedNodes() const;
        void setSelectedNodes(const std::set<int>& newSelected);
        void updateSelectedNodes();

        int getNodeAtIndex(int index) const;
        int getNextNodeIndex(int index) const;
        int getPrevNodeIndex(int index) const;
        int findNodeIndex(int node) const;

        void exchangeNodeAtIndex(int index, int newNode);
        void exchangeTwoNodes(int index1, int index2);
        bool areConsecutive(int index1, int index2) const;
        void exchangeTwoEdges(int edgeIndex1, int edgeIndex2);

        int evaluate(const std::vector<std::vector<int>>& distanceMatrix,
                    const std::vector<int>& costs) const;

        int mostBeneficialNode(const std::vector<int>& allDistances,
                               const std::vector<int>& allCosts,
                               const std::vector<int>& excludedNodes) const;

        int calculateDeltaInterRoute(const std::vector<std::vector<int>>& distanceMatrix,
                                     const std::vector<int>& costs,
                                     int exchangeIndex, int newNode) const;

        int calculateDeltaIntraRouteNodes(const std::vector<std::vector<int>>& distanceMatrix,
                                          int firstIndex, int secondIndex) const;

        int calculateDeltaInterRouteNodesCandidates(const std::vector<std::vector<int>>& distanceMatrix,
                                                    const std::vector<int>& costs,
                                                    int firstIndex,
                                                    int candidateNode,
                                                    int& removedIndex,
                                                    const std::string& direction) const;

        int calculateDeltaIntraRouteEdges(const std::vector<std::vector<int>>& distanceMatrix,
                                          int firstEdgeIndex, int secondEdgeIndex) const;

        void subtractDistanceFromDelta(int& delta, const std::vector<std::vector<int>>& distanceMatrix,
                                      int firstNode, int secondNode) const;

        void addDistanceToDelta(int& delta, const std::vector<std::vector<int>>& distanceMatrix,
                               int firstNode, int secondNode) const;

        void print() const;
        void calculateCostBreakdown(const std::vector<std::vector<int>>& distanceMatrix,
                                    const std::vector<int>& costs,
                                    int& pathLength,
                                    int& nodeCosts) const;
        
        void writeToCSV(const std::string& filename) const;
    };

}

#endif // SOLUTION_H
