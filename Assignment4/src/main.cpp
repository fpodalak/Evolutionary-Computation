#include "TSPSolver.h"
#include "construct.h"
#include "instance.h"
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <string>
#include <vector>

using namespace std;
using Clock = chrono::high_resolution_clock;

constexpr int RUNS = 200;

struct Stats {
    long long min = 0, max = 0;
    double avg = 0, tmin = 0, tmax = 0, tavg = 0;
    vector<int> best;
};

Stats runMethod(const Instance& inst, int K, const function<vector<int>(int)>& build) {
    Stats st;
    long long sum = 0;
    double tsum = 0;
    for (int r = 0; r < RUNS; ++r) {
        auto t0 = Clock::now();
        vector<int> sol = build(r);
        double ms = chrono::duration<double, milli>(Clock::now() - t0).count();
        if (!isValid(inst, sol, K)) {
            fprintf(stderr, "invalid solution in run %d\n", r);
            exit(1);
        }
        long long v = evaluate(inst, sol);
        sum += v;
        tsum += ms;
        if (r == 0 || v < st.min) { st.min = v; st.best = sol; }
        if (r == 0 || v > st.max) st.max = v;
        if (r == 0 || ms < st.tmin) st.tmin = ms;
        if (r == 0 || ms > st.tmax) st.tmax = ms;
    }
    st.avg = static_cast<double>(sum) / RUNS;
    st.tavg = tsum / RUNS;
    return st;
}

int main() {
    const vector<pair<string, string>> instances = {{"A", "TSPA.csv"}, {"B", "TSPB.csv"}};
    const vector<int> sweepK = {5, 10, 15, 20};

    ofstream summary("results/summary.csv");
    ofstream candidates("results/candidates.csv");
    ofstream build("results/candidate_build.csv");
    if (!summary || !candidates || !build) {
        fprintf(stderr, "cannot write to results/ (run via `make run`)\n");
        return 1;
    }
    summary << "instance,group,method,min,max,avg,time_min_ms,time_max_ms,time_avg_ms\n";
    candidates << "instance,K,min,max,avg,time_avg_ms\n";
    build << "instance,K,build_ms\n";

    char line[512];
    for (const auto& [name, path] : instances) {
        TSPSolver s(42);
        if (!s.loadInstance(path)) return 1;
        const Instance inst{s.n, s.costs, s.D};
        const int K = s.k;

        auto emit = [&](const string& group, const string& method, const Stats& st) {
            snprintf(line, sizeof line, "%s,%s,%s,%lld,%lld,%.2f,%.4f,%.4f,%.4f\n", name.c_str(), group.c_str(),
                     method.c_str(), st.min, st.max, st.avg, st.tmin, st.tmax, st.tavg);
            summary << line;
            printf("%s", line);
            ofstream best("results/best_" + name + "_" + method + ".txt");
            for (int v : st.best) best << v << "\n";
        };

        vector<vector<int>> starts(RUNS);
        emit("A1", "random", runMethod(inst, K, [&](int r) { return starts[r] = s.get_random_solution(); }));
        emit("A1", "nn_end", runMethod(inst, K, [&](int r) { return s.get_nn_end_solution(r % s.n); }));
        emit("A1", "nn_any", runMethod(inst, K, [&](int r) { return s.get_nn_any_solution(r % s.n); }));
        emit("A1", "greedy_cycle", runMethod(inst, K, [&](int r) { return s.get_greedy_cycle_solution(r % s.n); }));

        emit("A2", "nn_any_regret", runMethod(inst, K, [&](int r) { return construct(inst, r % s.n, K, Base::Path, 1, 0); }));
        emit("A2", "nn_any_weighted", runMethod(inst, K, [&](int r) { return construct(inst, r % s.n, K, Base::Path, 1, 1); }));
        emit("A2", "greedy_cycle_regret", runMethod(inst, K, [&](int r) { return construct(inst, r % s.n, K, Base::Cycle, 1, 0); }));
        emit("A2", "greedy_cycle_weighted", runMethod(inst, K, [&](int r) { return construct(inst, r % s.n, K, Base::Cycle, 1, 1); }));

        for (bool greedyStart : {false, true}) {
            for (bool steepest : {true, false}) {
                for (bool two : {false, true}) {
                    string key = string("ls_") + (steepest ? "steepest" : "greedy") + "_" + (two ? "2opt" : "swap") + "_" +
                                 (greedyStart ? "greedy" : "random");
                    emit("A3", key, runMethod(inst, K, [&](int r) {
                        vector<int> init = greedyStart ? s.get_greedy_cycle_solution(r % s.n) : starts[r];
                        return s.local_search(init, steepest, two);
                    }));
                }
            }
        }

        Stats atTen;
        for (int k : sweepK) {
            auto t0 = Clock::now();
            s.build_candidates(k);
            double ms = chrono::duration<double, milli>(Clock::now() - t0).count();
            snprintf(line, sizeof line, "%s,%d,%.4f\n", name.c_str(), k, ms);
            build << line;
            Stats st = runMethod(inst, K, [&](int r) { return s.local_search_candidates(starts[r]); });
            snprintf(line, sizeof line, "%s,%d,%lld,%lld,%.2f,%.4f\n", name.c_str(), k, st.min, st.max, st.avg, st.tavg);
            candidates << line;
            if (k == 10) atTen = st;
        }
        emit("A4", "ls_steepest_2opt_candidates", atTen);
    }
    return 0;
}
