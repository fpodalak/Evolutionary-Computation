import csv
import math

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.colors import LinearSegmentedColormap, Normalize

BLUE_RAMP = ["#cde2fb", "#9ec5f4", "#6da7ec", "#3987e5", "#256abf", "#184f95", "#0d366b"]
SERIES = {"greedy_cycle": "#2a78d6", "nn_any": "#eb6834"}
INK, INK_2, GRID, MUTED = "#0b0b0b", "#52514e", "#e4e3df", "#c3c2b7"
COST_CMAP = LinearSegmentedColormap.from_list("cost", BLUE_RAMP)
INSTANCES = {"A": "TSPA.csv", "B": "TSPB.csv"}

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


def plot_best(inst, method, points, expected):
    with open(f"results/best_{inst}_{method}.txt") as f:
        sol = [int(x) for x in f if x.strip()]
    obj = objective(points, sol)
    if obj != expected:
        raise SystemExit(f"{inst} {method}: recomputed {obj} != summary min {expected}")

    costs = [p[2] for p in points]
    norm = Normalize(min(costs), max(costs))
    chosen = set(sol)
    rest = [i for i in range(len(points)) if i not in chosen]

    fig, ax = plt.subplots(figsize=(9, 5.4))
    ax.scatter([points[i][0] for i in rest], [points[i][1] for i in rest],
               s=10, color=MUTED, linewidths=0, label="not selected")
    cyc = sol + [sol[0]]
    ax.plot([points[v][0] for v in cyc], [points[v][1] for v in cyc], color=INK_2, lw=1.2, zorder=2)
    sc = ax.scatter([points[v][0] for v in sol], [points[v][1] for v in sol],
                    s=[20 + 60 * norm(costs[v]) for v in sol], c=[costs[v] for v in sol],
                    cmap=COST_CMAP, norm=norm, edgecolors="white", linewidths=1.0, zorder=3,
                    label="selected (size ∝ cost)")
    cb = fig.colorbar(sc, ax=ax, fraction=0.035, pad=0.02, shrink=0.85)
    cb.set_label("node cost (selected nodes)")
    cb.outline.set_visible(False)
    ax.set_title(f"Instance {inst} · {method.replace('_', ' ')} · objective {obj:,}".replace(",", " "), loc="left")
    ax.set_aspect("equal")
    ax.legend(loc="upper center", bbox_to_anchor=(0.5, -0.06), ncol=2, frameon=False, fontsize=9)
    ax.tick_params(length=0)
    fig.tight_layout()
    fig.savefig(f"results/best_{inst}_{method}.png", dpi=150)
    plt.close(fig)


def plot_sweep():
    with open("results/sweep.csv") as f:
        rows = list(csv.DictReader(f))
    fig, axes = plt.subplots(1, 2, figsize=(11, 4.2))
    for ax, inst in zip(axes, INSTANCES):
        notes = []
        for base, color in SERIES.items():
            pts = sorted((float(r["w_regret"]), float(r["avg"])) for r in rows
                         if r["instance"] == inst and r["base"] == base and float(r["w_cost"]) == 1)
            xs, ys = zip(*pts)
            ax.plot(xs, ys, color=color, lw=2, marker="o", ms=7, mec="white", mew=1.5, label=base.replace("_", " "))
            best = min(pts, key=lambda p: p[1])
            ax.annotate(f"{best[1]:,.0f}".replace(",", " "), best, textcoords="offset points",
                        xytext=(0, -15 if base == "greedy_cycle" else 9), ha="center", fontsize=8, color=INK_2)
            pure = next(float(r["avg"]) for r in rows
                        if r["instance"] == inst and r["base"] == base and float(r["w_cost"]) == 0)
            notes.append(f"{base.replace('_', ' ')}: {pure:,.0f}".replace(",", " "))
        ax.set_title(f"Instance {inst}", loc="left")
        ax.set_xticks([0, 0.5, 1, 2], ["0\n(greedy)", "0.5", "1\n(equal)", "2"])
        ax.set_xlabel("w_regret  (w_cost = 1)")
        ax.set_ylabel("average objective (200 starts)")
        ax.grid(axis="y", color=GRID, lw=0.8)
        ax.set_axisbelow(True)
        ax.tick_params(length=0)
        for s in ("top", "right"):
            ax.spines[s].set_visible(False)
        ax.set_title("pure 2-regret (off scale): " + " · ".join(notes), loc="right", fontsize=8, color=INK_2)
        ax.margins(x=0.06, y=0.15)
    fig.suptitle("Weighted 2-regret: effect of the regret weight", x=0.01, ha="left", color=INK)
    handles, labels = axes[0].get_legend_handles_labels()
    fig.legend(handles, labels, loc="upper right", bbox_to_anchor=(0.985, 0.995), ncol=2, frameon=False)
    fig.tight_layout(rect=(0, 0, 1, 0.95))
    fig.savefig("results/sweep.png", dpi=150)
    plt.close(fig)


def main():
    with open("results/summary.csv") as f:
        summary = list(csv.DictReader(f))
    points = {k: read_points(v) for k, v in INSTANCES.items()}
    for r in summary:
        plot_best(r["instance"], r["method"], points[r["instance"]], int(r["min"]))
    plot_sweep()
    print(f"wrote {len(summary)} solution plots + sweep.png")


if __name__ == "__main__":
    main()
