#include "../include/chess.h"
#include <regex>
#include <cctype>
#include <sstream>
#include <cmath>
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

int rank(int square) {
	return square >> 4;
}

int file(int square) {
	return square & 0xf;
}

bool isDigit(std::string c) {
	std::string str = "0123456789";
	return str.find(c) != std::string::npos;
}

square stringToSquare(const std::string& squareStr) {
	int f = squareStr[0] - 'a';
	int r = '8' - squareStr[1];
	return static_cast<square>(r * 8 + f);
}

std::string squareToString(square sq) {
	static std::vector<std::string> square_names = {
		"a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
		"a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
		"a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
		"a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
		"a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
		"a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
		"a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
		"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1"
	};

	return square_names[static_cast<int>(sq)];
}

square algebraic(int square) {
	int f = file(square);
	int r = rank(square);
	return stringToSquare(std::string(1, "abcdefgh"[f]) + std::string(1, "87654321"[r]));
}

color swapColor(color color) {
	return color == WHITE ? BLACK : WHITE;
}

// Used to identify ambigous moves
std::string getDisambiguator(internalMove move, std::vector<internalMove> moves) {
	const square from = static_cast<square>(move.from);
	const square to = static_cast<square>(move.to);
	const pieceSymbol piece = move.piece;

	int ambiguities = 0;
	int sameRank = 0;
	int sameFile = 0;

	for (int i = 0; i < moves.size(); i++) {
		const square ambigFrom = static_cast<square>(moves[i].from);
		const square ambigTo = static_cast<square>(moves[i].to);
		const pieceSymbol ambigPiece = moves[i].piece;

		if (piece == ambigPiece && from != ambigFrom && to != ambigTo) {
			ambiguities++;

			if (rank(static_cast<int>(from)) == rank(static_cast<int>(ambigFrom))) {
				sameRank++;
			}
			if (file(static_cast<int>(from)) == file(static_cast<int>(ambigFrom))) {
				sameFile++;
			}
		}
	}
	if (ambiguities > 0) {
		if (sameRank > 0 && sameFile > 0) {
			return squareToString(algebraic(static_cast<int>(from)));
		}
		else if (sameFile > 0) {
			return std::string(squareToString(algebraic(static_cast<int>(from))), 1);
		}
		else {
			return std::string(squareToString(algebraic(static_cast<int>(from))), 0);
		}
	}
	return "";
}

void addMove(std::vector<internalMove> moves, color color, int from, int to, pieceSymbol piece, std::optional<pieceSymbol> captured, int flags = BITS.at("NORMAL")) {
	const int r = rank(to);
	if (piece == PAWN && (r == RANK_1 || r == RANK_8)) {
		for (int i = 0; i < PROMOTIONS.size(); i++) {
			const pieceSymbol promotion = PROMOTIONS[i];
			moves.push_back({
				color,
				from,
				to,
				piece,
				captured,
				promotion,
				flags |= static_cast<int>(BITS.at("PROMOTION"))
				});
		}
	}
	else {
		moves.push_back({
			color,
			from,
			to,
			piece,
			captured,
			std::nullopt,
			flags |= static_cast<int>(BITS.at("PROMOTION"))
			});
	}
}

std::map<char, pieceSymbol> strPchrs = {
	{'p', pieceSymbol::p}, {'r', pieceSymbol::r}, {'b', pieceSymbol::b}, {'n', pieceSymbol::n}, {'q', pieceSymbol::q}, {'k', pieceSymbol::k}
};

std::optional<pieceSymbol> inferPieceType(std::string san) {
	char pieceType = san.at(0);
	if (pieceType >= 'a' && pieceType <= 'h') {
		std::regex pattern("[a-h]\d.*[a-h]\d");
		if (std::regex_search(san, pattern)) {
			return std::nullopt;
		}
		return PAWN;
	}
	pieceType = std::tolower(pieceType);
	if (pieceType == 'o') {
		return KING;
	}
	return strPchrs.at(pieceType);
}

std::string strippedSan(std::string move) {
	return std::regex_replace(std::regex_replace(move, std::regex("="), ""), std::regex("[+#] ? [? !] * $"), "");
}
std::string trimFen(std::string fen) {
	std::vector<std::string> stpld = split(fen, ' ');
	return join(std::vector<std::string>(stpld.begin(), stpld.begin() + std::min<size_t>(4, stpld.size())), " ");
}

