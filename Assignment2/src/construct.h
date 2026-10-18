#pragma once
#include "instance.h"
#include <climits>
#include <vector>

enum class Base { Cycle, Path };

struct Insertion {
    int best1 = INT_MAX;
    int best2 = INT_MAX;
    int pos = -1;
};

inline void consider(Insertion& ins, int delta, int pos) {
    if (delta < ins.best1) {
        ins.best2 = ins.best1;
        ins.best1 = delta;
        ins.pos = pos;
    } else if (delta < ins.best2) {
        ins.best2 = delta;
    }
}

inline Insertion bestInsertions(const Instance& inst, const std::vector<int>& sol, int k, Base base) {
    const auto& d = inst.d;
    const int c = inst.cost[k];
    const int m = static_cast<int>(sol.size());
    Insertion ins;
    if (base == Base::Cycle) {
        if (m == 1) {
            consider(ins, 2 * d[sol[0]][k] + c, 1);
        } else {
            for (int i = 0; i < m; ++i) {
                int a = sol[i], b = sol[(i + 1) % m];
                consider(ins, d[a][k] + d[k][b] - d[a][b] + c, i + 1);
            }
        }
    } else {
        consider(ins, d[k][sol[0]] + c, 0);
        for (int i = 0; i + 1 < m; ++i) {
            int a = sol[i], b = sol[i + 1];
            consider(ins, d[a][k] + d[k][b] - d[a][b] + c, i + 1);
        }
        consider(ins, d[sol[m - 1]][k] + c, m);
    }
    if (ins.best2 == INT_MAX) ins.best2 = ins.best1;
    return ins;
}

inline std::vector<int> construct(const Instance& inst, int start, int K, Base base,
                                  double wRegret, double wCost) {
    std::vector<int> sol{start};
    std::vector<bool> inSol(inst.n, false);
    inSol[start] = true;
    while (static_cast<int>(sol.size()) < K) {
        int bestK = -1, bestPos = -1, bestB1 = 0;
        double bestScore = 0;
        for (int k = 0; k < inst.n; ++k) {
            if (inSol[k]) continue;
            Insertion ins = bestInsertions(inst, sol, k, base);
            double score = wRegret * (ins.best2 - ins.best1) - wCost * ins.best1;
            if (bestK == -1 || score > bestScore || (score == bestScore && ins.best1 < bestB1)) {
                bestK = k;
                bestPos = ins.pos;
                bestB1 = ins.best1;
                bestScore = score;
            }
        }
        sol.insert(sol.begin() + bestPos, bestK);
        inSol[bestK] = true;
    }
    return sol;
}
