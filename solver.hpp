#pragma once

#include <chrono>
#include <climits>
#include <set>
#include "engine.hpp"
#include "path.hpp"

struct ActionCandidate {
    enum class Kind { QUERY_PATH, QUERY_NONPATH, SWAP } kind;
    std::pair<int,int> posA, posB;
    double f;
    int    pathIdx;
};

struct SolveResult {
    bool      success          = false;
    int       turnsUsed        = 0;
    long long actionsEvaluated = 0;
    double    timeMs           = 0.0;
};

inline SolveResult runSolver(int n, std::mt19937 rng, int turnBudget, bool useAStar) {
    auto t0 = std::chrono::high_resolution_clock::now();

    std::vector<PathStep> targetPath = buildStaircasePath({0,0},{n-1,n-1});
    PuzzleEngine engine(n, rng, turnBudget, targetPath);

    std::set<std::pair<int,int>> pathPosSet;
    for (auto& s : targetPath) pathPosSet.insert(s.pos);

    long long actionsEvaluated = 0;

    while (!engine.solved && !engine.isOutOfTurns()) {
        auto flowPos = engine.currentFlowPos();
        int  fIdx    = engine.currentFlowIdx();

        std::vector<std::pair<int,int>> knownNonPath;
        for (int r=0;r<n;r++) for (int c=0;c<n;c++)
            if (!pathPosSet.count({r,c}) &&
                engine.knowledge.known[r][c] &&
               !engine.knowledge.locked[r][c])
                knownNonPath.push_back({r,c});

        std::pair<int,int> firstStuck = {-1,-1};
        int firstStuckPathIdx = -1;
        for (int i=1; i<(int)targetPath.size(); i++) {
            auto& step = targetPath[i]; auto [r,c] = step.pos;
            if (engine.knowledge.locked[r][c]) continue;
            if (!engine.knowledge.known[r][c]) {
                firstStuck = step.pos; firstStuckPathIdx = i; break;
            }
            if (!pipeMatchesRequirement(engine.knowledge.belief[r][c], step.inDir, step.outDir)) {
                bool has = false;
                for (auto& src : knownNonPath)
                    if (pipeMatchesRequirement(engine.knowledge.belief[src.first][src.second],
                                               step.inDir, step.outDir)) { has=true; break; }
                if (!has) { firstStuck = step.pos; firstStuckPathIdx = i; break; }
            }
        }

        ActionCandidate best;
        double bestF   = 1e18;
        int    bestPathIdx = INT_MAX;
        bool   hasAny  = false;

        auto tryUpdate = [&](ActionCandidate cand, double f, int pi) {
            if (f < bestF || (f==bestF && pi < bestPathIdx)) {
                bestF=f; bestPathIdx=pi; best=cand; hasAny=true;
            }
        };

        for (int i=1; i<(int)targetPath.size(); i++) {
            auto& step = targetPath[i]; auto [r,c] = step.pos;
            if (engine.knowledge.known[r][c])  continue;
            if (engine.knowledge.locked[r][c]) continue;
            actionsEvaluated++;
            int g = i - fIdx;
            int h = manhattan(step.pos, flowPos);
            double f = useAStar ? (g + h) : (double)h;
            tryUpdate({ActionCandidate::Kind::QUERY_PATH, step.pos, {-1,-1}, f, i}, f, i);
        }

        for (int i=1; i<(int)targetPath.size(); i++) {
            auto& step = targetPath[i]; auto [tr,tc] = step.pos;
            if (!engine.knowledge.known[tr][tc])  continue;
            if ( engine.knowledge.locked[tr][tc]) continue;
            if (pipeMatchesRequirement(engine.knowledge.belief[tr][tc],
                                       step.inDir, step.outDir)) continue;

            for (auto& src : knownNonPath) {
                actionsEvaluated++;
                PipeType srcType = engine.knowledge.belief[src.first][src.second];
                if (!pipeMatchesRequirement(srcType, step.inDir, step.outDir)) continue;

                int g = i - fIdx;
                int h = manhattan(step.pos, flowPos);
                double f = useAStar ? (g + h) : (double)h;
                tryUpdate({ActionCandidate::Kind::SWAP, step.pos, src, f, i}, f, i);
            }
        }

        if (firstStuck.first != -1) {
            for (int r=0; r<n; r++) {
                for (int c=0; c<n; c++) {
                    if (pathPosSet.count({r,c}))      continue;
                    if (engine.knowledge.known[r][c]) continue;
                    actionsEvaluated++;
                    int g = manhattan({r,c}, firstStuck);
                    int h = manhattan(firstStuck, flowPos);
                    double f = useAStar ? (g + h) : (double)g;
                    f += 1000.0;
                    tryUpdate({ActionCandidate::Kind::QUERY_NONPATH, {r,c}, {-1,-1}, f, INT_MAX-1}, f, INT_MAX-1);
                }
            }
        }

        if (!hasAny) break;

        if (best.kind == ActionCandidate::Kind::SWAP)
            engine.swapTile(best.posA, best.posB);
        else
            engine.queryTile(best.posA.first, best.posA.second);
    }

    auto t1 = std::chrono::high_resolution_clock::now();

    SolveResult res;
    res.success          = engine.solved;
    res.turnsUsed        = engine.turnsUsed;
    res.actionsEvaluated = actionsEvaluated;
    res.timeMs           = std::chrono::duration<double,std::milli>(t1-t0).count();
    return res;
}
