#pragma once

#include <vector>
#include <set>
#include <random>
#include <algorithm>
#include "pipe.hpp"
#include "path.hpp"

struct World {
    int n;
    std::vector<std::vector<PipeType>> trueType;
    std::set<std::pair<int,int>> pathSet;

    World(int size, std::mt19937& rng, const std::vector<PathStep>& targetPath)
        : n(size), trueType(size, std::vector<PipeType>(size)) {
        for (auto& s : targetPath) pathSet.insert(s.pos);

        for (int i=0;i<n;i++) for (int j=0;j<n;j++)
            trueType[i][j] = static_cast<PipeType>(rng()%6);

        auto& src = targetPath.front();
        trueType[src.pos.first][src.pos.second] =
            maskToType((1<<src.inDir)|(1<<src.outDir));

        std::vector<std::pair<int,int>> nonPath;
        for (int i=0;i<n;i++) for (int j=0;j<n;j++)
            if (!pathSet.count({i,j})) nonPath.push_back({i,j});
        shuffle(nonPath.begin(), nonPath.end(), rng);

        size_t cur = 0;
        for (size_t k=1; k<targetPath.size(); k++) {
            auto& step = targetPath[k];
            PipeType correct = maskToType((1<<step.inDir)|(1<<step.outDir));

            PipeType& pt = trueType[step.pos.first][step.pos.second];
            while (pt == correct) pt = static_cast<PipeType>((int(pt)+1)%6);

            if (cur < nonPath.size()) {
                trueType[nonPath[cur].first][nonPath[cur].second] = correct;
                cur++;
            }
        }
    }
};
