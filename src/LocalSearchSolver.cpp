#include "LocalSearchSolver.h"
#include "RandomSolution.h"

#include <algorithm>
#include <numeric>
#include <iostream>
#include <limits>
#include <random>
#include <chrono>

namespace LS {

    LocalSearchSolver::LocalSearchSolver(const std::string& instanceFilename, double fractionNodes, const Solution& initialSolution)
        : BaseSolver(instanceFilename, fractionNodes), bestSolution(initialSolution)
    {
        bestSolution.setNodes(initialSolution.getNodes());
        bestSolution.setSelectedNodes(initialSolution.getSelectedNodes());
        bestSolutionEvaluation = bestSolution.evaluate(distanceMatrix, costs);

        iterator1.reserve(bestSolution.getNumberOfNodes());
        iterator1.resize(bestSolution.getNumberOfNodes());
        std::iota(iterator1.begin(), iterator1.end(), 0);

        iterator2.reserve(bestSolution.getNumberOfNodes());
        iterator2.resize(bestSolution.getNumberOfNodes());
        std::iota(iterator2.begin(), iterator2.end(), 0);

        iteratorLong.reserve(totalNodes);
        iteratorLong.resize(totalNodes);
        std::iota(iteratorLong.begin(), iteratorLong.end(), 0);

        std::random_device rd;
        rng.seed(rd());
    }

    void LocalSearchSolver::reset()
    {
        // Set new random solution
        RandomSolution newInitialSolution;
        newInitialSolution.generate(totalNodes, numNodes);
        bestSolution = newInitialSolution;
        bestSolution.setNodes(newInitialSolution.getNodes());
        bestSolution.setSelectedNodes(newInitialSolution.getSelectedNodes());

        bestSolutionEvaluation = bestSolution.evaluate(distanceMatrix, costs);
    }

    void LocalSearchSolver::setInitialSolution(const Solution& newInitialSolution)
    {
        bestSolution = newInitialSolution;
        bestSolutionEvaluation = bestSolution.evaluate(distanceMatrix, costs);
    }

    void LocalSearchSolver::setInitialSolutionCopy(const Solution& newInitialSolution)
    {
        bestSolution = newInitialSolution;
        bestSolution.setNodes(newInitialSolution.getNodes());
        bestSolution.setSelectedNodes(newInitialSolution.getSelectedNodes());
        bestSolutionEvaluation = bestSolution.evaluate(distanceMatrix, costs);
    }

    void LocalSearchSolver::writeBestToCSV(const std::string& filename)
    {
        bestSolution.writeToCSV(filename);
    }

    int LocalSearchSolver::getBestSolutionEval() const
    {
        return bestSolutionEvaluation;
    }

    std::vector<int> LocalSearchSolver::getBestSolution() const
    {
        return bestSolution.getNodes();
    }

    Solution LocalSearchSolver::getBestFullSolution() const
    {
        return bestSolution;
    }

    Solution* LocalSearchSolver::getBestSolutionPtr()
    {
        return &bestSolution;
    }

    void LocalSearchSolver::perturbBestSolution(int n)
    {
        std::uniform_int_distribution<int> distEdge(0, 99); // Assuming 100 edges
        for (int i = 0; i < n; ++i)
        {
            int edge1 = 0;
            int edge2 = 0;
            while (std::abs(edge1 - edge2) < 2)
            {
                edge1 = distEdge(rng);
                edge2 = distEdge(rng);
                if (edge1 > edge2)
                {
                    std::swap(edge1, edge2);
                }
            }
            int delta = bestSolution.calculateDeltaIntraRouteEdges(distanceMatrix, edge1, edge2);
            bestSolutionEvaluation += delta;
            bestSolution.exchangeTwoEdges(edge1, edge2);
        }
    }

    void LocalSearchSolver::destroyAndRepairBestSolution()
    {
        // Destroy
        int destroySequencesAmount = (rng() % 4) + 2;
        int length = bestSolution.getNumberOfNodes() / (4 * destroySequencesAmount);

        for (int idx = 0; idx < destroySequencesAmount; ++idx)
        {
            std::uniform_int_distribution<int> distIndex(0, bestSolution.getNumberOfNodes() - length);
            int indexF = distIndex(rng);
            bestSolution.removeNodes(indexF, length);
        }

        // Repair
        std::vector<int> tmpSol;
        greedyCycleRepair(tmpSol);

        bestSolution.setNodes(tmpSol);
        bestSolution.updateSelectedNodes();
        bestSolutionEvaluation = bestSolution.evaluate(distanceMatrix, costs);
    }

    void LocalSearchSolver::destroyAndRepairBestSolutionV2()
    {
        // Similar to destroyAndRepairBestSolution with slight variations
        int destroySequencesAmount = (rng() % 3) + 2;
        int length = bestSolution.getNumberOfNodes() / (4 * destroySequencesAmount);

        for (int idx = 0; idx < destroySequencesAmount; ++idx)
        {
            std::uniform_int_distribution<int> distIndex(0, bestSolution.getNumberOfNodes() - length);
            int indexF = distIndex(rng);
            bestSolution.removeNodes(indexF, length);
        }

        // Repair
        std::vector<int> tmpSol;
        greedyCycleRepair(tmpSol);

        bestSolution.setNodes(tmpSol);
        bestSolution.updateSelectedNodes();
        bestSolutionEvaluation = bestSolution.evaluate(distanceMatrix, costs);
    }

    void LocalSearchSolver::greedyCycleRepair(std::vector<int>& correctOrderNodes)
    {
        std::vector<std::vector<int>> edges;
        const auto& nodes = bestSolution.getNodes();
        for (size_t idx = 0; idx < nodes.size() - 1; idx++)
        {
            edges.emplace_back(std::vector<int>{nodes[idx], nodes[idx + 1]});
        }
        edges.emplace_back(std::vector<int>{nodes.back(), nodes[0]});

        while (bestSolution.getNumberOfNodes() < numNodes)
        {
            int nodeToAddIdx = -1;
            int edgeToRemoveIdx = -1;
            int minTotalCost = std::numeric_limits<int>::max();

            for (size_t nodeIdx = 0; nodeIdx < distanceMatrix.size(); nodeIdx++)
            {
                if (!bestSolution.contains(nodeIdx))
                {
                    for (size_t edgeIdx = 0; edgeIdx < edges.size(); edgeIdx++)
                    {
                        int node1 = edges[edgeIdx][0];
                        int node2 = edges[edgeIdx][1];

                        int totalCost = distanceMatrix[node1][nodeIdx] +
                                        distanceMatrix[node2][nodeIdx] -
                                        distanceMatrix[node1][node2] + costs[nodeIdx];

                        if (totalCost < minTotalCost)
                        {
                            minTotalCost = totalCost;
                            edgeToRemoveIdx = edgeIdx;
                            nodeToAddIdx = nodeIdx;
                        }
                    }
                }
            }

            if (edgeToRemoveIdx == -1 || nodeToAddIdx == -1) break; // No possible addition

            int nodeToConnect1 = edges[edgeToRemoveIdx][0];
            int nodeToConnect2 = edges[edgeToRemoveIdx][1];

            edges.erase(edges.begin() + edgeToRemoveIdx);
            edges.emplace_back(std::vector<int>{nodeToConnect1, nodeToAddIdx});
            edges.emplace_back(std::vector<int>{nodeToConnect2, nodeToAddIdx});

            bestSolution.addNode(nodeToAddIdx);
        }

        if (!edges.empty())
        {
            correctOrderNodes.emplace_back(edges[0][0]);
            while (edges.size() > 1)
            {
                bool found = false;
                for (size_t i = 0; i < edges.size(); )
                {
                    if (edges[i][0] == correctOrderNodes.back())
                    {
                        correctOrderNodes.emplace_back(edges[i][1]);
                        edges.erase(edges.begin() + i);
                        found = true;
                    }
                    else if (edges[i][1] == correctOrderNodes.back())
                    {
                        correctOrderNodes.emplace_back(edges[i][0]);
                        edges.erase(edges.begin() + i);
                        found = true;
                    }
                    else
                    {
                        ++i;
                    }
                    if (found) break;
                }
            }
        }
    }

    void LocalSearchSolver::runBasic(const std::string& neighborhoodMethod, const std::string& searchMethod)
    {
        int currentBestDelta = -1;
        int bestInterDelta, bestIntraNodesDelta, bestIntraEdgesDelta;

        int arg1, arg2;
        std::string moveType;

        // Define neighborhood methods
        typedef void (LocalSearchSolver::*VoidFunctionFourParams)(int&, int&, int&, const std::string&);
        std::vector<VoidFunctionFourParams> neighborhoodMethods = {
            &LocalSearchSolver::findBestInterNeighbor,
            &LocalSearchSolver::findBestIntraNeighborNodes,
            &LocalSearchSolver::findBestIntraNeighborEdges
        };

        std::vector<std::string> moveTypes = { "inter", "intra_nodes", "intra_edges" };

        std::vector<int> neighborhoodMethodsIdxs = { 0 };

        int methodIdx = 0;
        if (neighborhoodMethod == "TWO_NODES")
        {
            methodIdx = 1;
        }
        if (neighborhoodMethod == "TWO_EDGES")
        {
            methodIdx = 2;
        }

        // Randomly decide to include the methodIdx
        std::uniform_int_distribution<int> dist(0,1);
        if (dist(rng) == 1)
        {
            neighborhoodMethodsIdxs.push_back(methodIdx);
        }
        else
        {
            neighborhoodMethodsIdxs.insert(neighborhoodMethodsIdxs.begin(), methodIdx);
        }

        // Iterate until no improvement
        while (currentBestDelta < 0)
        {
            currentBestDelta = 0;

            for (const auto& i : neighborhoodMethodsIdxs)
            {
                int tempBestEval, tempArg1, tempArg2;
                (this->*neighborhoodMethods[i])(tempBestEval, tempArg1, tempArg2, searchMethod);

                if (tempBestEval < currentBestDelta)
                {
                    arg1 = tempArg1;
                    arg2 = tempArg2;
                    moveType = moveTypes[i];
                    currentBestDelta = tempBestEval;

                    if (searchMethod == "GREEDY")
                    {
                        break;
                    }
                }
            }

            if (currentBestDelta >= 0)
            {
                // No improvement
                break;
            }

            bestSolutionEvaluation += currentBestDelta;
            applyMove(moveType, arg1, arg2);
        }
    }

    void LocalSearchSolver::findBestInterNeighbor(int& outDelta, int& exchangedNode, int& newNode, const std::string& searchMethod)
    {
        // Finds best neighbor by exchanging some selected node with a not selected node
        int delta = 0;
        int minDelta = 0;
        int minExchangedIdx = -1;
        int minNewNode = -1;

        if (searchMethod == "GREEDY")
        {
            std::shuffle(iterator1.begin(), iterator1.end(), rng);
            std::shuffle(iteratorLong.begin(), iteratorLong.end(), rng);
        }

        for (const auto& j : iteratorLong)
        {
            if (!bestSolution.contains(j))
            {
                for (const auto& i : iterator1)
                {
                    delta = bestSolution.calculateDeltaInterRoute(distanceMatrix, costs, i, j);
                    if (delta < minDelta)
                    {
                        minDelta = delta;
                        minExchangedIdx = i;
                        minNewNode = j;

                        if (searchMethod == "GREEDY")
                        {
                            outDelta = minDelta;
                            exchangedNode = minExchangedIdx;
                            newNode = minNewNode;
                            return;
                        }
                    }
                }
            }
        }

        outDelta = minDelta;
        exchangedNode = minExchangedIdx;
        newNode = minNewNode;
    }

    void LocalSearchSolver::findBestIntraNeighborNodes(int& outDelta, int& firstNodeIdx, int& secondNodeIdx, const std::string& searchMethod)
    {
        int minDelta = 0;
        int minNode1Idx = -1;
        int minNode2Idx = -1;
        int delta = 0;

        if (searchMethod == "GREEDY")
        {
            std::shuffle(iterator1.begin(), iterator1.end(), rng);
            std::shuffle(iterator2.begin(), iterator2.end(), rng);
        }

        for (const auto& node1Idx : iterator1)
        {
            for (const auto& node2Idx : iterator2)
            {
                if (node1Idx < node2Idx)
                {
                    delta = bestSolution.calculateDeltaIntraRouteNodes(distanceMatrix, node1Idx, node2Idx);
                    if (delta < minDelta)
                    {
                        minDelta = delta;
                        minNode1Idx = node1Idx;
                        minNode2Idx = node2Idx;

                        if (searchMethod == "GREEDY")
                        {
                            outDelta = minDelta;
                            firstNodeIdx = minNode1Idx;
                            secondNodeIdx = minNode2Idx;
                            return;
                        }
                    }
                }
            }
        }

        outDelta = minDelta;
        firstNodeIdx = minNode1Idx;
        secondNodeIdx = minNode2Idx;
    }

    void LocalSearchSolver::findBestIntraNeighborEdges(int& outDelta, int& firstEdgeIdx, int& secondEdgeIdx, const std::string& searchMethod)
    {
        int minDelta = 0;
        int minEdge1Idx = -1;
        int minEdge2Idx = -1;
        int delta = 0;

        if (searchMethod == "GREEDY")
        {
            std::shuffle(iterator1.begin(), iterator1.end(), rng);
            std::shuffle(iterator2.begin(), iterator2.end(), rng);
        }

        for (const auto& edge1Idx : iterator1)
        {
            for (const auto& edge2Idx : iterator2)
            {
                if (std::abs(edge1Idx - edge2Idx) > 1)
                {
                    delta = bestSolution.calculateDeltaIntraRouteEdges(distanceMatrix, edge1Idx, edge2Idx);
                    if (delta < minDelta)
                    {
                        minDelta = delta;
                        minEdge1Idx = edge1Idx;
                        minEdge2Idx = edge2Idx;

                        if (searchMethod == "GREEDY")
                        {
                            outDelta = minDelta;
                            firstEdgeIdx = minEdge1Idx;
                            secondEdgeIdx = minEdge2Idx;
                            return;
                        }
                    }
                }
            }
        }

        outDelta = minDelta;
        firstEdgeIdx = minEdge1Idx;
        secondEdgeIdx = minEdge2Idx;
    }

    void LocalSearchSolver::applyMove(const std::string& moveType, int arg1, int arg2)
    {
        if (moveType == "inter")
        {
            bestSolution.exchangeNodeAtIndex(arg1, arg2);
        }
        else if (moveType == "intra_nodes")
        {
            bestSolution.exchangeTwoNodes(arg1, arg2);
        }
        else if (moveType == "intra_edges")
        {
            bestSolution.exchangeTwoEdges(arg1, arg2);
        }
    }

}
