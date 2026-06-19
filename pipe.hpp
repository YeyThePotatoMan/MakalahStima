#pragma once

#include <cstdlib>
#include <utility>

enum class PipeType {
    STRAIGHT_V,
    STRAIGHT_H,
    ELBOW_UR,
    ELBOW_RD,
    ELBOW_DL,
    ELBOW_LU
};

const int DR[4] = {-1, 0, 1, 0};
const int DC[4] = {0, 1, 0, -1};
const char* DIR_NAME[4] = {"Up", "Right", "Down", "Left"};

inline int fixedPorts(PipeType t) {
    switch (t) {
        case PipeType::STRAIGHT_V: return 0b0101;
        case PipeType::STRAIGHT_H: return 0b1010;
        case PipeType::ELBOW_UR:   return 0b0011;
        case PipeType::ELBOW_RD:   return 0b0110;
        case PipeType::ELBOW_DL:   return 0b1100;
        case PipeType::ELBOW_LU:   return 0b1001;
    }
    return 0;
}

inline int opposite(int dir) {
    return (dir + 2) % 4;
}

inline bool pipeMatchesRequirement(PipeType t, int inDir, int outDir) {
    int ports = fixedPorts(t);
    return (ports & (1 << inDir)) && (ports & (1 << outDir));
}

inline PipeType maskToType(int mask) {
    if (mask == 0b0101) return PipeType::STRAIGHT_V;
    if (mask == 0b1010) return PipeType::STRAIGHT_H;
    if (mask == 0b0011) return PipeType::ELBOW_UR;
    if (mask == 0b0110) return PipeType::ELBOW_RD;
    if (mask == 0b1100) return PipeType::ELBOW_DL;
    if (mask == 0b1001) return PipeType::ELBOW_LU;
    return PipeType::STRAIGHT_V;
}

inline int manhattan(std::pair<int,int> a, std::pair<int,int> b) {
    return abs(a.first - b.first) + abs(a.second - b.second);
}
