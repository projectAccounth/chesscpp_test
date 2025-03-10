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
		game.cmove(moves[d(rnd)], false);
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		system("cls");
		std::cout << game.ascii() << '\n';
	}

	std::cout << game.pgn();

	return 0;
}