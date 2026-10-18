#include "assignment1.h"
#include "construct.h"
#include "instance.h"
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>

struct Stats {
    long long min = 0, max = 0;
    double avg = 0;
    std::vector<int> best;
};

Stats runAll(const Instance& inst, Base base, double wRegret, double wCost) {
    const int K = requiredCount(inst.n);
    Stats st;
    long long sum = 0;
    for (int s = 0; s < inst.n; ++s) {
        std::vector<int> sol = construct(inst, s, K, base, wRegret, wCost);
        if (!isValid(inst, sol, K)) {
            std::fprintf(stderr, "invalid solution from start %d\n", s);
            std::exit(1);
        }
        long long v = evaluate(inst, sol);
        sum += v;
        if (s == 0 || v < st.min) { st.min = v; st.best = sol; }
        if (s == 0 || v > st.max) st.max = v;
    }
    st.avg = static_cast<double>(sum) / inst.n;
    return st;
}

Stats fromAssignment1(const Instance& inst, const ExperimentResult& r) {
    if (!isValid(inst, r.best_cycle, requiredCount(inst.n))) {
        std::fprintf(stderr, "invalid Assignment 1 solution\n");
        std::exit(1);
    }
    return {static_cast<long long>(r.min_val), static_cast<long long>(r.max_val), r.avg_val, r.best_cycle};
}

struct Method {
    std::string name;
    Base base;
    double wRegret, wCost;
};

const char* baseName(Base base) { return base == Base::Cycle ? "greedy_cycle" : "nn_any"; }

int main() {
    const std::vector<std::pair<std::string, std::string>> instances = {{"A", "TSPA.csv"}, {"B", "TSPB.csv"}};
    const std::vector<Method> methods = {
        {"nn_any_regret", Base::Path, 1, 0},
        {"nn_any_weighted", Base::Path, 1, 1},
        {"greedy_cycle_regret", Base::Cycle, 1, 0},
        {"greedy_cycle_weighted", Base::Cycle, 1, 1},
    };
    const std::vector<std::pair<double, double>> sweepWeights = {{0, 1}, {0.5, 1}, {1, 1}, {2, 1}, {1, 0}};

    std::ofstream summary("results/summary.csv");
    std::ofstream sweep("results/sweep.csv");
    if (!summary || !sweep) {
        std::fprintf(stderr, "cannot write to results/ (run via `make run`)\n");
        return 1;
    }
    summary << "instance,method,min,max,avg\n";
    sweep << "instance,base,w_regret,w_cost,min,avg\n";
    std::printf("instance,method,min,max,avg\n");

    char line[256];
    for (const auto& [instName, path] : instances) {
        Instance inst = load(path);
        const int K = requiredCount(inst.n);
        auto emit = [&](const std::string& name, const Stats& st) {
            std::snprintf(line, sizeof line, "%s,%s,%lld,%lld,%.2f\n", instName.c_str(), name.c_str(), st.min, st.max, st.avg);
            summary << line;
            std::printf("%s", line);
            std::ofstream bestFile("results/best_" + instName + "_" + name + ".txt");
            for (int v : st.best) bestFile << v << "\n";
        };
        emit("random", fromAssignment1(inst, solve_random(inst.n, K, inst.d, inst.cost)));
        emit("nn_end", fromAssignment1(inst, solve_nn_end(inst.n, K, inst.d, inst.cost)));
        emit("nn_any", fromAssignment1(inst, solve_nn_any(inst.n, K, inst.d, inst.cost)));
        emit("greedy_cycle", fromAssignment1(inst, solve_greedy_cycle(inst.n, K, inst.d, inst.cost)));
        for (const Method& m : methods) emit(m.name, runAll(inst, m.base, m.wRegret, m.wCost));
        for (Base base : {Base::Path, Base::Cycle}) {
            for (const auto& [wr, wc] : sweepWeights) {
                Stats st = runAll(inst, base, wr, wc);
                std::snprintf(line, sizeof line, "%s,%s,%g,%g,%lld,%.2f\n", instName.c_str(), baseName(base), wr, wc, st.min, st.avg);
                sweep << line;
            }
        }
    }
    return 0;
}
