#pragma once

#include <vector>
#include <utility>
#include "pipe.hpp"

struct PathStep {
    std::pair<int,int> pos;
    int inDir, outDir;
};

inline std::vector<PathStep> buildStaircasePath(std::pair<int,int> source, std::pair<int,int> sink) {
    std::vector<std::pair<int,int>> cells;
    int r = source.first, c = source.second;
    cells.push_back({r, c});
    while (r < sink.first)  { r++; cells.push_back({r, c}); }
    while (c < sink.second) { c++; cells.push_back({r, c}); }

    std::vector<PathStep> path;
    for (size_t i = 0; i < cells.size(); i++) {
        int inDir = -1, outDir = -1;
        if (i > 0) {
            auto [pr,pc] = cells[i-1]; auto [cr,cc] = cells[i];
            for (int d=0;d<4;d++) if (pr+DR[d]==cr&&pc+DC[d]==cc) inDir=opposite(d);
        }
        if (i+1 < cells.size()) {
            auto [cr,cc] = cells[i]; auto [nr,nc] = cells[i+1];
            for (int d=0;d<4;d++) if (cr+DR[d]==nr&&cc+DC[d]==nc) outDir=d;
        }
        if (inDir  == -1) inDir  = (outDir+2)%4;
        if (outDir == -1) outDir = (inDir +2)%4;
        path.push_back({cells[i], inDir, outDir});
    }
    return path;
}
