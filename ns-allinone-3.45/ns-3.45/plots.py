#!/usr/bin/env python3
"""
Graph Generator - Reads CSVs and generates 2x2 combined graphs.

Usage:
  python3 plot_graphs.py wifi       (reads results_wifi_*.csv, outputs graphs_wifi/)
  python3 plot_graphs.py zigbee     (reads results_zigbee_*.csv, outputs graphs_zigbee/)
"""

import sys
import os
import pandas as pd
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

# ===== Config =====
prefix = sys.argv[1] if len(sys.argv) > 1 else "wifi"
net_label = "802.11" if prefix == "wifi" else "802.15.4"
out_dir = f"graphs_{prefix}"
os.makedirs(out_dir, exist_ok=True)

GROUP_A = {
    'name': 'Grayhole Attack Group', 'tag': 'grayhole',
    'scenarios': {
        'normal':         {'label': 'Normal AODV',       'color': '#27ae60', 'marker': 'o', 'ls': '-',  'lw': 2.2},
        'grayhole_none':  {'label': 'Grayhole (No Def)', 'color': '#e74c3c', 'marker': 's', 'ls': '--', 'lw': 2.0},
        'grayhole_ids':   {'label': 'Grayhole + IDS',    'color': '#f39c12', 'marker': 'D', 'ls': '--', 'lw': 1.8},
        'grayhole_mbdp':  {'label': 'Grayhole + MBDP',   'color': '#2980b9', 'marker': '^', 'ls': '-', 'lw': 1.8},
        'grayhole_trust': {'label': 'Grayhole + Trust',  'color': '#8e44ad', 'marker': 'v', 'ls': '-',  'lw': 2.2},
    }
}

GROUP_B = {
    'name': 'Smart Grayhole Attack Group', 'tag': 'smartgray',
    'scenarios': {
        'normal':          {'label': 'Normal AODV',       'color': '#27ae60', 'marker': 'o', 'ls': '-',  'lw': 2.2},
        'smartgray_none':  {'label': 'Smart GH (No Def)', 'color': '#e74c3c', 'marker': 's', 'ls': '--', 'lw': 2.0},
        'smartgray_ids':   {'label': 'Smart GH + IDS',    'color': '#f39c12', 'marker': 'D', 'ls': '--', 'lw': 1.5},
        'smartgray_mbdp':  {'label': 'Smart GH + MBDP',   'color': '#2980b9', 'marker': '^', 'ls': '-', 'lw': 1.5},
        'smartgray_trust': {'label': 'Smart GH + Trust',  'color': '#8e44ad', 'marker': 'v', 'ls': '-',  'lw': 2.2},
    }
}

METRICS = {
    'throughput': {'col': 'throughput', 'ylabel': 'Throughput (Kbps)',       'title': 'Network Throughput'},
    'delay':      {'col': 'delay',      'ylabel': 'End-to-End Delay (ms)',  'title': 'End-to-End Delay'},
    'pdr':        {'col': 'pdr',        'ylabel': 'PDR (%)',                'title': 'Packet Delivery Ratio'},
    'dropRatio':  {'col': 'dropRatio',  'ylabel': 'Packet Drop Ratio (%)', 'title': 'Packet Drop Ratio'},
    'energy':     {'col': 'energy',     'ylabel': 'Energy Consumed (J)',    'title': 'Energy Consumption'},
}

EXPS = [
    {'file': f'results_{prefix}_nodes.csv', 'x_col': 'nNodes',   'xlabel': 'Number of Nodes',            'title': 'Varying Nodes'},
    {'file': f'results_{prefix}_flows.csv', 'x_col': 'nFlows',   'xlabel': 'Number of Flows',            'title': 'Varying Flows'},
    {'file': f'results_{prefix}_pps.csv',   'x_col': 'pps',      'xlabel': 'Packets Per Second',         'title': 'Varying PPS'},
    {'file': f'results_{prefix}_area.csv',  'x_col': 'areaMult', 'xlabel': 'Coverage Area (× Tx Range)', 'title': 'Varying Coverage Area'},
]

# ===== Generate =====
count = 0
for group in [GROUP_A, GROUP_B]:
    for mk, met in METRICS.items():
        fig, axes = plt.subplots(2, 2, figsize=(18, 13))
        fig.suptitle(f'{net_label} Static — {group["name"]} — {met["title"]}', fontsize=18, fontweight='bold', y=0.98)

        for idx, exp in enumerate(EXPS):
            ax = axes[idx // 2][idx % 2]

            if not os.path.exists(exp['file']):
                ax.text(0.5, 0.5, f'{exp["file"]}\nnot found', ha='center', va='center')
                continue

            df = pd.read_csv(exp['file'])

            for sc, style in group['scenarios'].items():
                sub = df[df['scenario'] == sc].sort_values(exp['x_col'])
                if sub.empty:
                    continue
                ax.plot(sub[exp['x_col']], sub[met['col']],
                        label=style['label'], color=style['color'],
                        marker=style['marker'], linestyle=style['ls'],
                        linewidth=style['lw'], markersize=7)

            ax.set_xlabel(exp['xlabel'], fontsize=12, fontweight='bold')
            ax.set_ylabel(met['ylabel'], fontsize=12, fontweight='bold')
            ax.set_title(exp['title'], fontsize=14, fontweight='bold')
            ax.grid(True, alpha=0.25, linestyle='--')
            ax.tick_params(labelsize=10)
            if idx == 0:
                ax.legend(fontsize=8.5, loc='best', framealpha=0.9)

        plt.tight_layout(rect=[0, 0, 1, 0.95])
        fname = f'{out_dir}/{prefix}_{met["col"]}_{group["tag"]}.png'
        fig.savefig(fname, dpi=150, bbox_inches='tight')
        plt.close(fig)
        count += 1
        print(f'[{count}/10] {fname}')

print(f'\nDone! {count} graphs saved in {out_dir}/')