import pandas as pd
import matplotlib.pyplot as plt
import os
import csv

OUTPUT_CSV = "blackhole_results.csv"
FIXED_NODES_FOR_SPEED = 50
FIXED_SPEED_FOR_NODES = 0

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

    
    plot_metric_vs_variable(data, "Speed", "PDR", "Speed (m/s)", "Packet Delivery Ratio (%)", FIXED_NODES_FOR_SPEED, "Nodes")


def plot_metric_vs_variable(data, x_var, y_var, x_label, y_label, fixed_val, fixed_var_name):
    plt.figure(figsize=(10, 6))
    
    # Filter data for the fixed variable
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
    plot_results()
