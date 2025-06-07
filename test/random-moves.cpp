#include "../include/chesscpp"
#include <random>
#include <thread>
#include <chrono>
#include <iostream>

int main() {
	std::random_device rd;
	std::mt19937 rnd(rd());

	ChessCpp::Chess game = ChessCpp::Chess("8/P7/8/8/8/5k2/2K5/8 w - - 0 1");
	auto ms = game.getMoves(true);

	while (!game.isCheckmate()) {
		system("cls");
		std::cout << game.ascii() << '\n';
		auto moves = game.getMoves(true);
		std::uniform_int_distribution<int> d(0, moves.size() - 1);
		auto pickedMove = moves[d(rnd)];
		game.makeMove(pickedMove);
		system("cls");
		std::cout << game.ascii() << '\n';
	}

	std::cout << game.pgn() << '\n';
	system("pause");

	return 0;
}