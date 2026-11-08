// Assignment 5: steepest local search that re-uses move evaluations (deltas)
// from previous iterations via a list of improving moves (LM).
//
// Neighbourhood (same as Assignment 3/4): 2-opt (two-edges exchange) inside the
// cycle + exchange of a selected node with an unselected one.
//
// Moves are stored by node ids (positions change when segments are reversed).
//   2-opt  : removed edges (a,b), (c,d), added edges (a,c), (b,d)
//   exchange: node x between p and q is replaced by v
//
// Browsing LM from the best move, every stored move is in one of three situations:
//   1. a removed edge no longer exists          -> delete the move from LM
//   2. edges exist, relative direction differs   -> keep the move, skip it for now
//   3. edges exist, same relative direction      -> apply it (and delete it from LM)
//      (both edges reversed counts as the same direction)
//
// After a move is applied only the moves that involve a NEW edge (or a node whose
// selected/unselected status changed) have to be evaluated. For every pair of edges
// both reconnections are evaluated: the one applicable now and the "inverted" one,
// which becomes applicable if the relative direction of the two edges flips later.

#include "TSPSolver.h"

#include <cstdio>
#include <cstdlib>
#include <set>
#include <utility>

using namespace std;

namespace {

struct ByDelta {
    bool operator()(const ListMove& x, const ListMove& y) const { return x.delta < y.delta; }
};

enum class Status { Drop, Keep, Apply };

[[noreturn]] void fail(const char* msg) {
    fprintf(stderr, "LM verification failed: %s\n", msg);
    exit(1);
}

class LMSearch {
public:
    LMSearch(TSPSolver& solver, vector<int> start, bool use_cand)
        : S(solver), D(solver.D), cost(solver.costs), cycle(std::move(start)),
          sz(static_cast<int>(cycle.size())), useCand(use_cand), pos(solver.n, -1), uidx(solver.n, -1) {
        if (useCand && S.is_cand.empty()) fail("candidates requested but build_candidates() was not called");
        for (int i = 0; i < sz; ++i) pos[cycle[i]] = i;
        for (int v = 0; v < S.n; ++v)
            if (pos[v] < 0) {
                uidx[v] = static_cast<int>(unsel.size());
                unsel.push_back(v);
            }
    }

    vector<int> run(bool verify) {
        initialMoves();
        long long cur = verify ? S.evaluate(cycle) : 0;
        while (true) {
            const int expected = verify ? fullBest() : 0;
            bool applied = false;
            for (auto it = lm.begin(); it != lm.end();) {
                int i = 0, j = 0;
                const Status st = check(*it, i, j);
                if (st == Status::Drop) {          // situation 1
                    it = lm.erase(it);
                    continue;
                }
                if (st == Status::Keep) {          // situation 2
                    ++it;
                    continue;
                }
                const ListMove m = *it;            // situation 3
                lm.erase(it);
                if (verify && m.delta != expected) fail("best applicable LM move != best move of the neighbourhood");
                apply(m, i, j);
                if (verify) {
                    cur += m.delta;
                    if (cur != S.evaluate(cycle)) fail("tracked objective differs from recomputed objective");
                }
                applied = true;
                break;
            }
            if (!applied) {
                if (verify && expected != 0) fail("LM empty but an improving move exists");
                break;
            }
            ++iters;
        }
        S.last_evals = evals;
        S.last_iters = iters;
        return cycle;
    }

private:
    TSPSolver& S;
    const vector<vector<int>>& D;
    const vector<int>& cost;
    vector<int> cycle;
    int sz;
    bool useCand;
    vector<int> pos;     // position in cycle or -1
    vector<int> unsel;   // unselected nodes
    vector<int> uidx;    // index in unsel or -1
    multiset<ListMove, ByDelta> lm;
    long long evals = 0, iters = 0;

    int nxt(int i) const { return i + 1 == sz ? 0 : i + 1; }
    int prv(int i) const { return i == 0 ? sz - 1 : i - 1; }
    int succ(int u) const { return cycle[nxt(pos[u])]; }
    int pred(int u) const { return cycle[prv(pos[u])]; }

    // edge u->v exists in the current cycle in this direction
    bool fwd(int u, int v) const { return pos[u] >= 0 && pos[v] >= 0 && cycle[nxt(pos[u])] == v; }

    bool candEdge(int u, int v) const { return !useCand || S.is_cand[u][v] || S.is_cand[v][u]; }
    bool candExchange(int p, int q, int v) const { return !useCand || S.is_cand[p][v] || S.is_cand[q][v]; }

    // ---- evaluation of new moves -------------------------------------------------

    // Edges (a,b), (c,d) with b = succ a, d = succ c, all four nodes distinct.
    void tryTwoOpt(int a, int b, int c, int d) {
        const int removed = D[a][b] + D[c][d];
        if (candEdge(a, c) || candEdge(b, d)) {            // applicable now
            ++evals;
            const int delta = D[a][c] + D[b][d] - removed;
            if (delta < 0) lm.insert({ListMove::TWO_OPT, delta, a, b, c, d});
        }
        if (candEdge(a, d) || candEdge(b, c)) {            // relatively inverted edges
            ++evals;
            const int delta = D[a][d] + D[b][c] - removed;
            if (delta < 0) lm.insert({ListMove::TWO_OPT, delta, a, b, d, c});
        }
    }

    // selected node x, unselected node v
    void tryExchange(int x, int v) {
        const int p = pred(x), q = succ(x);
        if (!candExchange(p, q, v)) return;
        ++evals;
        const int delta = D[p][v] + D[v][q] - D[p][x] - D[x][q] + cost[v] - cost[x];
        if (delta < 0) lm.insert({ListMove::EXCHANGE, delta, p, x, q, v});
    }

    void initialMoves() {
        for (int i = 0; i < sz; ++i)
            for (int j = i + 1; j < sz; ++j) {
                if (j == i + 1 || (i == 0 && j == sz - 1)) continue;
                tryTwoOpt(cycle[i], cycle[nxt(i)], cycle[j], cycle[nxt(j)]);
            }
        for (int i = 0; i < sz; ++i)
            for (int v : unsel) tryExchange(cycle[i], v);
    }

    // `changed`: selected nodes that got a new neighbour, `ne`: the new edges,
    // `newUnsel`: node that just left the cycle (or -1).
    void generate(const vector<int>& changed, vector<pair<int, int>> ne, int newUnsel) {
        for (auto& e : ne)
            if (succ(e.first) != e.second) swap(e.first, e.second);   // orient along the cycle

        // 2-opt: every new edge paired with every other edge (pairs of two new edges only once)
        for (size_t t = 0; t < ne.size(); ++t) {
            const int a = ne[t].first, b = ne[t].second;
            for (int k = 0; k < sz; ++k) {
                const int c = cycle[k], d = cycle[nxt(k)];
                if (c == a || c == b || d == a || d == b) continue;
                bool dup = false;
                for (size_t l = 0; l < t; ++l)
                    if (ne[l].first == c) dup = true;
                if (!dup) tryTwoOpt(a, b, c, d);
            }
        }
        // exchange: nodes with a new neighbour against every unselected node ...
        for (int s : changed)
            for (int v : unsel) tryExchange(s, v);
        // ... and the node that just became unselected against all other selected nodes
        if (newUnsel >= 0)
            for (int y : cycle) {
                bool isChanged = false;
                for (int s : changed)
                    if (s == y) isChanged = true;
                if (!isChanged) tryExchange(y, newUnsel);
            }
    }

    // ---- applicability and application -------------------------------------------

    // On Apply, (i, j) are the parameters for the position-based move.
    Status check(const ListMove& m, int& i, int& j) const {
        if (m.type == ListMove::TWO_OPT) {
            const bool s1 = fwd(m.a, m.b), r1 = fwd(m.b, m.a);
            const bool s2 = fwd(m.c, m.d), r2 = fwd(m.d, m.c);
            if (!(s1 || r1) || !(s2 || r2)) return Status::Drop;
            if (s1 && s2) { i = pos[m.a]; j = pos[m.c]; return Status::Apply; }
            if (r1 && r2) { i = pos[m.b]; j = pos[m.d]; return Status::Apply; }
            return Status::Keep;
        }
        if (pos[m.d] >= 0 || pos[m.b] < 0) return Status::Drop;       // v selected / x gone
        const bool f = fwd(m.a, m.b) && fwd(m.b, m.c);
        const bool r = fwd(m.b, m.a) && fwd(m.c, m.b);
        if (!(f || r)) return Status::Drop;
        i = pos[m.b];
        j = m.d;
        return Status::Apply;
    }

    void apply(const ListMove& m, int i, int j) {
        if (m.type == ListMove::TWO_OPT) {
            S.apply_move(cycle, Move{Move::INTRA_EDGE_2OPT, i, j});
            const int stop = nxt(j);
            for (int idx = nxt(i); idx != stop; idx = nxt(idx)) pos[cycle[idx]] = idx;
            generate({m.a, m.b, m.c, m.d}, {{m.a, m.c}, {m.b, m.d}}, -1);
        } else {
            const int x = m.b, v = m.d;
            cycle[i] = v;
            pos[v] = i;
            pos[x] = -1;
            const int u = uidx[v];
            unsel[u] = x;
            uidx[x] = u;
            uidx[v] = -1;
            generate({m.a, v, m.c}, {{m.a, v}, {v, m.c}}, x);
        }
    }

    // ---- verification (tests only) -----------------------------------------------

    // Best delta (<= 0) over the whole neighbourhood, restricted to candidate moves if useCand.
    int fullBest() const {
        int best = 0;
        for (int i = 0; i < sz; ++i) {
            const int x = cycle[i], p = cycle[prv(i)], q = cycle[nxt(i)];
            for (int v : unsel) {
                if (!candExchange(p, q, v)) continue;
                best = min(best, D[p][v] + D[v][q] - D[p][x] - D[x][q] + cost[v] - cost[x]);
            }
        }
        for (int i = 0; i < sz; ++i)
            for (int j = i + 1; j < sz; ++j) {
                if (j == i + 1 || (i == 0 && j == sz - 1)) continue;
                const int a = cycle[i], b = cycle[nxt(i)], c = cycle[j], d = cycle[nxt(j)];
                if (!candEdge(a, c) && !candEdge(b, d)) continue;
                best = min(best, D[a][c] + D[b][d] - D[a][b] - D[c][d]);
            }
        return best;
    }
};

}  // namespace

vector<int> TSPSolver::local_search_lm(vector<int> cycle, bool use_candidates, bool verify) {
    LMSearch search(*this, std::move(cycle), use_candidates);
    return search.run(verify);
}
