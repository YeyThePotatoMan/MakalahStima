#pragma once

#include <iostream>
#include <iomanip>
#include <random>
#include "solver.hpp"

inline void runBenchmark() {
    std::vector<int> sizes = {10, 20, 30, 50, 80, 100};
    const int TRIALS = 100;
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "GridSize,Algorithm,AvgTimeMs,AvgActionsEvaluated,SuccessRate,AvgTurnsUsed\n";

    for (int n : sizes) {
        int turnBudget = n * n * 4;
        double tTA=0,tTG=0; long long tAA=0,tAG=0;
        int sA=0,sG=0; long long tTuA=0,tTuG=0;

        for (int t=0; t<TRIALS; t++) {
            unsigned seed = 42 + t;
            std::mt19937 rA(seed), rG(seed);
            auto ra = runSolver(n, rA, turnBudget, true);
            auto rg = runSolver(n, rG, turnBudget, false);
            tTA+=ra.timeMs; tTG+=rg.timeMs;
            tAA+=ra.actionsEvaluated; tAG+=rg.actionsEvaluated;
            tTuA+=ra.turnsUsed; tTuG+=rg.turnsUsed;
            if(ra.success) sA++;
            if(rg.success) sG++;
        }

        auto row=[&](const std::string& algo,double tT,long long tA,int s,long long tTu){
            std::cout<<n<<"x"<<n<<","<<algo<<","
                <<std::fixed<<std::setprecision(4)<<(tT/TRIALS)<<","
                <<(tA/TRIALS)<<","
                <<std::fixed<<std::setprecision(1)<<(100.0*s/TRIALS)<<"%,"
                <<std::fixed<<std::setprecision(2)<<(double)tTu/TRIALS<<"\n";
        };
        row("A*",tTA,tAA,sA,tTuA);
        row("GBFS",tTG,tAG,sG,tTuG);
    }
}
