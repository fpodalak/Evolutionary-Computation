#include "TSPSolver.h"
#include "ExperimentRunner.h"
#include <iostream>
#include <fstream>
#include <filesystem>

using namespace std;

static void save_solutions(const string& tag, const vector<ResultStats>& results) {
    filesystem::create_directories("solutions");
    for (const auto& r : results) {
        string fname = r.name;
        for (char& c : fname) {
            if (!isalnum((unsigned char)c)) c = '_';
        }
        ofstream out("solutions/" + tag + "_" + fname + ".txt");
        for (size_t i = 0; i < r.best_cycle.size(); ++i) {
            out << r.best_cycle[i] << (i + 1 < r.best_cycle.size() ? " " : "\n");
        }
    }
}

int main(int argc, char** argv) {
    string file_a = "TSPA.csv";
    string file_b = "TSPB.csv";
    if (argc > 1) file_a = argv[1];
    if (argc > 2) file_b = argv[2];

    TSPSolver solver_a(42);
    TSPSolver solver_b(42);

    cout << "Loading problem instances..." << endl;
    if (!solver_a.loadInstance(file_a)) {
        cerr << "Error loading " << file_a << endl;
        return 1;
    }
    if (!solver_b.loadInstance(file_b)) {
        cerr << "Error loading " << file_b << endl;
        return 1;
    }

    vector<ResultStats> res_a, res_b;

    cout << "\n=== RUNNING BENCHMARKS FOR INSTANCE 1 (" << file_a << ") ===" << endl;
    res_a.push_back(ExperimentRunner::run_experiment(solver_a, "Random Solution", false, false, false, false));
    res_a.push_back(ExperimentRunner::run_experiment(solver_a, "Greedy Cycle Baseline", false, false, false, true));
    res_a.push_back(ExperimentRunner::run_experiment(solver_a, "LS Steepest + Nodes (Random Start)", true, true, false, false));
    res_a.push_back(ExperimentRunner::run_experiment(solver_a, "LS Steepest + Edges (Random Start)", true, true, true, false));
    res_a.push_back(ExperimentRunner::run_experiment(solver_a, "LS Greedy + Nodes (Random Start)", true, false, false, false));
    res_a.push_back(ExperimentRunner::run_experiment(solver_a, "LS Greedy + Edges (Random Start)", true, false, true, false));
    res_a.push_back(ExperimentRunner::run_experiment(solver_a, "LS Steepest + Nodes (Greedy Start)", true, true, false, true));
    res_a.push_back(ExperimentRunner::run_experiment(solver_a, "LS Steepest + Edges (Greedy Start)", true, true, true, true));
    res_a.push_back(ExperimentRunner::run_experiment(solver_a, "LS Greedy + Nodes (Greedy Start)", true, false, false, true));
    res_a.push_back(ExperimentRunner::run_experiment(solver_a, "LS Greedy + Edges (Greedy Start)", true, false, true, true));

    cout << "\n=== RUNNING BENCHMARKS FOR INSTANCE 2 (" << file_b << ") ===" << endl;
    res_b.push_back(ExperimentRunner::run_experiment(solver_b, "Random Solution", false, false, false, false));
    res_b.push_back(ExperimentRunner::run_experiment(solver_b, "Greedy Cycle Baseline", false, false, false, true));
    res_b.push_back(ExperimentRunner::run_experiment(solver_b, "LS Steepest + Nodes (Random Start)", true, true, false, false));
    res_b.push_back(ExperimentRunner::run_experiment(solver_b, "LS Steepest + Edges (Random Start)", true, true, true, false));
    res_b.push_back(ExperimentRunner::run_experiment(solver_b, "LS Greedy + Nodes (Random Start)", true, false, false, false));
    res_b.push_back(ExperimentRunner::run_experiment(solver_b, "LS Greedy + Edges (Random Start)", true, false, true, false));
    res_b.push_back(ExperimentRunner::run_experiment(solver_b, "LS Steepest + Nodes (Greedy Start)", true, true, false, true));
    res_b.push_back(ExperimentRunner::run_experiment(solver_b, "LS Steepest + Edges (Greedy Start)", true, true, true, true));
    res_b.push_back(ExperimentRunner::run_experiment(solver_b, "LS Greedy + Nodes (Greedy Start)", true, false, false, true));
    res_b.push_back(ExperimentRunner::run_experiment(solver_b, "LS Greedy + Edges (Greedy Start)", true, false, true, true));

    save_solutions("A", res_a);
    save_solutions("B", res_b);

    ExperimentRunner::print_table("OBJECTIVE FUNCTION VALUES (AVG (MIN - MAX))", res_a, res_b);
    ExperimentRunner::print_time_table("RUNNING TIMES IN MILLISECONDS (AVG (MIN - MAX))", res_a, res_b);

    return 0;
}