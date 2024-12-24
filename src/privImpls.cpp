#include "privImpls.h"

std::string trim(const std::string& str) {
	auto start = std::find_if_not(str.begin(), str.end(), [](unsigned char ch) {
		return std::isspace(ch);
		});
	auto end = std::find_if_not(str.rbegin(), str.rend(), [](unsigned char ch) {
		return std::isspace(ch);
		}).base();

	return (start < end ? std::string(start, end) : std::string());
}

bool operator<(square lhs, square rhs) {
	return static_cast<int>(lhs) < static_cast<int>(rhs);
}

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

int rank(int square) {
	return square >> 4;
}

int file(int square) {
	return square & 0xf;
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

	return square_names[static_cast<unsigned int>(sq)];
}

square algebraic(int square) {
	int f = file(square);
	int r = rank(square);
	return stringToSquare(std::string(1, static_cast<char>(std::string("abcdefgh").at(f))) + std::string(1, static_cast<char>(std::string("87654321").at(r))));
}

color swapColor(color color) {
	return color == WHITE ? BLACK : WHITE;
}

std::string getDisambiguator(internalMove move, std::vector<internalMove> moves) {
	const square from = algebraic(move.from);
	const square to = algebraic(move.to);
	const pieceSymbol p = move.piece;

	int ambiguities = 0;
	int sameRank = 0;
	int sameFile = 0;

	for (int i = 0; i < static_cast<int>(moves.size()); i++) {
		const square ambigFrom = algebraic(moves[i].from);
		const square ambigTo = algebraic(moves[i].to);
		const pieceSymbol ambigPiece = moves[i].piece;

		if (!(p == ambigPiece && from != ambigFrom && to == ambigTo)) continue;

		ambiguities++;

		if (rank(Ox88.at(from)) == rank(Ox88.at(ambigFrom))) {
			sameRank++;
		}
		if (file(Ox88.at(from)) == file(Ox88.at(ambigFrom))) {
			sameFile++;
		}
	}
	if (ambiguities <= 0) return "";

	if (sameRank > 0 && sameFile > 0) {
		return squareToString(from);
	}
	else if (sameFile > 0) {
		return std::string(1, squareToString(from).at(1));
	}
	else {
		return std::string(1, squareToString(from).at(0));
	}
}

void addMove(std::vector<internalMove>& moves, color color, int from, int to, pieceSymbol p, std::optional<pieceSymbol> captured, int flags) {
	const int r = rank(to);
	if (p == PAWN && (r == RANK_1 || r == RANK_8)) {
		for (int i = 0; i < static_cast<int>(PROMOTIONS.size()); i++) {
			const pieceSymbol promotion = PROMOTIONS[i];
			moves.push_back({
				color,
				from,
				to,
				p,
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
			p,
			captured,
			std::nullopt,
			flags |= static_cast<int>(BITS.at("PROMOTION"))
			});
	}
}

std::string replaceSubstring(const std::string& str, const std::string& from, const std::string& to) {
	size_t startPos = str.find(from);
	if (startPos == std::string::npos) {
		return str;
	}
	return str.substr(0, startPos) + to + str.substr(startPos + from.length());
}

std::pair<bool, std::string> validateFen(std::string fen) {
	const std::vector<std::string> tokens = splitWithRegex(fen, R"(\s+)");
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

	const std::vector<std::string> rows = split(tokens[0], '/');
	if (rows.size() != 8) {
		return { false, "Invalid FEN" };
	}

	for (int i = 0; i < static_cast<int>(rows.size()); i++) {
		int sumFields = 0;
		bool previousWasNumber = false;

		for (int k = 0; k < static_cast<int>(rows[i].size()); k++) {
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

std::optional<pieceSymbol> inferPieceType(std::string san) {
	char pieceType = san.at(0);
	if (pieceType >= 'a' && pieceType <= 'h') {
		std::regex pattern("[a-h]\\d.*[a-h]\\d");
		if (std::regex_search(san, pattern)) {
			return std::nullopt;
		}
		return PAWN;
	}
	pieceType = static_cast<char>(std::tolower(pieceType));
	if (pieceType == 'o') {
		return KING;
	}
	return strPchrs.at(pieceType);
}

std::string strippedSan(std::string move) {
	return std::regex_replace(std::regex_replace(move, std::regex("="), ""), std::regex("[+#]?[?!]*$"), "");
}

std::string trimFen(std::string fen) {
	std::vector<std::string> stpld = split(fen, ' ');
	return join(std::vector<std::string>(stpld.begin(), stpld.begin() + std::min<size_t>(4, stpld.size())), " ");
}
