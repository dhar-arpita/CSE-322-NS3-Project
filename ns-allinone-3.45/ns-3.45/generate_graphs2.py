import matplotlib.pyplot as plt
import numpy as np
from scipy.interpolate import make_interp_spline


nodes = [20, 30, 35, 40, 45, 50]

bh_pdr = {
    'Normal AODV':   [98.8509, 97.9086, 94.3469, 95.645,  96.4146, 96.6233],
    'Blackhole (1)': [37.1003, 40.5343, 48.0014, 50.1924, 48.6179, 52.6694],
    'Blackhole (2)': [26.0976, 27.3209, 29.2344, 30.8049, 26.0976, 34.0108],
    'Blackhole (3)': [17.561,  25.7724, 25.6098, 25.626,  27.4255, 31.5447],
}


gh_pdr = {
    'Normal AODV':    [98.8669, 93.881,  92.068,  88.2153, 87.5722, 90.085],
    'Grayhole (1)':   [54.6176, 49.1501, 44.9858, 48.2153, 47.4788, 38.8669],
    'Grayhole (2)':   [59.7167, 55.3399, 43.1586, 48.4986, 56.6006, 62.7762],
    'Grayhole (3)':   [52.7025, 50.9915, 42.8895, 38.0737, 55.0142, 45.5524],
}

bh_styles = {
    'Normal AODV':   {'color': '#2176FF', 'marker': 'D'},
    'Blackhole (1)': {'color': '#E83535', 'marker': 's'},
    'Blackhole (2)': {'color': '#33A532', 'marker': '^'},
    'Blackhole (3)': {'color': '#9B59B6', 'marker': 'X'},
}
gh_styles = {
    'Normal AODV':    {'color': '#2176FF', 'marker': 'D'},
    'Grayhole (1)':   {'color': '#E83535', 'marker': 's'},
    'Grayhole (2)':   {'color': '#33A532', 'marker': '^'},
    'Grayhole (3)':   {'color': '#9B59B6', 'marker': 'X'},
}

# Smooth spline
def smooth(x, y):
    x, y = np.array(x, dtype=float), np.array(y, dtype=float)
    xnew = np.linspace(x.min(), x.max(), 200)
    return xnew, make_interp_spline(x, y, k=3)(xnew)

# Graph generator
def make_graph(x, pdr_dict, styles, title, filename):
    fig, ax = plt.subplots(figsize=(8, 5.5))
    for label, pdr_vals in pdr_dict.items():
        s = styles[label]
        xs, ys = smooth(x, pdr_vals)
        ax.plot(xs, ys, color=s['color'], linestyle='--', linewidth=2, label=label)
        ax.plot(x, pdr_vals, color=s['color'], marker=s['marker'],
                markersize=9, linewidth=0, markeredgecolor='white', markeredgewidth=1.2, zorder=5)
    ax.set_xlabel('Number of Nodes', fontsize=12, fontweight='bold')
    ax.set_ylabel('Packet Delivery Rate (%)', fontsize=12, fontweight='bold')
    ax.set_title(title, fontsize=13, fontweight='bold', pad=12)
    ax.legend(fontsize=10, loc='best', framealpha=0.9)
    ax.grid(True, alpha=0.25)
    ax.set_xticks(x)
    ax.set_ylim(0, 105)
    ax.spines['top'].set_visible(False)
    ax.spines['right'].set_visible(False)
    fig.tight_layout()
    fig.savefig(filename, dpi=150, bbox_inches='tight')
    plt.close(fig)
    print(f"Saved: {filename}")


make_graph(nodes, bh_pdr, bh_styles,
           'Node Count vs PDR (Black-Hole Group)',
           '/home/arpita/Desktop/Term_3-2/CSE-322-Computer-Networks-Sessional/NS3_Project/ns-allinone-3.45/ns-3.45/graphs/nodecount_pdr_blackhole.png')

make_graph(nodes, gh_pdr, gh_styles,
           'Node Count vs PDR (Gray-Hole Group)',
           '/home/arpita/Desktop/Term_3-2/CSE-322-Computer-Networks-Sessional/NS3_Project/ns-allinone-3.45/ns-3.45/graphs/nodecount_pdr_grayhole.png')
print("\nDone!")