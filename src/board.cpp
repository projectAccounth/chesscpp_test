#include "../include/chess.h"
#include <cmath>
#include <string>
#include <vector>
#include <sstream>
#include <cctype>
#include <regex>
#include <tuple>

std::vector<std::string> split(const std::string& str, char delimiter) {
	std::vector<std::string> tokens;
	std::stringstream ss(str);
	std::string token;

	while (std::getline(ss, token, delimiter)) {
		tokens.push_back(token);
	}

	return tokens;
}

std::vector<std::string> splitWithRegex(const std::string& str, const std::string& pattern) {
	std::regex re(pattern);
	std::sregex_token_iterator it(str.begin(), str.end(), re, -1); // -1 means "split by the regex"
	std::sregex_token_iterator end;

	return { it, end };
}

std::string join(const std::vector<std::string>& elements, const std::string& delimiter) {
	std::string result;

	for (size_t i = 0; i < elements.size(); ++i) {
		result += elements[i];
		if (i < elements.size() - 1) {
			result += delimiter;
		}
	}

	return result;
}

bool isDigit(std::string c) {
	std::string str = "0123456789";
	return str.find(c) != std::string::npos;
}

std::pair<bool, std::string> validateFen(std::string fen) {
	const std::vector<std::string> tokens = splitWithRegex(fen, "\\s+");
	if (tokens.size() != 6) {
		return { false, "Invalid FEN" };
	}

	const int moveNumber = std::stoi(tokens[5]);
	if (std::isnan(moveNumber) || moveNumber <= 0) {
		return { false, "Invalid FEN" };
	}

	const int halfMoves = std::stoi(tokens[4]);
	if (std::isnan(halfMoves) || halfMoves <= 0) {
		return { false, "Invalid FEN" };
	}

	if (!std::regex_search(tokens[3], std::regex("^(-|[abcdefgh][36])$"))) {
		return { false, "Invalid FEN" };
	}

	if (!std::regex_search(tokens[2], std::regex("[^kKqQ-]"))) {
		return { false, "Invalid FEN" };
	}

	if (!std::regex_search(tokens[1], std::regex("^(w|b)$"))) {
		return { false, "Invalid FEN" };
	}

	const std::vector<std::string> rows = split(tokens[0], '/');
	if (rows.size() != 8) {
		return { false, "Invalid FEN" };
	}

	for (int i = 0; i < rows.size(); i++) {
		int sumFields = 0;
		bool previousWasNumber = false;

		for (int k = 0; k < rows[i].size(); k++) {
			if (isDigit(std::string(1, rows[i][k]))) {
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

	if (
		tokens[3][1] == '3' && tokens[1] == "w" ||
		tokens[3][1] == '6' && tokens[1] == "b"
		) {
		return { false, "Invalid FEN" };
	}

	const std::vector<std::tuple<std::string, std::regex>> kings = {
		{"white", std::regex("K")}, {"black", std::regex("k")}
	};

	for (const auto& kI : kings) {
		if (!std::regex_search(std::get<0>(kI), std::get<1>(kI))) {
			return { false, "Invalid FEN" };
		}
		auto b = std::sregex_iterator(std::get<0>(kI).begin(), std::get<0>(kI).end(), std::get<1>(kI));
		auto e = std::sregex_iterator();
		if ((std::regex_search(std::get<0>(kI), std::get<1>(kI)) || std::distance(b, e))) {
			return { false, "Invalid FEN" };
		}
	}

	bool f = std::any_of((rows[0] + rows[7]).begin(), (rows[0] + rows[7]).end(), [](char c) {
		return std::toupper(c) == 'P';
	});

	if (f)
		return { false, "Invalid FEN" };

	return { true, "" };
}

