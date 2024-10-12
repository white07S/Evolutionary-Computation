import pandas as pd
import numpy as np
from pathlib import Path
import argparse
import os
import glob
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors
from matplotlib.cm import ScalarMappable
from datetime import datetime

ALGORITHMS_ORDERED = ['RandomSearch', 'NearestNeighboursEndInsert', 'NearestNeighboursAnywhereInsert', 'GreedyCycle']

script_dir = Path(__file__).parent

def read_instance(experiment_dir, data_filename) -> dict:
    data_path = script_dir / experiment_dir / "data" / data_filename
    df = pd.read_csv(data_path, header=None, names=['x', 'y', 'cost'], sep=";")
    instance_dict = df.to_dict(orient='index')
    return instance_dict
     
def read_algorithm_result(filename) -> dict:
    result_dict= {}
    with open(filename) as f:
        result_dict['experiment_name'] = filename.split('/')[-1].rstrip('_result.txt')
        result_dict['best_cost'] = int(f.readline().split(':')[-1])
        result_dict['worst_cost'] = int(f.readline().split(':')[-1])
        result_dict['average_cost'] = float(f.readline().split(':')[-1])
        f.readline()
        result_dict['path_lengths'] = int(f.readline().split(':')[-1])
        result_dict['node_costs'] = int(f.readline().split(':')[-1])
        result_dict['best_solution'] = f.readline().split(':')[-1].split()
        result_dict['best_solution'] = [int(x) for x in result_dict['best_solution']]
    return result_dict


def shift_solution(solution):
    n = len(solution)
    new_solution = [0] * n
    min_id = solution.index(min(solution))
    for i in range(n):
        new_solution[i] = solution[(i+min_id)%n]
    return new_solution


def calculate_distance(x1, y1, x2, y2):
    return round(
        ((x1 - x2)**2 + (y1-y2)**2)**(1/2)
    )

def validate_result(tsp_dict, result_dict):
    node_costs = 0
    path_lengths = 0
    solution: list = result_dict['best_solution']
    for i in range(len(solution)-1):
        node_costs += tsp_dict[solution[i]]['cost']
        path_lengths += calculate_distance(
                                         tsp_dict[solution[i]]['x'],
                                         tsp_dict[solution[i]]['y'],
                                         tsp_dict[solution[i+1]]['x'],
                                         tsp_dict[solution[i+1]]['y']
                                         )
    
    # add the last connection between the last and the first node 
    node_costs += tsp_dict[solution[-1]]['cost']
    path_lengths += calculate_distance(
                                         tsp_dict[solution[-1]]['x'],
                                         tsp_dict[solution[-1]]['y'],
                                         tsp_dict[solution[0]]['x'],
                                         tsp_dict[solution[0]]['y']
                                         )
    
    assert path_lengths == result_dict['path_lengths']
    assert node_costs == result_dict['node_costs']
    
def save_plot_experiment(out_dir, result_dict, tsp_dict):
    
    tsp_df = pd.DataFrame.from_dict(tsp_dict, orient='index')
    
        # Normalize the costs for color mapping
    norm = mcolors.Normalize(vmin=min(tsp_df.cost), vmax=max(tsp_df.cost))
    cmap = plt.get_cmap("YlOrBr")  # Choose a colormap

    # Create a figure and axis
    fig, ax = plt.subplots()


    # Plot the edges of the TSP solution
    tsp_solution = result_dict["best_solution"]
    for i in range(len(tsp_solution) - 1):
        id_current = tsp_solution[i]
        id_next = tsp_solution[i+1]
        ax.plot(
            [tsp_dict[id_current]['x'], tsp_dict[id_next]['x']],
            [tsp_dict[id_current]['y'], tsp_dict[id_next]['y']],
            color='black'
        )
        
    ax.plot(
        [tsp_dict[tsp_solution[-1]]['x'], tsp_dict[tsp_solution[0]]['x']],
        [tsp_dict[tsp_solution[-1]]['y'],tsp_dict[tsp_solution[0]]['y']],
        color="black"
    )
        
    # Plot nodes with colors based on costs
    ax.scatter(tsp_df.x, tsp_df.y, c=tsp_df.cost, cmap=cmap, norm=norm, s=40, zorder=10)
     
    ax.set_axis_off()
    # Add color bar
    cbar = fig.colorbar(ScalarMappable(norm=norm, cmap=cmap), ax=ax)
    cbar.set_label('Node Costs')

    # Label the nodes
    for key, val in tsp_dict.items():
        ax.text(val['x'], val['y'], key, fontsize=3, ha='center', zorder=11)

    ax.set_title(result_dict['experiment_name'])
    
    filename = out_dir / f'{result_dict['experiment_name']}.png'
    plt.savefig(filename, dpi=300, bbox_inches='tight')

            
        
    
def generate_report(all_results, script_dir, experiment_name):
    # get unique algorithm names
    algorithm_names = list(set([result_dict['experiment_name'].replace('_TSPA', '')
                        .replace('_TSPB', '') for result_dict in all_results.values()]))
    
    table_data = {}
    
    for algorithm_name in ALGORITHMS_ORDERED:
        table_data[algorithm_name] = {}
        for dataset in ['TSPA', 'TSPB']:
            result_dict = all_results[f'{algorithm_name}_{dataset}']
            table_data[algorithm_name][dataset] = {
                    'worst': result_dict['worst_cost'],
                    'avg': result_dict['average_cost'],
                    'best': result_dict['best_cost']
            }

    df = pd.DataFrame.from_dict({(alg, dataset): scores 
                             for alg, dataset_scores in table_data.items() 
                             for dataset, scores in dataset_scores.items()},
                             orient='index')
    df.index.names = ['Algorithm', 'Dataset']
    
 
    
    timestamp = datetime.now().strftime('%m_%d_%H_%M_%S')
    filename = script_dir /experiment_name / f'{timestamp}_report.md'
    
    #  writ the results and the plots to the report file
    with open(filename, 'w') as out_md:

        
        text = '\n## Algorithms\n'
        for algorithm_name in ALGORITHMS_ORDERED:
            text = text + \
f"""### {algorithm_name}

**pesudocode**

```
Code goes here
```
            
<p float="left">
<img src="/{experiment_name}/plots/{algorithm_name}_TSPA.png" width="400" />
<img src="/{experiment_name}/plots/{algorithm_name}_TSPB.png" width="400" /> 
</p>

"""
            for dataset in ['TSPA', 'TSPB']:
                solution_str = str(shift_solution(all_results[f'{algorithm_name}_{dataset}']['best_solution']))
                text = text + f'**Best solution for {dataset}:**\n\n' + solution_str + '\n\n'
        
        out_md.write(text)


        out_md.write('\n## Results\n')
        out_md.write(df.reset_index().to_markdown(index=False))
    

if __name__ =="__main__":
    parser = argparse.ArgumentParser(description='This script reads the result of a specified experiment, verifies the scores, and generates the visualisations')
    parser.add_argument('experiment_dir', help='')
    args = parser.parse_args()
    
    tspa_dict = read_instance(args.experiment_dir, "TSPA.csv")
    tspb_dict = read_instance(args.experiment_dir, "TSPB.csv")
    
    pattern = os.path.join(args.experiment_dir, '*TSPA*.txt')
    pattern2 = os.path.join(args.experiment_dir, '*TSPB*.txt')
    files_tspa = glob.glob(pattern)
    files_tspb = glob.glob(pattern2)
    all_experiments_filenames = files_tspa + files_tspb
    

    # Define the directory name
    plots_dir = script_dir / args.experiment_dir / "plots"
    if not os.path.exists(plots_dir):
        os.makedirs(plots_dir)    
    
    all_results = {}
    for experiment_filename in all_experiments_filenames:
        result_dict: dict = read_algorithm_result(experiment_filename)
        all_results[result_dict['experiment_name']] = result_dict
        tsp_dict = tspa_dict if "TSPA" in experiment_filename else tspb_dict
        
        validate_result(tsp_dict, result_dict)
        
        save_plot_experiment(plots_dir, result_dict, tsp_dict)
        
    
    # experiment_dir = script_dir / args.experiment_dir
    generate_report(all_results, script_dir, args.experiment_dir)
   
        
    
        