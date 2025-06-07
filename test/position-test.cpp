#include "../include/chesscpp"
#include <iostream>

int main() {
    std::string optFEN = "r3kbnr/p3p1pp/2n1bp1B/2p5/P1B1P3/2p5/1P1N1PPP/RN1K3R b kq - 1 11";
    std::string optFEN1 = "1r2k1nr/pb4pp/5b2/2p2nP1/P4p1B/1K2pB2/5PNP/R4RN1 b k - 6 31";
    ChessCpp::Chess game = ChessCpp::Chess("8/P7/8/8/8/5k2/K7/8 w - - 0 1");
    int hmf = 0;
    std::vector<std::string> moveHistory;

    while (!game.isGameOver()) {
        system("cls");
        std::cout << game.ascii() + '\n';
        std::cout << "Make your move: ";
        std::string move;
        std::cin >> move;
        if (move == "undo") { if (hmf > 1) { game.undo(); hmf--; } continue; }
        if (move == "next") { if (hmf > 0 && !moveHistory.empty()) { game.makeMove(moveHistory[hmf - 1]); hmf++; } continue; }
        auto moves = game.getMoves();
        try {
            auto m = game.makeMove(move);
            moveHistory.push_back(move);
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