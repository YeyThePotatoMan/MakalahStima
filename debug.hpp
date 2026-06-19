#pragma once

#include <iostream>
#include <random>
#include <climits>
#include "engine.hpp"
#include "path.hpp"
#include "solver.hpp"

inline void runDebug() {
    const int n=5; const unsigned seed=42;
    std::mt19937 rng(seed);
    std::vector<PathStep> tp = buildStaircasePath({0,0},{n-1,n-1});
    PuzzleEngine engine(n, rng, n*n*4, tp);
    std::set<std::pair<int,int>> pathPosSet;
    for (auto& s : tp) pathPosSet.insert(s.pos);

    std::cerr<<"=== DEBUG (5x5, seed=42) ===\nTarget path:\n";
    for(auto& s:tp) std::cerr<<"  ("<<s.pos.first<<","<<s.pos.second<<") in="<<DIR_NAME[s.inDir]<<" out="<<DIR_NAME[s.outDir]<<"\n";
    std::cerr<<"\nGround truth (0=SV,1=SH,2=UR,3=RD,4=DL,5=LU):\n";
    for(int i=0;i<n;i++){for(int j=0;j<n;j++) std::cerr<<(int)engine.world.trueType[i][j]<<" ";std::cerr<<"\n";}
    std::cerr<<"flowIdx="<<engine.flowIdx<<" solved="<<engine.solved<<"\n\n";

    int iter=0;
    while(!engine.solved&&!engine.isOutOfTurns()&&iter<200){
        iter++;
        auto flowPos=engine.currentFlowPos(); int fIdx=engine.currentFlowIdx();
        ActionCandidate best; double bestF=1e18; int bestPI=INT_MAX; bool hasAny=false;

        std::vector<std::pair<int,int>> knp;
        for(int r=0;r<n;r++) for(int c=0;c<n;c++)
            if(!pathPosSet.count({r,c})&&engine.knowledge.known[r][c]&&!engine.knowledge.locked[r][c])
                knp.push_back({r,c});

        std::pair<int,int> fs={-1,-1};
        for(int i=1;i<(int)tp.size();i++){
            auto&s=tp[i];auto[r,c]=s.pos;
            if(engine.knowledge.locked[r][c]) continue;
            if(!engine.knowledge.known[r][c]){fs=s.pos;break;}
            if(!pipeMatchesRequirement(engine.knowledge.belief[r][c],s.inDir,s.outDir)){
                bool has=false;
                for(auto&src:knp) if(pipeMatchesRequirement(engine.knowledge.belief[src.first][src.second],s.inDir,s.outDir)){has=true;break;}
                if(!has){fs=s.pos;break;}
            }
        }

        auto tryU=[&](ActionCandidate c,double f,int pi){if(f<bestF||(f==bestF&&pi<bestPI)){bestF=f;bestPI=pi;best=c;hasAny=true;}};

        for(int i=1;i<(int)tp.size();i++){
            auto&s=tp[i];auto[r,c]=s.pos;
            if(engine.knowledge.known[r][c]||engine.knowledge.locked[r][c]) continue;
            int g=i-fIdx; int h=manhattan(s.pos,flowPos);
            double f=g+h;
            tryU({ActionCandidate::Kind::QUERY_PATH,s.pos,{-1,-1},f,i},f,i);
        }
        for(int i=1;i<(int)tp.size();i++){
            auto&s=tp[i];auto[tr,tc]=s.pos;
            if(!engine.knowledge.known[tr][tc]||engine.knowledge.locked[tr][tc]) continue;
            if(pipeMatchesRequirement(engine.knowledge.belief[tr][tc],s.inDir,s.outDir)) continue;
            for(auto&src:knp){
                PipeType st=engine.knowledge.belief[src.first][src.second];
                if(!pipeMatchesRequirement(st,s.inDir,s.outDir)) continue;
                int g=i-fIdx; int h=manhattan(s.pos,flowPos);
                double f=g+h;
                tryU({ActionCandidate::Kind::SWAP,s.pos,src,f,i},f,i);
            }
        }
        if(fs.first!=-1){
            for(int r=0;r<n;r++) for(int c=0;c<n;c++){
                if(pathPosSet.count({r,c})||engine.knowledge.known[r][c]||engine.knowledge.locked[r][c]) continue;
                double f=manhattan({r,c},fs)+manhattan(fs,flowPos)+1000.0;
                tryU({ActionCandidate::Kind::QUERY_NONPATH,{r,c},{-1,-1},f,INT_MAX-1},f,INT_MAX-1);
            }
        }

        if(!hasAny){std::cerr<<"iter="<<iter<<": NO CANDIDATES. flow="<<engine.flowIdx<<"/"<<tp.size()-1<<"\n";break;}

        std::string kStr;
        if(best.kind==ActionCandidate::Kind::QUERY_PATH) kStr="QUERY_PATH";
        else if(best.kind==ActionCandidate::Kind::QUERY_NONPATH) kStr="QUERY_NP";
        else kStr="SWAP";

        if(best.kind==ActionCandidate::Kind::SWAP){
            std::cerr<<"iter="<<iter<<"  "<<kStr<<"("<<best.posA.first<<","<<best.posA.second<<" <-> "<<best.posB.first<<","<<best.posB.second<<")  flow="<<engine.flowIdx<<" turns="<<engine.turnsUsed<<"\n";
            engine.swapTile(best.posA,best.posB);
        } else {
            std::cerr<<"iter="<<iter<<"  "<<kStr<<"("<<best.posA.first<<","<<best.posA.second<<")  flow="<<engine.flowIdx<<" turns="<<engine.turnsUsed<<"\n";
            engine.queryTile(best.posA.first,best.posA.second);
        }
    }
    std::cerr<<"\n=== HASIL AKHIR ===\n";
    std::cerr<<"solved="<<engine.solved<<" flowIdx="<<engine.flowIdx<<"/"<<(tp.size()-1)<<" turnsUsed="<<engine.turnsUsed<<"\n";
}
