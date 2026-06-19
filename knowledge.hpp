#pragma once

#include <vector>
#include "pipe.hpp"

struct SolverKnowledge {
    int n;
    std::vector<std::vector<bool>>     known;
    std::vector<std::vector<bool>>     locked;
    std::vector<std::vector<PipeType>> belief;

    SolverKnowledge(int size) : n(size) {
        known.assign (n, std::vector<bool>    (n, false));
        locked.assign(n, std::vector<bool>    (n, false));
        belief.assign(n, std::vector<PipeType>(n, PipeType::STRAIGHT_V));
    }
};
