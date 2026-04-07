import subprocess
import os
import matplotlib.pyplot as plt
import csv
from datetime import datetime

# Configuration
NS3_PATH = "./ns3"
SCRIPT_NAME = "aodv-benchmark"
OUTPUT_CSV = "blackhole_results.csv"

# Experiment Parameters
NODE_COUNTS = [20, 30, 40, 50, 60]
SPEEDS = [0, 10, 20]
BLACKHOLE_COUNTS = [0, 1, 2, 3]
FIXED_NODES_FOR_SPEED = 50
FIXED_SPEED_FOR_NODES = 0 # Static for node density test, or random if desired. User said "speed 0" first.
SIM_TIME = 200

def run_simulation(n_nodes, n_blackholes, speed, sim_time):
    """
    Runs a single simulation using the ns3 script.
    """
    cmd = [
        # Direct executable call for speed (avoid waf check)
        "./build/scratch/ns3.45-aodv-benchmark-debug",
        f"--nNodes={n_nodes}",
        f"--nBlackholes={n_blackholes}",
        f"--speed={speed}",
        f"--xSize=1000",
        f"--ySize=1500",
        f"--simTime={sim_time}",
        f"--csvOutput={OUTPUT_CSV}"
    ]
    
    print(f"Running: Nodes={n_nodes}, BH={n_blackholes}, Speed={speed}...", end=" ", flush=True)
    try:
        # Check if output file exists, if not create header
        if not os.path.exists(OUTPUT_CSV):
             with open(OUTPUT_CSV, 'w') as f:
                f.write("Nodes,Speed,Blackholes,PDR,Throughput,Delay\n")
        
        result = subprocess.run(cmd, capture_output=True, text=True, cwd=os.getcwd())
        if result.returncode != 0:
            print(f"FAILED. Error: {result.stderr}")
        else:
            print("Done.")
    except Exception as e:
        print(f"EXCEPTION: {e}")

def run_node_variation_experiment():
    print("\n--- Starting Node Variation Experiment ---")
    speed = FIXED_SPEED_FOR_NODES
    for n in NODE_COUNTS:
        for bh in BLACKHOLE_COUNTS:
            run_simulation(n, bh, speed, SIM_TIME)

def run_speed_variation_experiment():
    print("\n--- Starting Speed Variation Experiment ---")
    nodes = FIXED_NODES_FOR_SPEED
    for s in SPEEDS:
        for bh in BLACKHOLE_COUNTS:
            # Avoid repeating the overlapping case if already run? 
            # It's fine to re-run or skipped if we implement smart management.
            # For simplicity, just run.
            run_simulation(nodes, bh, s, SIM_TIME)

def plot_results():
    if not os.path.exists(OUTPUT_CSV):
        print("No results file found.")
        return

    data = []
    with open(OUTPUT_CSV, 'r') as f:
        reader = csv.DictReader(f)
        for row in reader:
            data.append({
                "Nodes": int(row["Nodes"]),
                "Speed": float(row["Speed"]),
                "Blackholes": int(row["Blackholes"]),
                "PDR": float(row["PDR"]),
                "Throughput": float(row["Throughput"]),
                "Delay": float(row["Delay"])
            })

    # Plot 1: Node Density vs Metrics (Fixed Speed)
    plot_metric_vs_variable(data, "Nodes", "PDR", "Nodes", "Packet Delivery Ratio (%)", FIXED_SPEED_FOR_NODES, "Speed")
    plot_metric_vs_variable(data, "Nodes", "Throughput", "Nodes", "Throughput (kbps)", FIXED_SPEED_FOR_NODES, "Speed")
    plot_metric_vs_variable(data, "Nodes", "Delay", "Nodes", "Average End-to-End Delay (ms)", FIXED_SPEED_FOR_NODES, "Speed")

    # Plot 2: Speed vs Metrics (Fixed Nodes)
    plot_metric_vs_variable(data, "Speed", "PDR", "Speed (m/s)", "Packet Delivery Ratio (%)", FIXED_NODES_FOR_SPEED, "Nodes")
    plot_metric_vs_variable(data, "Speed", "Throughput", "Speed (m/s)", "Throughput (kbps)", FIXED_NODES_FOR_SPEED, "Nodes")
    plot_metric_vs_variable(data, "Speed", "Delay", "Speed (m/s)", "Average End-to-End Delay (ms)", FIXED_NODES_FOR_SPEED, "Nodes")

def plot_metric_vs_variable(data, x_var, y_var, x_label, y_label, fixed_val, fixed_var_name):
    plt.figure(figsize=(10, 6))
    
    # Filter data for the fixed variable
    # We need to handle float comparison carefully for Speed
    filtered_data = [d for d in data if abs(d[fixed_var_name] - fixed_val) < 0.1]
    
    if not filtered_data:
        print(f"No data found for {fixed_var_name}={fixed_val}")
        return

    # Group by Blackholes
    bh_groups = sorted(list(set(d["Blackholes"] for d in filtered_data)))
    
    for bh in bh_groups:
        subset = [d for d in filtered_data if d["Blackholes"] == bh]
        # Sort by x_var
        subset.sort(key=lambda x: x[x_var])
        
        x_vals = [d[x_var] for d in subset]
        y_vals = [d[y_var] for d in subset]
        
        plt.plot(x_vals, y_vals, marker='o', label=f"{bh} Blackhole(s)")

    plt.xlabel(x_label)
    plt.ylabel(y_label)
    plt.title(f"{y_var} vs {x_var} ({fixed_var_name}={fixed_val})")
    plt.legend()
    plt.grid(True)
    
    filename = f"plot_{y_var}_vs_{x_var}.png"
    plt.savefig(filename)
    print(f"Saved plot: {filename}")
    plt.close()

if __name__ == "__main__":
    # Clear previous results?
    if os.path.exists(OUTPUT_CSV):
        os.remove(OUTPUT_CSV)
        
    run_node_variation_experiment()
    run_speed_variation_experiment()
    plot_results()
