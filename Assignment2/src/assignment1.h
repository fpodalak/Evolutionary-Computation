#pragma once
#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>

struct ExperimentResult {
    double min_val;
    double max_val;
    double avg_val;
    std::vector<int> best_cycle;
};

constexpr int RANDOM_TRIALS = 200;

inline int evaluate_cycle(const std::vector<int>& cycle, const std::vector<std::vector<int>>& dist_matrix, const std::vector<int>& costs) {
    int k = cycle.size();
    int total_cost = 0;
    for (int i = 0; i < k; ++i) {
        total_cost += dist_matrix[cycle[i]][cycle[(i + 1) % k]];
        total_cost += costs[cycle[i]];
    }
    return total_cost;
}

inline ExperimentResult solve_random(int n, int k, const std::vector<std::vector<int>>& dist_matrix, const std::vector<int>& costs) {
    double min_val = std::numeric_limits<double>::infinity();
    double max_val = -1.0;
    double sum_val = 0.0;
    std::vector<int> best_cycle;
    std::vector<int> candidates(n);
    std::iota(candidates.begin(), candidates.end(), 0);
    std::mt19937 generator(std::random_device{}());

    for (int trial = 0; trial < RANDOM_TRIALS; ++trial) {
        std::shuffle(candidates.begin(), candidates.end(), generator);
        std::vector<int> cycle(candidates.begin(), candidates.begin() + k);
        int score = evaluate_cycle(cycle, dist_matrix, costs);
        sum_val += score;
        if (score < min_val) {
            min_val = score;
            best_cycle = cycle;
        }
        if (score > max_val) {
            max_val = score;
        }
    }
    return {min_val, max_val, sum_val / RANDOM_TRIALS, best_cycle};
}

inline ExperimentResult solve_nn_end(int n, int k, const std::vector<std::vector<int>>& dist_matrix, const std::vector<int>& costs) {
    double min_val = std::numeric_limits<double>::infinity();
    double max_val = -1.0;
    double sum_val = 0.0;
    std::vector<int> best_cycle;

    for (int start = 0; start < n; ++start) {
        std::vector<bool> visited(n, false);
        visited[start] = true;
        std::vector<int> path;
        path.reserve(k);
        path.push_back(start);

        while (static_cast<int>(path.size()) < k) {
            int best_node = -1;
            int best_increase = std::numeric_limits<int>::max();
            int last = path.back();

            for (int v = 0; v < n; ++v) {
                if (visited[v]) continue;
                int increase = dist_matrix[last][v] + costs[v];
                if (increase < best_increase) {
                    best_increase = increase;
                    best_node = v;
                }
            }

            if (best_node == -1) {
                throw std::runtime_error("NN-End: no feasible node found");
            }

            visited[best_node] = true;
            path.push_back(best_node);
        }

        int score = evaluate_cycle(path, dist_matrix, costs);
        sum_val += score;
        if (score < min_val) {
            min_val = score;
            best_cycle = path;
        }
        if (score > max_val) {
            max_val = score;
        }
    }
    return {min_val, max_val, sum_val / n, best_cycle};
}

inline ExperimentResult solve_nn_any(int n, int k, const std::vector<std::vector<int>>& dist_matrix, const std::vector<int>& costs) {
    double min_val = std::numeric_limits<double>::infinity();
    double max_val = -1.0;
    double sum_val = 0.0;
    std::vector<int> best_cycle;

    for (int start = 0; start < n; ++start) {
        std::vector<bool> visited(n, false);
        visited[start] = true;
        std::vector<int> path;
        path.reserve(k);
        path.push_back(start);

        while (static_cast<int>(path.size()) < k) {
            int cur_m = path.size();
            int best_node = -1;
            int best_pos = -1;
            int best_increase = std::numeric_limits<int>::max();

            for (int v = 0; v < n; ++v) {
                if (visited[v]) continue;

                int d0 = dist_matrix[v][path[0]] + costs[v];
                if (d0 < best_increase) {
                    best_increase = d0;
                    best_node = v;
                    best_pos = 0;
                }

                int dm = dist_matrix[path.back()][v] + costs[v];
                if (dm < best_increase) {
                    best_increase = dm;
                    best_node = v;
                    best_pos = cur_m;
                }

                for (int i = 0; i < cur_m - 1; ++i) {
                    int d_in = dist_matrix[path[i]][v] + dist_matrix[v][path[i+1]] - dist_matrix[path[i]][path[i+1]] + costs[v];
                    if (d_in < best_increase) {
                        best_increase = d_in;
                        best_node = v;
                        best_pos = i + 1;
                    }
                }
            }

            if (best_node == -1 || best_pos == -1) {
                throw std::runtime_error("NN-Any: no feasible insertion found");
            }

            visited[best_node] = true;
            path.insert(path.begin() + best_pos, best_node);
        }

        int score = evaluate_cycle(path, dist_matrix, costs);
        sum_val += score;
        if (score < min_val) {
            min_val = score;
            best_cycle = path;
        }
        if (score > max_val) {
            max_val = score;
        }
    }
    return {min_val, max_val, sum_val / n, best_cycle};
}

inline ExperimentResult solve_greedy_cycle(int n, int k, const std::vector<std::vector<int>>& dist_matrix, const std::vector<int>& costs) {
    double min_val = std::numeric_limits<double>::infinity();
    double max_val = -1.0;
    double sum_val = 0;
    std::vector<int> best_cycle;

    for (int start = 0; start < n; ++start) {
        std::vector<bool> visited(n, false);
        visited[start] = true;
        std::vector<int> cycle;
        cycle.reserve(k);
        cycle.push_back(start);

        while (static_cast<int>(cycle.size()) < k) {
            int cur_m = cycle.size();
            int best_node = -1;
            int best_pos = -1;
            int best_increase = std::numeric_limits<int>::max();

            for (int v = 0; v < n; ++v) {
                if (visited[v]) continue;

                for (int i = 0; i < cur_m; ++i) {
                    int u1 = cycle[i];
                    int u2 = cycle[(i+1) % cur_m];
                    int increase = dist_matrix[u1][v] + dist_matrix[v][u2] - dist_matrix[u1][u2] + costs[v];
                    if (increase < best_increase) {
                        best_increase = increase;
                        best_node = v;
                        best_pos = i + 1;
                    }
                }
            }

            if (best_node == -1 || best_pos == -1) {
                throw std::runtime_error("Greedy-Cycle: no feasible insertion found");
            }

            visited[best_node] = true;
            cycle.insert(cycle.begin() + best_pos, best_node);
        }
        
        int score = evaluate_cycle(cycle, dist_matrix, costs);
        sum_val += score;
        if (score < min_val) {
            min_val = score;
            best_cycle = cycle;
        }
        if (score > max_val) {
            max_val = score;
        }
    }
    return {min_val, max_val, sum_val / n, best_cycle};
}
