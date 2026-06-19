#include "benchmark.hpp"
#include "debug.hpp"

int main(int argc, char** argv) {
    if (argc>1 && std::string(argv[1])=="debug") { runDebug(); return 0; }
    std::cout<<"--- Benchmark Pipe Solver (A* vs GBFS, myopic, no-rotation) ---\n\n";
    runBenchmark();
    return 0;
}
