#include "Solution.h"
#include "Utils.h"

#include <algorithm>
#include <fstream>
#include <iostream>

namespace LS {

    void Solution::addNode(int node)
    {
        nodes.emplace_back(node);
        selectedNodes.emplace(node);
        ++numNodes;
    }

    void Solution::removeNode(int index)
    {
        if (index < 0 || index >= numNodes) return;
        selectedNodes.erase(nodes[index]);
        nodes.erase(nodes.begin() + index);
        --numNodes;
    }

    void Solution::removeNodes(int index, int amount)
    {
        for (int i = 0; i < amount; ++i)
        {
            removeNode(index);
        }
    }

    bool Solution::contains(int node) const
    {
        return selectedNodes.find(node) != selectedNodes.end();
    }

    const std::vector<int>& Solution::getNodes() const
    {
        return nodes;
    }

    void Solution::setNodes(const std::vector<int>& newNodes)
    {
        nodes = newNodes;
        numNodes = nodes.size();
        selectedNodes = std::set<int>(nodes.begin(), nodes.end());
    }

    int Solution::getNumberOfNodes() const
    {
        return numNodes;
    }

    int Solution::calculateNumberOfNodes() const
    {
        return nodes.size();
    }

    const std::set<int>& Solution::getSelectedNodes() const
    {
        return selectedNodes;
    }

    void Solution::setSelectedNodes(const std::set<int>& newSelected)
    {
        selectedNodes = newSelected;
    }

    void Solution::updateSelectedNodes()
    {
        selectedNodes = std::set<int>(nodes.begin(), nodes.end());
    }

    int Solution::getNodeAtIndex(int index) const
    {
        if (index < 0 || index >= numNodes) return -1;
        return nodes[index];
    }

    int Solution::getNextNodeIndex(int index) const
    {
        if (numNodes == 0) return -1;
        return (index + 1) % numNodes;
    }

    int Solution::getPrevNodeIndex(int index) const
    {
        if (numNodes == 0) return -1;
        return (index + numNodes - 1) % numNodes;
    }

    int Solution::findNodeIndex(int node) const
    {
        auto it = std::find(nodes.begin(), nodes.end(), node);
        if (it != nodes.end()) {
            return std::distance(nodes.begin(), it);
        }
        return -1;
    }

    void Solution::exchangeNodeAtIndex(int index, int newNode)
    {
        if (index < 0 || index >= numNodes) return;
        selectedNodes.erase(nodes[index]);
        nodes[index] = newNode;
        selectedNodes.emplace(newNode);
    }

    void Solution::exchangeTwoNodes(int index1, int index2)
    {
        if (index1 < 0 || index1 >= numNodes || index2 < 0 || index2 >= numNodes) return;
        std::swap(nodes[index1], nodes[index2]);
    }

    bool Solution::areConsecutive(int index1, int index2) const
    {
        int nextIdx1 = getNextNodeIndex(index1);
        int prevIdx1 = getPrevNodeIndex(index1);
        return (nextIdx1 == index2) || (prevIdx1 == index2);
    }

    void Solution::exchangeTwoEdges(int edgeIndex1, int edgeIndex2)
    {
        if (edgeIndex1 < 0 || edgeIndex1 >= numNodes ||
            edgeIndex2 < 0 || edgeIndex2 >= numNodes)
            return;

        if (edgeIndex2 < edgeIndex1)
            std::swap(edgeIndex1, edgeIndex2);

        auto start = nodes.begin() + edgeIndex1 + 1;
        auto end = nodes.begin() + edgeIndex2 + 1;
        std::reverse(start, end);
    }

    int Solution::evaluate(const std::vector<std::vector<int>>& distanceMatrix,
                          const std::vector<int>& costs) const
    {
        if (nodes.empty()) return 0;

        int totalCost = costs[nodes[0]];

        for (size_t i = 1; i < nodes.size(); ++i)
        {
            totalCost += costs[nodes[i]];
            totalCost += distanceMatrix[nodes[i-1]][nodes[i]];
        }

        // Add distance from last to first node to complete the cycle
        totalCost += distanceMatrix[nodes.back()][nodes[0]];

        return totalCost;
    }

    int Solution::mostBeneficialNode(const std::vector<int>& allDistances,
                                     const std::vector<int>& allCosts,
                                     const std::vector<int>& excludedNodes) const
    {
        int minIdx = -1;
        int minTotalCost = std::numeric_limits<int>::max();

        for (size_t i = 0; i < allDistances.size(); ++i)
        {
            if (!Utils::contains(excludedNodes, static_cast<int>(i)))
            {
                int tmpTotalCost = allDistances[i] + allCosts[i];
                if (tmpTotalCost < minTotalCost)
                {
                    minIdx = static_cast<int>(i);
                    minTotalCost = tmpTotalCost;
                }
            }
        }
        return minIdx;
    }

    int Solution::calculateDeltaInterRoute(const std::vector<std::vector<int>>& distanceMatrix,
                                          const std::vector<int>& costs,
                                          int exchangeIndex, int newNode) const
    {
        // Calculate node cost change
        int delta = costs[newNode] - costs[nodes[exchangeIndex]];

        // Calculate distance change
        int prevNodeIdx = getPrevNodeIndex(exchangeIndex);
        int nextNodeIdx = getNextNodeIndex(exchangeIndex);

        int prevNode = nodes[prevNodeIdx];
        int nextNode = nodes[nextNodeIdx];

        // Subtract distance to the node to be exchanged
        subtractDistanceFromDelta(delta, distanceMatrix, prevNode, nodes[exchangeIndex]);
        subtractDistanceFromDelta(delta, distanceMatrix, nodes[exchangeIndex], nextNode);

        // Add distance to the new node
        addDistanceToDelta(delta, distanceMatrix, prevNode, newNode);
        addDistanceToDelta(delta, distanceMatrix, newNode, nextNode);

        return delta;
    }

    int Solution::calculateDeltaIntraRouteNodes(const std::vector<std::vector<int>>& distanceMatrix,
                                               int firstIndex, int secondIndex) const
    {
        int delta = 0;

        int firstNode = nodes[firstIndex];
        int secondNode = nodes[secondIndex];

        if (firstNode == secondNode)
            return 0;

        // Get previous and next of the first node
        int firstPrevIdx = getPrevNodeIndex(firstIndex);
        int firstNextIdx = getNextNodeIndex(firstIndex);

        int firstPrevNode = nodes[firstPrevIdx];
        int firstNextNode = nodes[firstNextIdx];

        // Get previous and next of the second node
        int secondPrevIdx = getPrevNodeIndex(secondIndex);
        int secondNextIdx = getNextNodeIndex(secondIndex);

        int secondPrevNode = nodes[secondPrevIdx];
        int secondNextNode = nodes[secondNextIdx];

        if (secondNextNode == firstNode)
        {
            // Switch first node with second node
            // if second node precedes the first one
            std::swap(firstNode, secondNode);
        }

        // Subtract distances between first-previous and second-next
        subtractDistanceFromDelta(delta, distanceMatrix, firstPrevNode, firstNode);
        subtractDistanceFromDelta(delta, distanceMatrix, secondNode, secondNextNode);

        // Add distances between first-prev - second and first - second-next
        addDistanceToDelta(delta, distanceMatrix, firstPrevNode, secondNode);
        addDistanceToDelta(delta, distanceMatrix, firstNode, secondNextNode);

        if (areConsecutive(firstIndex, secondIndex))
        {
            // This is enough to do for consecutive nodes
            return delta;
        }

        // Subtract distances between first - first-next and second-prev - second
        subtractDistanceFromDelta(delta, distanceMatrix, firstNode, firstNextNode);
        subtractDistanceFromDelta(delta, distanceMatrix, secondPrevNode, secondNode);

        // Add distances between second - first-next and first - second-prev
        addDistanceToDelta(delta, distanceMatrix, secondNode, firstNextNode);
        addDistanceToDelta(delta, distanceMatrix, firstNode, secondPrevNode);

        return delta;
    }

    int Solution::calculateDeltaInterRouteNodesCandidates(const std::vector<std::vector<int>>& distanceMatrix,
                                                          const std::vector<int>& costs,
                                                          int firstIndex,
                                                          int candidateNode,
                                                          int& removedIndex,
                                                          const std::string& direction) const
    {
        int nFirstNodeIdx;
        int nNFirstNodeIdx;

        if (direction == "previous")
        {
            nFirstNodeIdx = getPrevNodeIndex(firstIndex);
            nNFirstNodeIdx = getPrevNodeIndex(nFirstNodeIdx);
        }
        else // "next"
        {
            nFirstNodeIdx = getNextNodeIndex(firstIndex);
            nNFirstNodeIdx = getNextNodeIndex(nFirstNodeIdx);
        }

        // Assign index of the node which will be removed
        removedIndex = nFirstNodeIdx;

        int firstNode = nodes[firstIndex];
        // Get the nodes connected by the edges
        int nFirstNode = nodes[nFirstNodeIdx];     // closest neighbor
        int nNFirstNode = nodes[nNFirstNodeIdx]; // second closest neighbor

        int delta = 0;
        delta += costs[candidateNode] - costs[nFirstNode];

        subtractDistanceFromDelta(delta, distanceMatrix, firstNode, nFirstNode);
        subtractDistanceFromDelta(delta, distanceMatrix, nFirstNode, nNFirstNode);

        addDistanceToDelta(delta, distanceMatrix, firstNode, candidateNode);
        addDistanceToDelta(delta, distanceMatrix, candidateNode, nNFirstNode);

        return delta;
    }

    int Solution::calculateDeltaIntraRouteEdges(const std::vector<std::vector<int>>& distanceMatrix,
                                               int firstEdgeIndex, int secondEdgeIndex) const
    {
        // Get the nodes that are connected by edges
        int node1Edge1 = nodes[firstEdgeIndex];
        int node2Edge1 = nodes[getNextNodeIndex(firstEdgeIndex)];
        int node1Edge2 = nodes[secondEdgeIndex];
        int node2Edge2 = nodes[getNextNodeIndex(secondEdgeIndex)];

        int delta = 0;
        // Subtract distances of erased edges
        delta -= distanceMatrix[node1Edge1][node2Edge1];
        delta -= distanceMatrix[node1Edge2][node2Edge2];

        // Add distances of new edges
        delta += distanceMatrix[node1Edge1][node1Edge2];
        delta += distanceMatrix[node2Edge1][node2Edge2];

        return delta;
    }

    void Solution::subtractDistanceFromDelta(int& delta, const std::vector<std::vector<int>>& distanceMatrix,
                                            int firstNode, int secondNode) const
    {
        delta -= distanceMatrix[firstNode][secondNode];
    }

    void Solution::addDistanceToDelta(int& delta, const std::vector<std::vector<int>>& distanceMatrix,
                                     int firstNode, int secondNode) const
    {
        delta += distanceMatrix[firstNode][secondNode];
    }

    void Solution::print() const
    {
        for (const auto& node : nodes)
        {
            std::cout << node << " ";
        }
        std::cout << std::endl;
    }

    
    void Solution::writeToCSV(const std::string& filename) const
    {
        std::ofstream myfile(filename);
        if (!myfile.is_open()) {
            throw std::runtime_error("Could not open file for writing: " + filename);
        }

        for (const auto& node : nodes)
        {
            myfile << node << "\n";
        }

        myfile.close();
    }

}
