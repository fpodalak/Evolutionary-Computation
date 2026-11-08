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
    std::vector<std::vector<int>> cand;
    std::vector<std::vector<char>> is_cand;   // is_cand[u][v] == 1 iff v is in cand[u]

    // Set by the steepest searches after every call: moves evaluated, moves applied.
    long long last_evals = 0;
    long long last_iters = 0;

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

    // Assignment 4
    void build_candidates(int K);
    void collect_candidate_moves(const std::vector<int>& cycle, const std::vector<int>& pos, std::vector<Move>& out) const;
    std::vector<int> local_search_candidates(std::vector<int> cycle);

    // Assignment 5 (src/LMSearch.cpp): steepest search with a list of improving moves.
    // use_candidates: only moves that add a candidate edge enter the list (needs build_candidates).
    // verify: slow self-check used by the tests (compares every step with a full scan).
    std::vector<int> local_search_lm(std::vector<int> cycle, bool use_candidates = false, bool verify = false);
};

#endif
