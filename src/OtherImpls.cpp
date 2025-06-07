#include "Helper.h"

using namespace ChessCpp;

bool operator<(Square lhs, Square rhs) {
	return static_cast<int>(lhs) < static_cast<int>(rhs);
}

int ChessCpp::rank(int square) {
	return square >> 4;
}

int ChessCpp::file(int square) {
	return square & 0xf;
}

Square ChessCpp::stringToSquare(const std::string& squareStr) {
	return static_cast<Square>(('8' - squareStr[1]) * 8 + (squareStr[0] - 'a'));
}

std::string ChessCpp::squareToString(const Square& sq) {
	return SQUARES[static_cast<unsigned int>(sq)];
}

Square ChessCpp::algebraic(int square) {
	return ChessCpp::stringToSquare(
		std::string(1, static_cast<char>(std::string("abcdefgh")[ChessCpp::file(square)])) +
		std::string(1, static_cast<char>(std::string("87654321")[ChessCpp::rank(square)]))
	);
}

std::pair<bool, std::string> ChessCpp::validateFen(std::string fen) {
	const std::vector<std::string> tokens = Helper::splitWithRegex(fen, R"(\s+)");
	if (tokens.size() != 6) {
		return { false, "Invalid FEN" };
	}

	const int moveNumber = std::stoi(tokens[5]);
	if (std::isnan(moveNumber) || moveNumber <= 0) {
		return { false, "Invalid FEN" };
	}

	const int halfMoves = std::stoi(tokens[4]);
	if (std::isnan(halfMoves) || halfMoves < 0) {
		return { false, "Invalid FEN" };
	}

	if (!std::regex_search(tokens[3], std::regex("^(-|[abcdefgh][36])$"))) {
		return { false, "Invalid FEN" };
	}

	if (std::regex_search(tokens[2], std::regex("[^kKqQ-]"))) {
		return { false, "Invalid FEN" };
	}

	if (!std::regex_search(tokens[1], std::regex("^(w|b)$"))) {
		return { false, "Invalid FEN" };
	}

	const std::vector<std::string> rows = Helper::split(tokens[0], '/');
	if (rows.size() != 8) {
		return { false, "Invalid FEN" };
	}

	for (int i = 0; i < static_cast<int>(rows.size()); i++) {
		int sumFields = 0;
		bool previousWasNumber = false;

		for (int k = 0; k < static_cast<int>(rows[i].size()); k++) {
			if (Helper::isDigit(std::string(1, rows[i][k]))) {
				if (previousWasNumber) {
					return { false, "Invalid FEN" };
				}
				sumFields += std::stoi(std::string(1, rows[i][k]));
				previousWasNumber = true;
			}
			else {
				if (!std::regex_search(std::string(1, rows[i][k]), std::regex("^[prnbqkPRNBQK]"))) {
					return { false, "Invalid FEN" };
				}
				sumFields += 1;
				previousWasNumber = false;
			}
		}
		if (sumFields != 8) {
			return { false, "Invalid FEN" };
		}
	}
	if (tokens[3][0] == '-') {}
	else if (
		(tokens[3][1] == '3' && tokens[1] == "w") ||
		(tokens[3][1] == '6' && tokens[1] == "b")
		) {
		return { false, "Invalid FEN" };
	}

	const std::vector<std::tuple<std::string, std::regex>> kings = {
		{"white", std::regex("K")}, {"black", std::regex("k")}
	};

	for (const auto& kI : kings) {
		if (!std::regex_search(tokens[0], std::get<1>(kI))) {
			return { false, "Invalid FEN" };
		}
		std::smatch matches;
		if (std::regex_search(tokens[0], matches, std::get<1>(kI))) {
			if (matches.size() > 1)
				return { false, "Invalid FEN" };
		}
	}
	std::string cnt = rows[0] + rows[7];
	bool f = false;
	/*std::any_of(cnt.begin(), cnt.end(), [&](char c) -> bool {
	return std::toupper(c) == 'P';
});*/
	for (auto& c : cnt) {
		if (c == 'P') {
			f = true;
		}
	}

	if (f)
		return { false, "Invalid FEN" };

	return { true, "" };
}