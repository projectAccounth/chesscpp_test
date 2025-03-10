#include "chesscpp.h"
#include <iostream>

int main() {
    std::string optFEN = "r3kbnr/p3p1pp/2n1bp1B/2p5/P1B1P3/2p5/1P1N1PPP/RN1K3R b kq - 1 11";
    std::string optFEN1 = "1r2k1nr/pb4pp/5b2/2p2nP1/P4p1B/1K2pB2/5PNP/R4RN1 b k - 6 31";
	Chess game = Chess();
    int hmf = 0;
    std::vector<std::string> moveHistory;

    while (!game.isGameOver()) {
        system("cls");
        std::cout << game.ascii() + '\n';
        std::cout << "Make your move: ";
        std::string move;
        std::cin >> move;
        if (move == "undo") { if (hmf > 1) { game.undo(); hmf--; } continue; }
        if (move == "next") { if (hmf > 0 && !moveHistory.empty()) { game.cmove(moveHistory[hmf - 1]); hmf++; } continue; }
        try {
            auto m = game.cmove(move);
            moveHistory.push_back(move);
            auto moves = game.moves();
            hmf++;
        }
        catch (const std::exception& e) {
            std::cout << std::flush;
            std::cout << e.what() << '\n';
            system("pause");
            continue;
        }
    }

	std::cout << game.pgn() << '\n';

	return 0;
}