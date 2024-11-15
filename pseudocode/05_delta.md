

## the Backbone

``` 
Algorithm 1 - Backbone of the local search with deltas

LM <- initialize a Queue of improving moves LM

current solution <- Generate a random solution

successor <- for each position in the solution, save the node's successor
predecessor <- for each position in the solution, save the node's predecessor

current cost <- calculate cost of a current solution


// perform local search
repeat:
	// first, check the LM
	repeat: 
		move <- get the next move from the LM
		update status <- update move (See alg 2)

		if update status == "edge to be removed"
			remove the move from LM
		elif update status == "edges reversed or mixed"
			keep a move in LM, but don't apply
		else (update status = "edges present, in correct order")
			// an applicable move has been found
			apply a move
			update current cost
			remove move from LM 
			break

	until an applicable move has been found or all moves from LM examined and not applicable

	// if no improving move in LM, evaluate all the possible moves
	delta_best = 0
	move_best = None
	
	// intra-route moves
	// NOTE: iterating over all nodes ensures that both cases of new edge placement is considered, both before and after original target node
	for each node_i in current solution:
		for each node_j that comes after node_i but is not subsequent to node_i:
		delta <- evaluate the two edge exchange for node_i, node_j (See alg 3)
		if delta < 0:
			move <- save the information about the move: introduced edges, move type, delta
			add a move to the LM
			
			if delta < delta_best:
				move_best <- move


		
	

	// inter-route moves
	for each node_i in current solution:
		for each node_j in nodes:
			if node_j is not in the current solution: // we keep track of the nodes in the current solution using boolean array
				delta <- evaluate the two nodes exchange for node_i, node_j (See alg 4)
				
				if delta < 0:
					move <- save the information about the move: introduced edges, move type, delta
					add a move to the LM
					
					if delta < delta_best:
						move_best <- move

	if delta_best < 0:
		apply the move_best
		current cost <- current cost + delta_best
		remove move_best from LM
					
	


until no improvement


```