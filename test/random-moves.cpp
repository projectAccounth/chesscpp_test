#include "chesscpp.h"
#include <random>
#include <thread>
#include <chrono>
#include <iostream>

int main() {
	std::random_device rd;
	std::mt19937 rnd(rd());

	Chess game = Chess();

	while (!game.isGameOver()) {
		system("cls");
		std::cout << game.ascii() << '\n';
		auto moves = game.moves();
		std::uniform_int_distribution<int> d(0, moves.size() - 1);
		auto pickedMove = moves[d(rnd)];
		game.cmove(pickedMove);
		system("cls");
		std::cout << game.ascii() << '\n';
	}

	std::cout << game.pgn() << '\n';
	system("pause");

	return 0;
}