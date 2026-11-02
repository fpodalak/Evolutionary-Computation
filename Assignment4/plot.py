import csv
import math

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.colors import LinearSegmentedColormap, Normalize

BLUE_RAMP = ["#cde2fb", "#9ec5f4", "#6da7ec", "#3987e5", "#256abf", "#184f95", "#0d366b"]
INK, INK_2, GRID, MUTED = "#0b0b0b", "#52514e", "#e4e3df", "#c3c2b7"
COST_CMAP = LinearSegmentedColormap.from_list("cost", BLUE_RAMP)
INSTANCES = {"A": "TSPA.csv", "B": "TSPB.csv"}
NAMES = {
    "random": "Random",
    "nn_end": "Nearest neighbour (end only)",
    "nn_any": "Nearest neighbour (any position)",
    "greedy_cycle": "Greedy cycle",
    "nn_any_regret": "Greedy 2-regret, NN any position",
    "nn_any_weighted": "Weighted sum, NN any position",
    "greedy_cycle_regret": "Greedy 2-regret, greedy cycle",
    "greedy_cycle_weighted": "Weighted sum, greedy cycle",
    "ls_steepest_swap_random": "LS steepest, node swap, random start",
    "ls_steepest_2opt_random": "LS steepest, 2-opt, random start (baseline)",
    "ls_greedy_swap_random": "LS greedy, node swap, random start",
    "ls_greedy_2opt_random": "LS greedy, 2-opt, random start",
    "ls_steepest_swap_greedy": "LS steepest, node swap, greedy start",
    "ls_steepest_2opt_greedy": "LS steepest, 2-opt, greedy start",
    "ls_greedy_swap_greedy": "LS greedy, node swap, greedy start",
    "ls_greedy_2opt_greedy": "LS greedy, 2-opt, greedy start",
    "ls_steepest_2opt_candidates": "LS steepest, 2-opt, candidate moves (K = 10)",
}

plt.rcParams.update({
    "font.size": 10, "axes.edgecolor": GRID, "axes.labelcolor": INK_2,
    "xtick.color": INK_2, "ytick.color": INK_2, "axes.titlecolor": INK,
    "figure.facecolor": "white", "axes.facecolor": "white",
})


def read_points(path):
    with open(path) as f:
        return [tuple(map(int, line.strip().split(";"))) for line in f if line.strip()]


def objective(points, sol):
    def d(i, j):
        return int(math.floor(math.hypot(points[i][0] - points[j][0], points[i][1] - points[j][1]) + 0.5))
    return sum(points[v][2] for v in sol) + sum(d(sol[i], sol[(i + 1) % len(sol)]) for i in range(len(sol)))


def draw(ax, fig, inst, points, sol, obj):
    costs = [p[2] for p in points]
    norm = Normalize(min(costs), max(costs))
    chosen = set(sol)
    rest = [i for i in range(len(points)) if i not in chosen]
    ax.scatter([points[i][0] for i in rest], [points[i][1] for i in rest], s=8, color=MUTED, linewidths=0)
    cyc = sol + [sol[0]]
    ax.plot([points[v][0] for v in cyc], [points[v][1] for v in cyc], color=INK_2, lw=1.0, zorder=2)
    sc = ax.scatter([points[v][0] for v in sol], [points[v][1] for v in sol],
                    s=[14 + 50 * norm(costs[v]) for v in sol], c=[costs[v] for v in sol],
                    cmap=COST_CMAP, norm=norm, edgecolors="white", linewidths=0.8, zorder=3)
    cb = fig.colorbar(sc, ax=ax, fraction=0.035, pad=0.015, shrink=0.9)
    cb.set_label("node cost", fontsize=8)
    cb.outline.set_visible(False)
    cb.ax.tick_params(labelsize=7, length=0)
    ax.set_title(f"TSP{inst} · objective {obj:,}".replace(",", " "), loc="left", fontsize=10)
    ax.set_aspect("equal")
    ax.tick_params(length=0, labelsize=7)


def main():
    with open("results/summary.csv") as f:
        rows = list(csv.DictReader(f))
    points = {k: read_points(v) for k, v in INSTANCES.items()}
    methods = list(dict.fromkeys(r["method"] for r in rows))
    for m in methods:
        fig, axes = plt.subplots(1, 2, figsize=(13, 3.75))
        for ax, inst in zip(axes, INSTANCES):
            row = next(r for r in rows if r["instance"] == inst and r["method"] == m)
            with open(f"results/best_{inst}_{m}.txt") as f:
                sol = [int(x) for x in f if x.strip()]
            obj = objective(points[inst], sol)
            if obj != int(row["min"]):
                raise SystemExit(f"{inst} {m}: recomputed {obj} != summary min {row['min']}")
            draw(ax, fig, inst, points[inst], sol, obj)
        fig.suptitle(f"{row['group']} · {NAMES.get(m, m)}", x=0.01, ha="left", color=INK, fontsize=12)
        fig.tight_layout(rect=(0, 0, 1, 0.93), w_pad=1.5)
        fig.savefig(f"results/fig_{m}.png", dpi=130)
        plt.close(fig)
    print(f"wrote {len(methods)} figures")


if __name__ == "__main__":
    main()
