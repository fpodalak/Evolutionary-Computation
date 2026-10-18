#pragma once
#include <cmath>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

struct Instance {
    int n = 0;
    std::vector<int> cost;
    std::vector<std::vector<int>> d;
};

inline Instance fromPoints(const std::vector<int>& x, const std::vector<int>& y,
                           const std::vector<int>& cost) {
    Instance inst;
    inst.n = static_cast<int>(x.size());
    inst.cost = cost;
    inst.d.assign(inst.n, std::vector<int>(inst.n, 0));
    for (int i = 0; i < inst.n; ++i)
        for (int j = 0; j < inst.n; ++j) {
            double dx = x[i] - x[j], dy = y[i] - y[j];
            inst.d[i][j] = static_cast<int>(std::floor(std::sqrt(dx * dx + dy * dy) + 0.5));
        }
    return inst;
}

inline Instance load(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("cannot open " + path);
    std::vector<int> x, y, cost;
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;
        for (char& ch : line) if (ch == ';') ch = ' ';
        std::istringstream ss(line);
        int a, b, c;
        if (!(ss >> a >> b >> c)) throw std::runtime_error("bad line in " + path + ": " + line);
        x.push_back(a); y.push_back(b); cost.push_back(c);
    }
    return fromPoints(x, y, cost);
}

inline int requiredCount(int n) { return (n + 1) / 2; }

inline long long evaluate(const Instance& inst, const std::vector<int>& sol) {
    long long total = 0;
    const int m = static_cast<int>(sol.size());
    for (int i = 0; i < m; ++i)
        total += inst.cost[sol[i]] + inst.d[sol[i]][sol[(i + 1) % m]];
    return total;
}

inline bool isValid(const Instance& inst, const std::vector<int>& sol, int K) {
    if (static_cast<int>(sol.size()) != K) return false;
    std::vector<bool> seen(inst.n, false);
    for (int v : sol) {
        if (v < 0 || v >= inst.n || seen[v]) return false;
        seen[v] = true;
    }
    return true;
}
