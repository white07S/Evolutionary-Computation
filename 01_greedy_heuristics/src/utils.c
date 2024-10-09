#include "utils.h"

int calculate_cost(const int* solution, int solution_size, const int** distances, const int* costs) {
    int cost = 0;
    for(int j = 0; j < solution_size - 1; j++) {
        cost += distances[solution[j]][solution[j+1]];
    }
    cost += distances[solution[solution_size -1]][solution[0]];
    for(int j = 0; j < solution_size; j++) {
        cost += costs[solution[j]];
    }
    return cost;
}

void calculate_cost_breakdown(const int* solution, int solution_size, const int** distances, const int* costs, int* path_length, int* node_costs) {
    int length = 0;
    int n_costs = 0;
    for(int j = 0; j < solution_size - 1; j++) {
        length += distances[solution[j]][solution[j+1]];
    }
    length += distances[solution[solution_size -1]][solution[0]];
    for(int j = 0; j < solution_size; j++) {
        n_costs += costs[solution[j]];
    }
    *path_length = length;
    *node_costs = n_costs;
}

void shuffle_array(int* array, int n) {
    if (n > 1) {
        for(int i = n -1; i > 0; i--) {
            int j = rand() % (i +1);
            int temp = array[j];
            array[j] = array[i];
            array[i] = temp;
        }
    }
}

int** read_file(const char* filename, int* num_nodes) {
    FILE* file = fopen(filename, "r");
    if(!file) {
        fprintf(stderr, "Error: Cannot open file %s\n", filename);
        *num_nodes = 0;
        return NULL;
    }

    char line[256];
    *num_nodes = 0;

    // Count the number of lines
    while(fgets(line, sizeof(line), file)) {
        (*num_nodes)++;
    }
    rewind(file);

    int** data = (int**)malloc((*num_nodes) * sizeof(int*));
    if(!data) {
        fprintf(stderr, "Error: Memory allocation failed for data\n");
        fclose(file);
        *num_nodes = 0;
        return NULL;
    }

    int node = 0;
    while(fgets(line, sizeof(line), file)) {
        data[node] = (int*)malloc(3 * sizeof(int));
        if(!data[node]) {
            fprintf(stderr, "Error: Memory allocation failed for data[%d]\n", node);
            for(int k = 0; k < node; k++) {
                free(data[k]);
            }
            free(data);
            fclose(file);
            *num_nodes = 0;
            return NULL;
        }

        char* token = strtok(line, ";");
        int col = 0;
        while(token && col < 3) {
            data[node][col] = atoi(token);
            token = strtok(NULL, ";");
            col++;
        }
        node++;
    }

    fclose(file);
    return data;
}

int** calcDistances(int** data, int num_nodes) {
    int** distances = (int**)malloc(num_nodes * sizeof(int*));
    if(!distances) {
        fprintf(stderr, "Error: Memory allocation failed for distances\n");
        return NULL;
    }

    for(int i = 0; i < num_nodes; i++) {
        distances[i] = (int*)malloc(num_nodes * sizeof(int));
        if(!distances[i]) {
            fprintf(stderr, "Error: Memory allocation failed for distances[%d]\n", i);
            for(int k = 0; k < i; k++) {
                free(distances[k]);
            }
            free(distances);
            return NULL;
        }
    }

    for(int i = 0; i < num_nodes; i++) {
        for(int j = 0; j < num_nodes; j++) {
            int x_diff = data[i][0] - data[j][0];
            int y_diff = data[i][1] - data[j][1];
            double distance = sqrt((double)(x_diff * x_diff + y_diff * y_diff));
            distance = round(distance);
            distances[i][j] = (int)distance;
        }
    }

    return distances;
}

int is_valid_solution(const int* solution, int solution_size, int num_nodes) {
    char* visited = (char*)calloc(num_nodes, sizeof(char));
    if(!visited) {
        fprintf(stderr, "Error: Memory allocation failed in is_valid_solution\n");
        return 0;
    }
    for(int i = 0; i < solution_size; i++) {
        int node = solution[i];
        if(node < 0 || node >= num_nodes) {
            free(visited);
            return 0;
        }
        if(visited[node]) {
            free(visited);
            return 0;
        }
        visited[node] = 1;
    }
    free(visited);
    return 1;
}

void write_solution_to_file(const int* sol, int sol_size, const char* filename) {
    FILE* file = fopen(filename, "a"); // Append mode
    if(!file) {
        fprintf(stderr, "Error: Cannot open file %s for writing\n", filename);
        return;
    }
    for(int i =0; i < sol_size; i++) {
        fprintf(file, "%d,", sol[i]);
    }
    fprintf(file, "\n");
    fclose(file);
}

void write_Result_to_file(const Result res, const char* filename, const int** distances, const int* costs) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Error: Could not open file %s for writing\n", filename);
        return;
    }

    fprintf(file, "Best cost: %d\n", res.bestCost);
    fprintf(file, "Worst cost: %d\n", res.worstCost);
    fprintf(file, "Average cost: %.2lf\n", res.averageCost);

    // Calculate and print the total cost breakdown for the best solution
    int path_length = 0;
    int node_costs = 0;
    calculate_cost_breakdown(res.bestSolution, res.bestSolutionSize, distances, costs, &path_length, &node_costs);

    fprintf(file, "Best solution total cost breakdown:\n");
    fprintf(file, " - Path length: %d\n", path_length);
    fprintf(file, " - Node costs: %d\n", node_costs);

    fprintf(file, "Best solution: ");
    for (int i = 0; i < res.bestSolutionSize; i++) {
        fprintf(file, "%d ", res.bestSolution[i]);
    }
    fprintf(file, "\n");

    fclose(file);
}

void free_data(int** data, int num_nodes) {
    for (int i = 0; i < num_nodes; i++) {
        free(data[i]);
    }
    free(data);
}

void free_distances(int** distances, int num_nodes) {
    for(int i =0; i < num_nodes; i++) {
        free(distances[i]);
    }
    free(distances);
}
