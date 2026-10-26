#ifndef TYPES_H
#define TYPES_H

#include <string>
#include <vector>

struct Node {
    int x, y, cost;
};

struct Move {
    enum Type { INTER_EXCHANGE, INTRA_NODE_SWAP, INTRA_EDGE_2OPT } type;
    int i;
    int j_or_v;
};

struct ResultStats {
    std::string name;
    double min_cost;
    double max_cost;
    double avg_cost;
    double min_time_ms;
    double max_time_ms;
    double avg_time_ms;
    std::vector<int> best_cycle;
};

#endif