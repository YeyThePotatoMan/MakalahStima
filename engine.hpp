#pragma once

#include "world.hpp"
#include "knowledge.hpp"
#include "path.hpp"

struct PuzzleEngine {
    World            world;
    SolverKnowledge  knowledge;
    std::vector<PathStep> targetPath;
    int  flowIdx    = 0;
    int  turnsUsed  = 0;
    int  turnBudget;
    bool solved     = false;

    PuzzleEngine(int n, std::mt19937& rng, int budget, const std::vector<PathStep>& path)
        : world(n, rng, path), knowledge(n), targetPath(path), turnBudget(budget) {
        auto& src = targetPath.front().pos;
        knowledge.locked[src.first][src.second] = true;
        knowledge.known [src.first][src.second] = true;
        knowledge.belief[src.first][src.second] = world.trueType[src.first][src.second];
        advanceFlow();
    }

    std::pair<int,int> currentFlowPos() const { return targetPath[flowIdx].pos; }
    int           currentFlowIdx()  const { return flowIdx; }
    bool          isOutOfTurns()    const { return turnsUsed >= turnBudget; }

    bool queryTile(int r, int c) {
        if (isOutOfTurns() || knowledge.known[r][c]) return false;
        turnsUsed++;
        knowledge.known [r][c] = true;
        knowledge.belief[r][c] = world.trueType[r][c];
        advanceFlow();
        return true;
    }

    bool swapTile(std::pair<int,int> A, std::pair<int,int> B) {
        if (isOutOfTurns() || A==B) return false;
        if (!knowledge.known[A.first][A.second] || !knowledge.known[B.first][B.second]) return false;
        if (knowledge.locked[A.first][A.second] || knowledge.locked[B.first][B.second]) return false;
        turnsUsed++;
        std::swap(world.trueType  [A.first][A.second], world.trueType  [B.first][B.second]);
        std::swap(knowledge.belief[A.first][A.second], knowledge.belief[B.first][B.second]);
        advanceFlow();
        return true;
    }

    void advanceFlow() {
        while (flowIdx+1 < (int)targetPath.size()) {
            const PathStep& next = targetPath[flowIdx+1];
            PipeType t = world.trueType[next.pos.first][next.pos.second];
            if (!pipeMatchesRequirement(t, next.inDir, next.outDir)) break;
            flowIdx++;
            knowledge.locked[next.pos.first][next.pos.second] = true;
            if (!knowledge.known[next.pos.first][next.pos.second]) {
                knowledge.known [next.pos.first][next.pos.second] = true;
                knowledge.belief[next.pos.first][next.pos.second] = t;
            }
        }
        if (flowIdx == (int)targetPath.size()-1) solved = true;
    }
};
