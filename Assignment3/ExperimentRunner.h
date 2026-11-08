#ifndef EXPERIMENT_RUNNER_H
#define EXPERIMENT_RUNNER_H

#include "TSPSolver.h"
#include "Types.h"
#include <string>
#include <vector>

class ExperimentRunner {
public:
    static ResultStats run_experiment(TSPSolver& solver, const std::string& name, bool is_ls, bool is_steepest, bool is_2opt, bool is_greedy_init, int num_runs = 200);
    static void print_table(const std::string& header, const std::vector<ResultStats>& res_a, const std::vector<ResultStats>& res_b);
    static void print_time_table(const std::string& header, const std::vector<ResultStats>& res_a, const std::vector<ResultStats>& res_b);
};

#endif