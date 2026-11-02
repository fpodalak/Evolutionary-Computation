# Assignment 4 — Candidate moves in local search

Steepest local search (2-opt + selected/unselected node exchange) that evaluates only candidate moves: moves that add at least one edge to one of a node's K nearest neighbours (edge length + node cost). The program also runs every method from Assignments 1–3 with per-run timing for comparison, and a sweep over K = 5, 10, 15, 20.

Put `TSPA.csv` and `TSPB.csv` in this folder, then run:

```
make run
python3 plot.py
```

`make run` writes `results/summary.csv` (objective and time per method), `results/candidates.csv` (K sweep), `results/candidate_build.csv` and the best solution of each method to `results/best_*.txt`. `plot.py` (needs matplotlib) draws one figure per method to `results/fig_*.png`.
