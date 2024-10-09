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
     

    # Add color bar
    cbar = fig.colorbar(ScalarMappable(norm=norm, cmap=cmap), ax=ax)
    cbar.set_label('Node Costs')

    # Label the nodes
    for key, val in tsp_dict.items():
        ax.text(val['x'], val['y'], key, fontsize=3, ha='center', zorder=11)

    filename = out_dir / f'{result_dict['experiment_name']}.png'
    plt.savefig(filename, dpi=300, bbox_inches='tight')

            
        
    
    
        


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
    
    # Generate a timestamp string
    timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')  # Format: YYYYMMDD_HHMMSS

    # Define the directory name
    plots_dir = script_dir / args.experiment_dir / "plots" / timestamp 
    if not os.path.exists(plots_dir):
        os.makedirs(plots_dir)    
    
    for experiment_filename in all_experiments_filenames:
        result_dict: dict = read_algorithm_result(experiment_filename)
        tsp_dict = tspa_dict if "TSPA" in experiment_filename else tspb_dict
        
        validate_result(tsp_dict, result_dict)
        
        save_plot_experiment(plots_dir, result_dict, tsp_dict)
        
        
        
    
        