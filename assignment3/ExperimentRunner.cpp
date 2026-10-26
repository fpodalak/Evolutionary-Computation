#include "ExperimentRunner.h"
#include <chrono>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <limits>

using namespace std;

ResultStats ExperimentRunner::run_experiment(TSPSolver& solver, const string& name, bool is_ls, bool is_steepest, bool is_2opt, bool is_greedy_init, int num_runs) {
    cout << "Running: " << name << " (" << num_runs << " runs)... " << flush;
    
    double min_c = numeric_limits<double>::max();
    double max_c = numeric_limits<double>::lowest();
    double sum_c = 0;

    double min_t = numeric_limits<double>::max();
    double max_t = numeric_limits<double>::lowest();
    double sum_t = 0;

    vector<int> best_cycle;

    auto t_exp_start = chrono::high_resolution_clock::now();

    for (int run = 0; run < num_runs; ++run) {
        auto t_start = chrono::high_resolution_clock::now();

        vector<int> init_cycle;
        if (is_greedy_init) {
            init_cycle = solver.get_greedy_cycle_solution(run % solver.n);
        } else {
            init_cycle = solver.get_random_solution();
        }

        vector<int> final_cycle;
        if (is_ls) {
            final_cycle = solver.local_search(init_cycle, is_steepest, is_2opt);
        } else {
            final_cycle = init_cycle;
        }

        auto t_end = chrono::high_resolution_clock::now();
        double time_ms = chrono::duration<double, milli>(t_end - t_start).count();

        double c = solver.evaluate(final_cycle);

        if (c < min_c) {
            min_c = c;
            best_cycle = final_cycle;
        }
        if (c > max_c) max_c = c;
        sum_c += c;

        if (time_ms < min_t) min_t = time_ms;
        if (time_ms > max_t) max_t = time_ms;
        sum_t += time_ms;
    }

    auto t_exp_end = chrono::high_resolution_clock::now();
    double total_sec = chrono::duration<double>(t_exp_end - t_exp_start).count();
    cout << "Done in " << fixed << setprecision(2) << total_sec << " s (Avg cost: " << (int)(sum_c / num_runs) << ")" << endl;

    return {name, min_c, max_c, sum_c / num_runs, min_t, max_t, sum_t / num_runs, best_cycle};
}

void ExperimentRunner::print_table(const string& header, const vector<ResultStats>& res_a, const vector<ResultStats>& res_b) {
    cout << "\n========================================================================================================\n";
    cout << header << "\n";
    cout << "========================================================================================================\n";
    cout << left << setw(38) << "Method" 
         << " | " << setw(32) << "Instance 1 (TSPA.csv)" 
         << " | " << setw(32) << "Instance 2 (TSPB.csv)" << "\n";
    cout << "--------------------------------------------------------------------------------------------------------\n";
    for (size_t i = 0; i < res_a.size(); ++i) {
        stringstream ss_a, ss_b;
        ss_a << fixed << setprecision(2) << res_a[i].avg_cost << " (" << (int)res_a[i].min_cost << " - " << (int)res_a[i].max_cost << ")";
        ss_b << fixed << setprecision(2) << res_b[i].avg_cost << " (" << (int)res_b[i].min_cost << " - " << (int)res_b[i].max_cost << ")";
        
        cout << left << setw(38) << res_a[i].name 
             << " | " << setw(32) << ss_a.str() 
             << " | " << setw(32) << ss_b.str() << "\n";
    }
    cout << "========================================================================================================\n";
}

void ExperimentRunner::print_time_table(const string& header, const vector<ResultStats>& res_a, const vector<ResultStats>& res_b) {
    cout << "\n========================================================================================================\n";
    cout << header << "\n";
    cout << "========================================================================================================\n";
    cout << left << setw(38) << "Method" 
         << " | " << setw(32) << "Instance 1 Time [ms]" 
         << " | " << setw(32) << "Instance 2 Time [ms]" << "\n";
    cout << "--------------------------------------------------------------------------------------------------------\n";
    for (size_t i = 0; i < res_a.size(); ++i) {
        stringstream ss_a, ss_b;
        ss_a << fixed << setprecision(2) << res_a[i].avg_time_ms << " (" << res_a[i].min_time_ms << " - " << res_a[i].max_time_ms << ")";
        ss_b << fixed << setprecision(2) << res_b[i].avg_time_ms << " (" << res_b[i].min_time_ms << " - " << res_b[i].max_time_ms << ")";
        
        cout << left << setw(38) << res_a[i].name 
             << " | " << setw(32) << ss_a.str() 
             << " | " << setw(32) << ss_b.str() << "\n";
    }
    cout << "========================================================================================================\n";
}