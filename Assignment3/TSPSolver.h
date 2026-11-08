#ifndef TSPSOLVER_H
#define TSPSOLVER_H

#include "Types.h"
#include <vector>
#include <string>
#include <random>

class TSPSolver {
public:
    int n;
    int k;
    std::vector<Node> nodes;
    std::vector<std::vector<int>> D;
    std::vector<int> costs;
    std::mt19937 rng;

    explicit TSPSolver(int seed = 42);

    bool loadInstance(const std::string& filename);
    int evaluate(const std::vector<int>& cycle) const;

    std::vector<int> get_random_solution();
    std::vector<int> get_nn_end_solution(int start_node);
    std::vector<int> get_nn_any_solution(int start_node);
    std::vector<int> get_greedy_cycle_solution(int start_node);

    int calc_delta_inter(const std::vector<int>& cycle, int i, int v) const;
    int calc_delta_node_swap(const std::vector<int>& cycle, int i, int j) const;
    int calc_delta_2opt(const std::vector<int>& cycle, int i, int j) const;

    void apply_move(std::vector<int>& cycle, const Move& m) const;
    std::vector<int> local_search(std::vector<int> cycle, bool is_steepest, bool is_2opt);
};

#endif