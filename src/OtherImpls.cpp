#include "Helper.h"
#include <set>

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

bool isValidPiecePlacement(const std::string& placement) { 
	const std::vector<std::string> rows = Helper::split(placement, '/'); 
	if (rows.size() != 8) return false;  
	for (size_t i = 0; i < rows.size(); ++i) { 
		const std::string& row = rows[i]; 
		int sum = 0; 
		bool lastWasDigit = false;  
		for (char c : row) { 
			if (std::isdigit(c)) { 
				if (lastWasDigit) return false; 
				sum += c - '0'; 
				lastWasDigit = true; 
			} 
			else if (std::string("rnbqkpRNBQKP").find(c) != std::string::npos) { 
				sum += 1; lastWasDigit = false; 
			} 
			else { 
				return false; 
			} 
		} if (sum != 8) 
			return false; 
	}  
	// Disallow pawns on first or last rank 
	for (char c : rows[0] + rows[7]) { 
		if (c == 'P' || c == 'p') return false; 
	}  
	return true;
}

bool isValidActiveColor(const std::string& color) { 
	return color == "w" || color == "b";
}

bool isValidCastlingRights(const std::string& rights) { 
	if (rights == "-") return true;
	static const std::string valid = "KQkq";
	std::set<char> seen;
	for (char c : rights) { 
		if (valid.find(c) == std::string::npos) return false; 
		if (!seen.insert(c).second) return false; // duplicate 
	} 
	return true;
}

bool isValidEnPassant(const std::string& ep, const std::string& turn) { 
	if (ep == "-") return true;

	if (!std::regex_match(ep, std::regex("^[a-h][36]$"))) return false;
	char rank = ep[1];
	if ((rank == '3' && turn == "w") || (rank == '6' && turn == "b")) return false;

	return true;
}

bool isValidIntegerField(const std::string& s, bool mustBePositive) {
	try {
		int value = std::stoi(s);
		return mustBePositive ? value > 0 : value >= 0;
	} catch (...) {
		return false;
	}
}

bool hasExactlyOneKing(const std::string& board) {
	int whiteKings = std::count(board.begin(), board.end(), 'K');
	int blackKings = std::count(board.begin(), board.end(), 'k');

	return whiteKings == 1 && blackKings == 1;
}

std::pair<bool, std::string> ChessCpp::validateFen(std::string fen) { 	
	const std::vector<std::string> tokens = Helper::splitWithRegex(fen, R"(\s+)");
	if (tokens.size() != 6) {
		return { false, "Invalid FEN: Expected 6 fields" };
	}

	if (!isValidPiecePlacement(tokens[0])) return { false, "Invalid FEN: Bad piece placement" };
	if (!isValidActiveColor(tokens[1])) return { false, "Invalid FEN: Bad active color" };
	if (!hasExactlyOneKing(tokens[0])) return { false, "Invalid FEN: King count invalid" };
	if (!isValidCastlingRights(tokens[2])) return { false, "Invalid FEN: Bad castling rights" };
	if (!isValidEnPassant(tokens[3], tokens[1])) return { false, "Invalid FEN: Bad en passant square" };
	if (!isValidIntegerField(tokens[4], false)) return { false, "Invalid FEN: Bad halfmove clock" };
	if (!isValidIntegerField(tokens[5], true)) return { false, "Invalid FEN: Bad fullmove number" };

	return { true, "Successful validation" };
}
