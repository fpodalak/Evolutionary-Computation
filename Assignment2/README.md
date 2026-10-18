# Assignment 2 — Greedy regret heuristics

Greedy 2-regret and weighted 2-regret (2-regret + best change of the objective) built on nearest neighbour (any position) and greedy cycle. The Assignment 1 methods are reused from `src/assignment1.h` for comparison.

Put `TSPA.csv` and `TSPB.csv` in this folder, then run the tests, the experiments and the plots:

```
make test
make run
python3 plot.py
```

`make run` writes `results/summary.csv`, `results/sweep.csv` and the best solution of each method to `results/best_*.txt`. `plot.py` (needs matplotlib) draws `results/*.png`.
