# Solution for Greedy Heuristic Methods

## Problem Description
We are given three columns of integers with a row for each node. The first two columns contain x
and y coordinates of the node positions in a plane. The third column contains node costs. The goal is
to select exactly 50% of the nodes (if the number of nodes is odd we round the number of nodes to
be selected up) and form a Hamiltonian cycle (closed path) through this set of nodes such that the
sum of the total length of the path plus the total cost of the selected nodes is minimized.
The distances between nodes are calculated as Euclidean distances rounded mathematically to
integer values. The distance matrix should be calculated just after reading an instance and then only
the distance matrix (no nodes coordinates) should be accessed by optimization methods to allow
instances defined only by distance matrices.

## Methods

### Random Solution
```
pseudocode goes here
```
<p float="left">
  <img src="/01_greedy_heuristics/plots/RandomSearch_TSPA.png" width="400" />
  <img src="/01_greedy_heuristics/plots/RandomSearch_TSPB.png" width="400" /> 
</p>


### Nearest neighbor considering adding the node only at the end of the current path
```
pseudocode goes here
```
<p float="left">
  <img src="/01_greedy_heuristics/plots/NearestNeighboursEndInsert_TSPA.png" width="400" />
  <img src="/01_greedy_heuristics/plots/NearestNeighboursEndInsert_TSPB.png" width="400" /> 
</p>

### Nearest neighbor considering adding the node at all possible position
```
pseudocode goes here
```
<p float="left">
  <img src="/01_greedy_heuristics/plots/NearestNeighboursAnywhereInsert_TSPA.png" width="400" />
  <img src="/01_greedy_heuristics/plots/NearestNeighboursAnywhereInsert_TSPB.png" width="400" /> 
</p>

### Greedy cycle
```
pseudocode goes here
```
<p float="left">
  <img src="/01_greedy_heuristics/plots/GreedyCycle_TSPA.png" width="400" />
  <img src="/01_greedy_heuristics/plots/GreedyCycle_TSPB.png" width="400" /> 
</p>
