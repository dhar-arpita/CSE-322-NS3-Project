import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import numpy as np
import os

# ── Load data ──────────────────────────────────────────────────────────────────
script_dir = os.path.dirname(os.path.abspath(__file__))
csv_path = os.path.join(script_dir, "result_v1.csv")
df = pd.read_csv(csv_path)

# ── Config ─────────────────────────────────────────────────────────────────────
SPEEDS = [5, 15, 25, 35]

METRICS = [
    ("pdr",             "PDR (%)",                  "Packet Delivery Ratio",       (40, 100)),
    ("throughput",      "Throughput (bps)",          "Throughput",                  (8, 22)),
    ("routingOverhead", "Routing Overhead (bytes)",  "Routing Overhead",            (0, 120000)),
    ("nrl",             "NRL",                       "Normalised Routing Load",     (0, 90)),
]

ATTACK_TYPES = [
    ("grayhole",  "Grayhole Attack"),
    ("smartgray", "Smart Grayhole Attack"),
]

# 5 lines per subplot
LINE_SPECS = [
    # (label,            attack_filter,  defense_filter,  linestyle, marker, color)
    ("Normal AODV",      "none",         "none",          "-",       "o",    "#28F321"),
    ("Attack (No Def.)", None,           "none",          "--",      "s",    "#F44336"),
    ("Attack + IDS",     None,           "ids",           "-.",      "^",    "#FF9800"),
    ("Attack + MBDP",    None,           "mbdp",          ":",       "D",    "#276EB0"),
    ("Attack + Trust",   None,           "trust",         "-",       "P",    "#904CAF"),
]

# ── Helpers ────────────────────────────────────────────────────────────────────
def get_series(data, attack_type, attack_val, defense_val, n_malicious):
    """Return mean metric values across speeds for a given config."""
    if attack_val == "none":
        mask = (data["attack"] == "none") & (data["defense"] == "none") & (data["nMalicious"] == 0)
    else:
        mask = (
            (data["attack"] == attack_val)
            & (data["defense"] == defense_val)
            & (data["nMalicious"] == n_malicious)
        )
    sub = data[mask].sort_values("speed")
    return sub["speed"].tolist(), sub  # return sub so caller picks metric


def plot_graph(attack_type, attack_label, metric_col, metric_label, metric_title, ylim):
    fig, axes = plt.subplots(1, 2, figsize=(13, 5), sharey=True)
    fig.suptitle(
        f"{metric_title} — {attack_label}",
        fontsize=14, fontweight="bold", y=1.02
    )

    for ax_idx, n_mal in enumerate([1, 2]):
        ax = axes[ax_idx]
        ax.set_title(f"{n_mal} Malicious Node{'s' if n_mal > 1 else ''}", fontsize=12)

        for label, atk_filter, def_filter, ls, mk, col in LINE_SPECS:
            actual_atk = "none" if atk_filter == "none" else attack_type
            actual_def = def_filter if def_filter is not None else "none"
            actual_mal = 0 if atk_filter == "none" else n_mal

            if atk_filter == "none":
                mask = (
                    (df["attack"] == "none")
                    & (df["defense"] == "none")
                    & (df["nMalicious"] == 0)
                )
            else:
                mask = (
                    (df["attack"] == attack_type)
                    & (df["defense"] == actual_def)
                    & (df["nMalicious"] == n_mal)
                )

            sub = df[mask].sort_values("speed")
            if sub.empty:
                continue

            ax.plot(
                sub["speed"], sub[metric_col],
                linestyle=ls, marker=mk, color=col,
                linewidth=2, markersize=7, label=label
            )

        ax.set_xlabel("Node Speed (m/s)", fontsize=11)
        if ax_idx == 0:
            ax.set_ylabel(metric_label, fontsize=11)
        ax.set_xticks(SPEEDS)
        ax.set_ylim(ylim)
        ax.grid(True, linestyle="--", alpha=0.4)
        ax.tick_params(axis="both", labelsize=10)

        if ax_idx == 1:
            ax.legend(
                loc="best", fontsize=9,
                framealpha=0.85, edgecolor="#cccccc"
            )

    plt.tight_layout()
    safe_metric = metric_col.lower()
    safe_attack = attack_type.lower()
    out_path = os.path.join(script_dir, f"graph_{safe_attack}_{safe_metric}.png")
    plt.savefig(out_path, dpi=150, bbox_inches="tight")
    plt.close()
    print(f"  Saved → {out_path}")


# ── Generate all 8 graphs ──────────────────────────────────────────────────────
print("Generating graphs …")
for attack_type, attack_label in ATTACK_TYPES:
    for metric_col, metric_label, metric_title, ylim in METRICS:
        plot_graph(attack_type, attack_label, metric_col, metric_label, metric_title, ylim)

print("\nDone! 8 PNG files written.")