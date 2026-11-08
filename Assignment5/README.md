# Assignment 5 — Move evaluations from previous iterations

Steepest local search using 2-opt and selected/unselected node exchange. It keeps a list of improving moves and reuses their evaluations between iterations, evaluating only moves affected by newly created edges or changed node selections. An optional variant combines this approach with candidate moves (K = 10).

Put `TSPA.csv` and `TSPB.csv` in this folder, then run:

'''
make run
python plot.py
'''

`make run` writes `results/summary.csv` (objective and runtime), `results/moves.csv` (evaluated and applied moves), and the best solutions to `results/best_*.txt`. `plot.py` creates one figure per method in `results/fig_*.png` (needs matplotlib).
