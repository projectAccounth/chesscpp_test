#include <iostream>
#include <cstdlib>
#include "chesscpp.h"
#include <chrono>

using namespace std::literals::chrono_literals;

int main() {
    std::string optFEN = "8/7P/k7/8/8/5K2/8/8 w - - 0 1";
    std::string optFEN1 = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1";
    Chess game = Chess();
    std::vector<std::string> moveHistory;
    int hmf = 1;

    int analyzeDepth = 5;

    std::string initialFEN = game.fen();

    for (int i = 1; i <= analyzeDepth; i++) {
        const auto startTime = std::chrono::high_resolution_clock::now();
        int perftResult = game.perft(i);
        const auto endTime = std::chrono::high_resolution_clock::now();
        auto elapsed = endTime - startTime;
        double timeInSec = elapsed / 1.0s;

        int nodePerSec = static_cast<double>(perftResult / timeInSec);

        std::cout << "depth " << i
            << " time " << elapsed / 1ms
            << " nodes " << perftResult
            << " nps " << nodePerSec
            << " fen " << initialFEN
            << '\n';
    }
    system("pause");
    return 0;
}
