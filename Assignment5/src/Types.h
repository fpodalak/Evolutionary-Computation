#ifndef TYPES_H
#define TYPES_H

#include <string>
#include <vector>

struct Node {
    int x, y, cost;
};

// Position-based move used by the Assignment 3 / 4 local searches.
struct Move {
    enum Type { INTER_EXCHANGE, INTRA_NODE_SWAP, INTRA_EDGE_2OPT } type;
    int i;
    int j_or_v;
};

// Node-based move stored in the list of improving moves (Assignment 5).
// Positions change after every applied move, so the list stores node ids.
//   TWO_OPT : removes edges (a,b) and (c,d), adds (a,c) and (b,d).
//             Applicable when both edges exist with the same relative direction
//             (a->b and c->d, or both reversed).
//   EXCHANGE: node b (between a and c) is replaced by the unselected node d;
//             removes (a,b), (b,c), adds (a,d), (d,c).
struct ListMove {
    enum Type { TWO_OPT, EXCHANGE } type;
    int delta;
    int a, b, c, d;
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
