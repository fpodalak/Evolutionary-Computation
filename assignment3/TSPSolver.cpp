#include "TSPSolver.h"
#include <cmath>
#include <fstream>
#include <sstream>
#include <numeric>
#include <algorithm>
#include <limits>
#include <iostream>

using namespace std;

TSPSolver::TSPSolver(int seed) : n(0), k(0), rng(seed) {}

bool TSPSolver::loadInstance(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Could not open file: " << filename << endl;
        return false;
    }
    nodes.clear();
    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;
        
        // Usunięcie znaków końca linii Windows (\r)
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;

        // Usunięcie nagłówka UTF-8 BOM (\xEF\xBB\xBF) jeśli występuje
        if (line.size() >= 3 && (unsigned char)line[0] == 0xEF && (unsigned char)line[1] == 0xBB && (unsigned char)line[2] == 0xBF) {
            line = line.substr(3);
        }

        // Zamiana przecinków na średniki dla jednolitego parsowania
        for (char& c : line) {
            if (c == ',') c = ';';
        }

        stringstream ss(line);
        string val;
        vector<int> row;
        while (getline(ss, val, ';')) {
            if (val.empty()) continue;
            try {
                row.push_back(stoi(val));
            } catch (...) {}
        }

        if (row.size() < 3) {
            stringstream ss2(line);
            row.clear();
            int num;
            while (ss2 >> num) row.push_back(num);
        }

        if (row.size() >= 3) {
            nodes.push_back({row[0], row[1], row[2]});
        }
    }

    n = nodes.size();
    if (n == 0) {
        cerr << "Error: No valid nodes parsed from file: " << filename << endl;
        return false;
    }

    k = (n + 1) / 2;
    costs.resize(n);
    for (int i = 0; i < n; ++i) costs[i] = nodes[i].cost;

    D.assign(n, vector<int>(n, 0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i == j) D[i][j] = 0;
            else {
                double dist = sqrt(pow(nodes[i].x - nodes[j].x, 2) + pow(nodes[i].y - nodes[j].y, 2));
                D[i][j] = round(dist);
            }
        }
    }

    cout << "Successfully loaded " << n << " nodes from " << filename << endl;
    return true;
}

int TSPSolver::evaluate(const vector<int>& cycle) const {
    int total = 0;
    int sz = cycle.size();
    for (int i = 0; i < sz; ++i) {
        total += D[cycle[i]][cycle[(i + 1) % sz]];
        total += costs[cycle[i]];
    }
    return total;
}

vector<int> TSPSolver::get_random_solution() {
    vector<int> all(n);
    iota(all.begin(), all.end(), 0);
    shuffle(all.begin(), all.end(), rng);
    return vector<int>(all.begin(), all.begin() + k);
}

vector<int> TSPSolver::get_nn_end_solution(int start_node) {
    vector<bool> visited(n, false);
    visited[start_node] = true;
    vector<int> path = {start_node};

    while ((int)path.size() < k) {
        int last = path.back();
        int best_node = -1;
        int best_inc = numeric_limits<int>::max();
        for (int v = 0; v < n; ++v) {
            if (visited[v]) continue;
            int inc = D[last][v] + costs[v];
            if (inc < best_inc) {
                best_inc = inc;
                best_node = v;
            }
        }
        visited[best_node] = true;
        path.push_back(best_node);
    }
    return path;
}

vector<int> TSPSolver::get_nn_any_solution(int start_node) {
    vector<bool> visited(n, false);
    visited[start_node] = true;
    vector<int> path = {start_node};

    while ((int)path.size() < k) {
        int cur_m = path.size();
        int best_node = -1;
        int best_pos = -1;
        int best_inc = numeric_limits<int>::max();

        for (int v = 0; v < n; ++v) {
            if (visited[v]) continue;
            int inc_front = D[v][path[0]] + costs[v];
            if (inc_front < best_inc) {
                best_inc = inc_front;
                best_node = v;
                best_pos = 0;
            }
            int inc_end = D[path.back()][v] + costs[v];
            if (inc_end < best_inc) {
                best_inc = inc_end;
                best_node = v;
                best_pos = cur_m;
            }
            for (int i = 0; i < cur_m - 1; ++i) {
                int inc_in = D[path[i]][v] + D[v][path[i+1]] - D[path[i]][path[i+1]] + costs[v];
                if (inc_in < best_inc) {
                    best_inc = inc_in;
                    best_node = v;
                    best_pos = i + 1;
                }
            }
        }
        visited[best_node] = true;
        path.insert(path.begin() + best_pos, best_node);
    }
    return path;
}

vector<int> TSPSolver::get_greedy_cycle_solution(int start_node) {
    vector<bool> visited(n, false);
    visited[start_node] = true;
    vector<int> cycle = {start_node};

    while ((int)cycle.size() < k) {
        int cur_m = cycle.size();
        int best_node = -1;
        int best_pos = -1;
        int best_increase = numeric_limits<int>::max();

        for (int v = 0; v < n; ++v) {
            if (visited[v]) continue;
            for (int i = 0; i < cur_m; ++i) {
                int u1 = cycle[i];
                int u2 = cycle[(i + 1) % cur_m];
                int increase;
                if (cur_m == 1) {
                    increase = 2 * D[u1][v] + costs[v];
                } else {
                    increase = D[u1][v] + D[v][u2] - D[u1][u2] + costs[v];
                }
                if (increase < best_increase) {
                    best_increase = increase;
                    best_node = v;
                    best_pos = i + 1;
                }
            }
        }
        visited[best_node] = true;
        cycle.insert(cycle.begin() + best_pos, best_node);
    }
    return cycle;
}

int TSPSolver::calc_delta_inter(const vector<int>& cycle, int i, int v) const {
    int sz = cycle.size();
    int prev = cycle[(i - 1 + sz) % sz];
    int curr = cycle[i];
    int next = cycle[(i + 1) % sz];
    return D[prev][v] + D[v][next] - D[prev][curr] - D[curr][next] + costs[v] - costs[curr];
}

int TSPSolver::calc_delta_node_swap(const vector<int>& cycle, int i, int j) const {
    int sz = cycle.size();
    if (i == j) return 0;
    if (i > j) swap(i, j);

    int pi = cycle[i], pj = cycle[j];

    if (j == i + 1) {
        int prev_i = cycle[(i - 1 + sz) % sz];
        int next_j = cycle[(j + 1) % sz];
        return D[prev_i][pj] + D[pi][next_j] - D[prev_i][pi] - D[pj][next_j];
    } else if (i == 0 && j == sz - 1) {
        int prev_j = cycle[j - 1];
        int next_i = cycle[1];
        return D[prev_j][pi] + D[pj][next_i] - D[prev_j][pj] - D[pi][next_i];
    } else {
        int prev_i = cycle[(i - 1 + sz) % sz];
        int next_i = cycle[(i + 1) % sz];
        int prev_j = cycle[(j - 1 + sz) % sz];
        int next_j = cycle[(j + 1) % sz];
        return (D[prev_i][pj] + D[pj][next_i] + D[prev_j][pi] + D[pi][next_j])
             - (D[prev_i][pi] + D[pi][next_i] + D[prev_j][pj] + D[pj][next_j]);
    }
}

int TSPSolver::calc_delta_2opt(const vector<int>& cycle, int i, int j) const {
    int sz = cycle.size();
    int pi = cycle[i];
    int pi1 = cycle[(i + 1) % sz];
    int pj = cycle[j];
    int pj1 = cycle[(j + 1) % sz];
    return D[pi][pj] + D[pi1][pj1] - D[pi][pi1] - D[pj][pj1];
}

void TSPSolver::apply_move(vector<int>& cycle, const Move& m) const {
    int sz = cycle.size();
    if (m.type == Move::INTER_EXCHANGE) {
        cycle[m.i] = m.j_or_v;
    } else if (m.type == Move::INTRA_NODE_SWAP) {
        swap(cycle[m.i], cycle[m.j_or_v]);
    } else if (m.type == Move::INTRA_EDGE_2OPT) {
        int i = m.i;
        int j = m.j_or_v;
        int left = (i + 1) % sz;
        int right = j;
        if (left <= right) {
            reverse(cycle.begin() + left, cycle.begin() + right + 1);
        } else {
            vector<int> seg;
            for (int idx = left; idx != (right + 1) % sz; idx = (idx + 1) % sz) {
                seg.push_back(cycle[idx]);
            }
            reverse(seg.begin(), seg.end());
            int seg_idx = 0;
            for (int idx = left; idx != (right + 1) % sz; idx = (idx + 1) % sz) {
                cycle[idx] = seg[seg_idx++];
            }
        }
    }
}

vector<int> TSPSolver::local_search(vector<int> cycle, bool is_steepest, bool is_2opt) {
    vector<bool> in_cycle(n, false);
    for (int v : cycle) in_cycle[v] = true;

    while (true) {
        int sz = cycle.size();
        vector<int> unselected;
        unselected.reserve(n - sz);
        for (int v = 0; v < n; ++v) {
            if (!in_cycle[v]) unselected.push_back(v);
        }

        if (is_steepest) {
            int best_delta = 0;
            Move best_move;

            for (int i = 0; i < sz; ++i) {
                for (int v : unselected) {
                    int delta = calc_delta_inter(cycle, i, v);
                    if (delta < best_delta) {
                        best_delta = delta;
                        best_move = {Move::INTER_EXCHANGE, i, v};
                    }
                }
            }

            if (!is_2opt) {
                for (int i = 0; i < sz - 1; ++i) {
                    for (int j = i + 1; j < sz; ++j) {
                        int delta = calc_delta_node_swap(cycle, i, j);
                        if (delta < best_delta) {
                            best_delta = delta;
                            best_move = {Move::INTRA_NODE_SWAP, i, j};
                        }
                    }
                }
            } else {
                for (int i = 0; i < sz; ++i) {
                    for (int j = i + 1; j < sz; ++j) {
                        if ((i == 0 && j == sz - 1) || (j == i + 1)) continue;
                        int delta = calc_delta_2opt(cycle, i, j);
                        if (delta < best_delta) {
                            best_delta = delta;
                            best_move = {Move::INTRA_EDGE_2OPT, i, j};
                        }
                    }
                }
            }

            if (best_delta < 0) {
                if (best_move.type == Move::INTER_EXCHANGE) {
                    in_cycle[cycle[best_move.i]] = false;
                    in_cycle[best_move.j_or_v] = true;
                }
                apply_move(cycle, best_move);
            } else {
                break;
            }

        } else {
            vector<Move> moves;
            moves.reserve(sz * unselected.size() + sz * sz / 2);

            for (int i = 0; i < sz; ++i) {
                for (int v : unselected) {
                    moves.push_back({Move::INTER_EXCHANGE, i, v});
                }
            }

            if (!is_2opt) {
                for (int i = 0; i < sz - 1; ++i) {
                    for (int j = i + 1; j < sz; ++j) {
                        moves.push_back({Move::INTRA_NODE_SWAP, i, j});
                    }
                }
            } else {
                for (int i = 0; i < sz; ++i) {
                    for (int j = i + 1; j < sz; ++j) {
                        if ((i == 0 && j == sz - 1) || (j == i + 1)) continue;
                        moves.push_back({Move::INTRA_EDGE_2OPT, i, j});
                    }
                }
            }

            shuffle(moves.begin(), moves.end(), rng);

            bool improved = false;
            for (const auto& m : moves) {
                int delta = 0;
                if (m.type == Move::INTER_EXCHANGE) delta = calc_delta_inter(cycle, m.i, m.j_or_v);
                else if (m.type == Move::INTRA_NODE_SWAP) delta = calc_delta_node_swap(cycle, m.i, m.j_or_v);
                else if (m.type == Move::INTRA_EDGE_2OPT) delta = calc_delta_2opt(cycle, m.i, m.j_or_v);

                if (delta < 0) {
                    if (m.type == Move::INTER_EXCHANGE) {
                        in_cycle[cycle[m.i]] = false;
                        in_cycle[m.j_or_v] = true;
                    }
                    apply_move(cycle, m);
                    improved = true;
                    break;
                }
            }

            if (!improved) break;
        }
    }
    return cycle;
}