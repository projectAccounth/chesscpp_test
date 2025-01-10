#include <iostream>
#include <cstdlib>
#include "chesscpp.h"
#include <chrono>

using namespace std::literals::chrono_literals;

int main() {
    std::string optFEN = "8/7P/k7/8/8/5K2/8/8 w - - 0 1";
    std::string optFEN1 = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1";
    std::string optFEN2 = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
    std::string optFEN3 = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8";
    std::string optFEN4 = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ";
    Chess game = Chess(optFEN3);
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
