#!/usr/bin/env python3


import csv
import matplotlib.pyplot as plt
import numpy as np
import os
import sys

def read_results(filename):
    """Read simulation results from CSV file"""
    nodes_baseline = []
    pdr_baseline = []
    throughput_baseline = []
    delay_baseline = []
    
    nodes_attack = []
    pdr_attack = []
    throughput_attack = []
    delay_attack = []
    
    if not os.path.exists(filename):
        print(f"Error: Results file '{filename}' not found!")
        return None
    
    with open(filename, 'r') as f:
        reader = csv.DictReader(f)
        for row in reader:
            nodes = int(row['Nodes'])
            blackhole = int(row['BlackHoleEnabled'])
            pdr = float(row['PDR(%)'])
            throughput = float(row['Throughput(kbps)'])
            delay = float(row['AvgDelay(ms)'])
            
            if blackhole == 0:  # Baseline
                nodes_baseline.append(nodes)
                pdr_baseline.append(pdr)
                throughput_baseline.append(throughput)
                delay_baseline.append(delay)
            else:  # Black Hole attack
                nodes_attack.append(nodes)
                pdr_attack.append(pdr)
                throughput_attack.append(throughput)
                delay_attack.append(delay)
    
    return {
        'baseline': {
            'nodes': nodes_baseline,
            'pdr': pdr_baseline,
            'throughput': throughput_baseline,
            'delay': delay_baseline
        },
        'attack': {
            'nodes': nodes_attack,
            'pdr': pdr_attack,
            'throughput': throughput_attack,
            'delay': delay_attack
        }
    }

def plot_comparison(data, output_dir='blackhole_results'):
    """Generate comparison plots"""
    
    # Create output directory if it doesn't exist
    os.makedirs(output_dir, exist_ok=True)
    
    # Set up plot style
    plt.style.use('seaborn-v0_8-darkgrid')
    fig, axes = plt.subplots(1, 3, figsize=(18, 5))
    
    baseline = data['baseline']
    attack = data['attack']
    
    # Plot 1: PDR vs. Number of Nodes
    axes[0].plot(baseline['nodes'], baseline['pdr'], 'b-o', linewidth=2, 
                 markersize=8, label='Without Black Hole')
    axes[0].plot(attack['nodes'], attack['pdr'], 'r-s', linewidth=2, 
                 markersize=8, label='With Black Hole')
    axes[0].set_xlabel('Number of Nodes', fontsize=12, fontweight='bold')
    axes[0].set_ylabel('Packet Delivery Ratio (%)', fontsize=12, fontweight='bold')
    axes[0].set_title('PDR vs. Number of Nodes', fontsize=14, fontweight='bold')
    axes[0].legend(fontsize=10)
    axes[0].grid(True, alpha=0.3)
    axes[0].set_ylim([0, 105])
    
    # Plot 2: Throughput vs. Number of Nodes
    axes[1].plot(baseline['nodes'], baseline['throughput'], 'b-o', linewidth=2, 
                 markersize=8, label='Without Black Hole')
    axes[1].plot(attack['nodes'], attack['throughput'], 'r-s', linewidth=2, 
                 markersize=8, label='With Black Hole')
    axes[1].set_xlabel('Number of Nodes', fontsize=12, fontweight='bold')
    axes[1].set_ylabel('Average Throughput (kbps)', fontsize=12, fontweight='bold')
    axes[1].set_title('Throughput vs. Number of Nodes', fontsize=14, fontweight='bold')
    axes[1].legend(fontsize=10)
    axes[1].grid(True, alpha=0.3)
    
    # Plot 3: End-to-End Delay vs. Number of Nodes
    axes[2].plot(baseline['nodes'], baseline['delay'], 'b-o', linewidth=2, 
                 markersize=8, label='Without Black Hole')
    axes[2].plot(attack['nodes'], attack['delay'], 'r-s', linewidth=2, 
                 markersize=8, label='With Black Hole')
    axes[2].set_xlabel('Number of Nodes', fontsize=12, fontweight='bold')
    axes[2].set_ylabel('End-to-End Delay (ms)', fontsize=12, fontweight='bold')
    axes[2].set_title('E2E Delay vs. Number of Nodes', fontsize=14, fontweight='bold')
    axes[2].legend(fontsize=10)
    axes[2].grid(True, alpha=0.3)
    
    plt.tight_layout()
    
    # Save figure
    output_file = os.path.join(output_dir, 'blackhole_comparison.png')
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"✓ Comparison plots saved to: {output_file}")
    
    # Show plot
    plt.show()

def print_comparison_table(data):
    """Print comparison table"""
    print("\n" + "="*80)
    print("PERFORMANCE COMPARISON: AODV vs. AODV with Black Hole Attack")
    print("="*80)
    
    baseline = data['baseline']
    attack = data['attack']
    
    print(f"\n{'Nodes':<10} {'Metric':<25} {'Baseline':<15} {'Black Hole':<15} {'Change':<15}")
    print("-"*80)
    
    for i in range(len(baseline['nodes'])):
        nodes = baseline['nodes'][i]
        
        # PDR
        pdr_base = baseline['pdr'][i]
        pdr_att = attack['pdr'][i] if i < len(attack['pdr']) else 0
        pdr_change = ((pdr_att - pdr_base) / pdr_base * 100) if pdr_base > 0 else 0
        
        print(f"{nodes:<10} {'PDR (%)':<25} {pdr_base:<15.2f} {pdr_att:<15.2f} {pdr_change:+.2f}%")
        
        # Throughput
        thr_base = baseline['throughput'][i]
        thr_att = attack['throughput'][i] if i < len(attack['throughput']) else 0
        thr_change = ((thr_att - thr_base) / thr_base * 100) if thr_base > 0 else 0
        
        print(f"{'':<10} {'Throughput (kbps)':<25} {thr_base:<15.2f} {thr_att:<15.2f} {thr_change:+.2f}%")
        
        # Delay
        del_base = baseline['delay'][i]
        del_att = attack['delay'][i] if i < len(attack['delay']) else 0
        del_change = ((del_att - del_base) / del_base * 100) if del_base > 0 else 0
        
        print(f"{'':<10} {'E2E Delay (ms)':<25} {del_base:<15.2f} {del_att:<15.2f} {del_change:+.2f}%")
        print("-"*80)
    
    print("\n")

def main():
    """Main function"""
    results_file = 'blackhole_results/summary_results.csv'
    
    if len(sys.argv) > 1:
        results_file = sys.argv[1]
    
    print("\n" + "="*80)
    print("Black Hole Attack Results Analysis")
    print("="*80)
    
    # Read results
    print(f"\nReading results from: {results_file}")
    data = read_results(results_file)
    
    if data is None:
        print("Failed to read results. Exiting.")
        return 1
    
    # Print comparison table
    print_comparison_table(data)
    
    # Generate plots
    print("Generating comparison plots...")
    plot_comparison(data)
    
    print("\n" + "="*80)
    print("Analysis completed successfully!")
    print("="*80 + "\n")
    
    return 0

if __name__ == '__main__':
    sys.exit(main())
