# import numpy as np
# import pandas as pd
# import pathlib
# import matplotlib.pyplot as plt

# INSTANCES = ["TSPA_solutions.csv",
#              "TSPB_solutions.csv"]


# class Solution:
#     def __init__(self, sol_array) -> None:
#         self.solution = sol_array[:-1]
#         self.evaluation = sol_array[-1]

#         self.nodes = self.make_set_of_nodes(self.solution)
#         self.edges = self.make_set_of_edges(self.solution)

#     def make_set_of_nodes(self, arr: np.ndarray):
#         return set(arr)

#     def make_set_of_edges(self, arr: np.ndarray):
#         edges = []
#         for idx in range(len(arr)-1):
#             edges.append(sorted([arr[idx], arr[idx+1]]))
#         edges.append(sorted([arr[0], arr[-1]]))

#         return set(tuple(x) for x in edges)

# def read_data(path) -> list[Solution]:
#     solutions = []
#     dataset = pd.read_csv(path, header=None).to_numpy()
#     for sol in dataset[:-1]:
#         solutions.append(Solution(sol_array=sol))

#     return solutions, Solution(sol_array=dataset[-1])

# solutions, best_solution = read_data("TSPA_solutions.csv")


# def read_data(path) -> list[Solution]:
#     solutions = []
#     dataset = pd.read_csv(path, header=None).to_numpy()
#     for sol in dataset[:-1]:
#         solutions.append(Solution(sol_array=sol))

#     return solutions, Solution(sol_array=dataset[-1])

# solutions, best_solution = read_data("TSPA_solutions.csv")


# def get_comparison_list(solutions: list[Solution],
#                         best_solution: Solution,
#                         similarity_version: str):

#     if similarity_version == "BEST":
#         comparison_list = [best_solution]
#     if similarity_version == "AVERAGE":
#         comparison_list = solutions

#     return comparison_list

# def plot_similarity(ax,
#                     solution_evals: list,
#                     similarity_avgs: list,
#                     similarity_version: str,
#                     similarity_measure: str):
#     correlation = np.corrcoef(solution_evals,similarity_avgs)[0,1]
#     correlation = np.round(correlation, 3)

#     ax.scatter(x=solution_evals,
#                 y=similarity_avgs,
#                 s=10)
#     ax.set_xlabel("EVALUATION")
#     ax.set_ylabel("SIMILARITY")
#     ax.set_title(f"{similarity_version.lower()}_{similarity_measure.lower()}_({str(correlation)})",
#               fontsize=15)

# def plot_similarity(ax,
#                     solution_evals: list,
#                     similarity_avgs: list,
#                     similarity_version: str,
#                     similarity_measure: str):
#     correlation = np.corrcoef(solution_evals,similarity_avgs)[0,1]
#     correlation = np.round(correlation, 3)

#     ax.scatter(x=solution_evals,
#                 y=similarity_avgs,
#                 s=10)
#     ax.set_xlabel("EVALUATION")
#     ax.set_ylabel("SIMILARITY")
#     ax.set_title(f"{similarity_version.lower()}_{similarity_measure.lower()}_({str(correlation)})",
#               fontsize=15)



# for instance in INSTANCES:
#     solutions, best_solution = read_data(instance)

#     fig, axs = plt.subplots(2, 2, figsize=(15,10))
#     fig.tight_layout(pad=6)
#     fig.suptitle(instance.split("_")[0], fontsize=20)
#     axs = axs.flatten()
#     idx = 0

#     for similarity_version in ["BEST", "AVERAGE"]:
#         comparison_list = get_comparison_list(solutions,best_solution, similarity_version)

#         for similarity_measure in ["NODES", "EDGES"]:

#             similarity_avgs = []
#             solution_evals = []
#             for solution in solutions:

#                 comparison_list_tmp = [x for x in comparison_list if x != solution]
#                 similarities = []
#                 for comp_solution in comparison_list_tmp:
#                     similarities.append(similarities(sol1=solution,
#                                                    sol2=comp_solution,
#                                                    similarity_measure=similarity_measure))

#                 similarity_avgs.append(np.average(similarities))
#                 solution_evals.append(solution.evaluation)

#             plot_similarity(ax = axs[idx],
#                             solution_evals=solution_evals,
#                             similarity_avgs=similarity_avgs,
#                             similarity_measure=similarity_measure,
#                             similarity_version=similarity_version)
#             idx+=1

#     fig.savefig(f"plots/{instance.split('_')[0]}")
#     plt.close()



import numpy as np
import pandas as pd
import pathlib
import matplotlib.pyplot as plt



INSTANCES = ["TSPA_solutions.csv",
             "TSPB_solutions.csv"]


class Solution:
    def __init__(self, sol_array) -> None:
        self.solution = sol_array[:-1]
        self.evaluation = sol_array[-1]

        self.nodes = self.make_set_of_nodes(self.solution)
        self.edges = self.make_set_of_edges(self.solution)

    def make_set_of_nodes(self, arr: np.ndarray):
        return set(arr)
    
    def make_set_of_edges(self, arr: np.ndarray):
        edges = []
        for idx in range(len(arr)-1):
            edges.append(sorted([arr[idx], arr[idx+1]]))
        edges.append(sorted([arr[0], arr[-1]]))

        return set(tuple(x) for x in edges)


def read_data(path) -> list[Solution]:
    solutions = []
    dataset = pd.read_csv(path, header=None).to_numpy()
    for sol in dataset[:-1]:
        solutions.append(Solution(sol_array=sol))

    return solutions, Solution(sol_array=dataset[-1])
    
solutions, best_solution = read_data("TSPA_solutions.csv")


def similarity(sol1: Solution,
               sol2: Solution,
               similarity_measure: str):
    
    if similarity_measure == "NODES":
        set1, set2 = sol1.nodes, sol2.nodes

    if similarity_measure == "EDGES":
        set1, set2 = sol1.edges, sol2.edges

    return len(set1.intersection(set2))

similarity(sol1=solutions[0],
           sol2=best_solution, 
           similarity_measure="EDGES")


def get_comparison_list(solutions: list[Solution],
                        best_solution: Solution, 
                        similarity_version: str):
    
    if similarity_version == "BEST":
        comparison_list = [best_solution]
    if similarity_version == "AVERAGE":
        comparison_list = solutions

    return comparison_list


def plot_similarity(ax,
                    solution_evals: list,
                    similarity_avgs: list,
                    similarity_version: str,
                    similarity_measure: str):
    correlation = np.corrcoef(solution_evals,similarity_avgs)[0,1]
    correlation = np.round(correlation, 3)

    ax.scatter(x=solution_evals,
                y=similarity_avgs,
                s=10)
    ax.set_xlabel("EVALUATION")
    ax.set_ylabel("SIMILARITY")
    ax.set_title(f"{similarity_version.lower()}_{similarity_measure.lower()}_({str(correlation)})",
              fontsize=15)



for instance in INSTANCES:
    solutions, best_solution = read_data(instance)

    fig, axs = plt.subplots(2, 2, figsize=(15,10))
    fig.tight_layout(pad=6)
    fig.suptitle(instance.split("_")[0], fontsize=20)
    axs = axs.flatten()
    idx = 0
    
    for similarity_version in ["BEST", "AVERAGE"]:
        comparison_list = get_comparison_list(solutions,best_solution, similarity_version)

        for similarity_measure in ["NODES", "EDGES"]:

            similarity_avgs = []
            solution_evals = []
            for solution in solutions:

                comparison_list_tmp = [x for x in comparison_list if x != solution]
                similarities = []
                for comp_solution in comparison_list_tmp:
                    similarities.append(similarity(sol1=solution,
                                                   sol2=comp_solution,
                                                   similarity_measure=similarity_measure))
                    
                similarity_avgs.append(np.average(similarities))
                solution_evals.append(solution.evaluation)

            plot_similarity(ax = axs[idx],
                            solution_evals=solution_evals,
                            similarity_avgs=similarity_avgs,
                            similarity_measure=similarity_measure,
                            similarity_version=similarity_version)
            idx+=1

    fig.savefig(f"plots/{instance.split('_')[0]}")
    plt.close()


