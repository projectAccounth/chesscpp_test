#include "pimpl.h"

// Contains function definitions for general functions (except cmove for moving, it's in the moveHandling file)

using namespace privs;

void Chess::reset() {
	load(DEFAULT_POSITION);
}

bool Piece::isDefault() const {
	return type == PNONE || color == Color::NO_COLOR;
}

Piece::Piece() : type(PNONE), color(Color::NO_COLOR) {}
Piece::Piece(Color c, PieceSymbol p) : type(p), color(c) {}

Piece::operator bool() const {
	return !isDefault();
}

std::string Chess::pgn(char newline, int maxWidth) {
	std::vector<std::string> result;
	bool headerExists = false;
	for (const auto& k : chImpl->_header) {
		result.push_back(std::string('[' + k.first + " \"" + chImpl->_header.at(k.first) + "\"]" + newline));
		headerExists = true;
	}

	if (headerExists && chImpl->_history.size() != 0) {
		result.push_back(std::string(1, static_cast<char>(newline)));
	}
	const auto appendComment = [&](std::string moveString) -> std::string {
		if (chImpl->_comments.count(fen()) == 0) return moveString;
		const std::string comment = chImpl->_comments[fen()];
		if (comment != "") {
			const std::string delimiter = moveString.size() > 0 ? " " : "";
			moveString += delimiter + "{" + comment + "}";
		}
		return moveString;
		};
	std::vector<InternalMove> reservedHistory;
	while (chImpl->_history.size() > 0) {
		reservedHistory.push_back(chImpl->_undoMove());
	}

	std::vector<std::string> moves;
	std::string moveString = "";

	if (reservedHistory.size() == 0) {
		moves.push_back(appendComment(""));
	}
	while (reservedHistory.size() > 0) {
		moveString = appendComment(moveString);
		InternalMove m = reservedHistory.back();
		reservedHistory.pop_back();

		if (!m) break;

		if (chImpl->_history.size() == 0 && m.color == Color::b) {
			const std::string prefix = std::to_string(chImpl->_moveNumber) + ". ...";
			moveString = !moveString.empty() ? moveString + " " + prefix : prefix;
		}
		else if (m.color == Color::w) {
			if (!moveString.empty()) {
				moves.push_back(moveString);
			}
			moveString = std::to_string(chImpl->_moveNumber) + ".";
		}
		moveString = moveString + " " + chImpl->_moveToSan(m, chImpl->_moves(true));
		chImpl->_makeMove(m);
	}
	if (!moveString.empty()) {
		moves.push_back(appendComment(moveString));
	}
	if (chImpl->_header.count("Result") > 0) {
		moves.push_back(chImpl->_header.at("Result"));
	}
	if (maxWidth == 0) {
		auto resultStr = join(result, "").append(join(moves, " "));
		return resultStr;
	}

	const auto strip = [&]() -> bool {
		if (result.size() > 0 && result.back() == " ") {
			result.pop_back();
			return true;
		}
		return false;
		};
	const auto wrapComment = [&](int width, std::string move) -> int {
		for (const auto& token : split(move, ' ')) {
			if (token.empty()) {
				continue;
			}
			if (width + static_cast<int>(token.size()) > maxWidth) {
				while (strip()) {
					width--;
				}
				result.push_back(std::string(1, static_cast<char>(newline)));
				width = 0;
			}
			result.push_back(token);
			width += static_cast<int>(token.size());
			result.push_back(" ");
			width++;
		}
		if (strip()) {
			width--;
		}
		return width;
		};
	int currentWidth = 0;
	for (int i = 0; i < static_cast<int>(moves.size()); i++) {
		if (currentWidth + static_cast<int>(moves[i].size()) > maxWidth) {
			if (moves[i].find('{') != std::string::npos) {
				currentWidth = wrapComment(currentWidth, moves[i]);
				continue;
			}
		}
		if (currentWidth + static_cast<int>(moves[i].size()) > maxWidth && i != 0) {
			if (result.back() == " ") {
				result.pop_back();
			}
			result.push_back(std::string(1, static_cast<char>(newline)));
			currentWidth = 0;
		}
		else if (i != 0) {
			result.push_back(" ");
			currentWidth++;
		}
		result.push_back(moves[i]);
		currentWidth += static_cast<int>(moves[i].size());
	}
	return join(result, "");
}

void Chess::loadPgn(std::string pgn, bool strict, std::string newlineChar) {
	auto mask = [&](std::string str) -> std::string {
		return std::regex_replace(str, std::regex(R"(\\)"), R"(\)");
		};
	auto parsePgnHeader = [&](std::string header) -> std::map<std::string, std::string> {
		std::map<std::string, std::string> headerObj = {};
		std::vector<std::string> headers = splitWithRegex(header, newlineChar);
		std::string key = "";
		std::string value = "";

		for (int i = 0; i < static_cast<int>(headers.size()); i++) {
			const std::regex reg = std::regex(R"(^\s*\[\s*([A-Za-z]+)\s*\"(.*)\"\s*\]\s*$)");
			key = std::regex_replace(headers[i], reg, "$1");
			value = std::regex_replace(headers[i], reg, "$2");
			if (trim(key).size() > 0) {
				headerObj.at(key) = value;
			}
		}
		return headerObj;
		};
	pgn = trim(pgn);
	std::string hdrRg(
		R"(^(\[((?:)" +
		mask(newlineChar) +
		R"()|.)*\]))" +
		R"(((?:\s*)" +
		mask(newlineChar) +
		R"(){2}|(?:\s*)" +
		mask(newlineChar) +
		")*$)"
	);
	std::regex headerRegex(hdrRg);
	std::smatch matches;
	std::string headerString = std::regex_search(pgn, matches, headerRegex) ? (matches.size() >= 2 ? std::string(matches[1]) : "") : "";

	reset();

	std::map<std::string, std::string> headers = parsePgnHeader(headerString);
	std::string pfen = "";

	for (const auto& val : headers) {
		std::string lowerStr;
		for (char c : val.first) {
			lowerStr += static_cast<char>(std::tolower(c));
		}
		if (lowerStr == "fen") {
			pfen = headers.at(val.first);
		}
		header({ val.first, headers.at(val.first) });
	}

	if (!strict) {
		if (!pfen.empty()) {
			load(pfen, false, true);
		}
	}
	else {
		if (headers.at("SetUp") == "1") {
			if (!(headers.count("FEN") > 0)) {
				throw std::runtime_error("Invalid PGN: FEN tag must be supplied with SetUp tag.");
			}
		}
		load(headers.at("FEN"), false, true);
	}
	auto toHex = [](std::string s) -> std::string {
		std::stringstream hexStream;
		auto encodeURIComponent = [](char c) {
			std::string result;

			if ((c & 0x80) == 0) {
				// If the character is a single byte (ASCII), just add it
				result += c;
			}
			else {
				// Otherwise, handle multi-byte characters (simplified version)
				result += "%" + std::to_string(static_cast<int>(c));
			}

			return result;
			};
		for (char c : s) {
			if (static_cast<unsigned char>(c) < 128) {
				// For characters with code < 128, convert to hexadecimal directly
				hexStream << std::hex << std::setw(2) << std::setfill('0')
					<< static_cast<int>(c);
			}
			else {
				// For characters with code >= 128, encode them and convert to hex
				std::string encoded = "%" + encodeURIComponent(c);
				for (char encodedChar : encoded) {
					hexStream << std::hex << std::setw(2) << std::setfill('0')
						<< static_cast<int>(encodedChar);
				}
			}
		}

		return hexStream.str();
		};
	auto fromHex = [](std::string s) -> std::string {
		std::stringstream hexStream;

		for (char c : s) {
			hexStream << "%" << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(c);
		}

		return hexStream.str();
		};
	auto encodeComment = [&](std::string s) -> std::string {
		std::string modified = s;
		std::replace(modified.begin(), modified.end(), '\n', ' ');

		if (modified.length() > 2) {
			modified = modified.substr(1, modified.length() - 2);
		}

		return "{" + toHex(modified) + "}";
		};
	auto decodeComment = [&](std::string s) {
		if (s.front() == '{' && s.back() == '}') {
			return fromHex(s.substr(1, s.length() - 2));
		}
		return std::string("");
		};
	std::string ms = [&]() -> std::string {
		std::string processed = replaceSubstring(pgn, headerString, "");
		std::regex pattern(std::string(R"(({[^}]*})+?|;([^\r?\n]*))"), std::regex_constants::basic); // FIX THIS

		std::string result;
		std::smatch match;
		std::string::const_iterator searchStart(processed.cbegin());

		while (std::regex_search(searchStart, processed.cend(), match, pattern)) {
			result += processed.substr(0, match.position()); // Add text before match
			result += "";                                    // Replace match (empty string here)
			searchStart = match.suffix().first;             // Move past the match
		}
		result += std::string(searchStart, processed.cend()); // Append the rest

		return result;
		}();
	std::regex ravRegex(R"((\([^()]+\))+?)");

	while (std::regex_search(ms, ravRegex)) {
		ms = std::regex_replace(ms, ravRegex, "");
	}

	ms = std::regex_replace(ms, std::regex(R"(\d+\.(\.\.)?)"), "");

	ms = std::regex_replace(ms, std::regex(R"(\.\.\.)"), "");

	ms = std::regex_replace(ms, std::regex(R"($\d+)"), "");

	auto moves = splitWithRegex(trim(ms), R"(\s+)");

	std::string result = "";

	for (int halfMove = 0; halfMove < static_cast<int>(moves.size()); halfMove++) {
		const auto comment = decodeComment(moves[halfMove]);
		if (!comment.empty()) { chImpl->_comments.at(fen()) = comment; continue; }

		const auto m = chImpl->_moveFromSan(moves[halfMove], strict);

		if (!m) {
			if (std::find(TERMINATION_MARKERS.begin(), TERMINATION_MARKERS.end(), moves[halfMove]) != TERMINATION_MARKERS.end()) {
				result = moves[halfMove];
			}
			else {
				throw std::runtime_error("Error move in PGN: " + moves[halfMove]);
			}
		}
		else {
			result = "";
			chImpl->_makeMove(m);
			chImpl->_incPositionCount(fen());
		}
	}

	if (!result.empty() && !chImpl->_header.empty() && chImpl->_header.count("Header") == 0) {
		header({ "Result", result });
	}
}

std::string Chess::ascii(bool isWhitePersp) {
	std::string s = "   +---+---+---+---+---+---+---+---+\n";

	int start = isWhitePersp ? 0 : 7;
	int end = isWhitePersp ? 119 : 112;
	int step = isWhitePersp ? 1 : -1;

	for (int i = start; (isWhitePersp ? i <= end : i >= end); i += step) {
		if (file(i) == 0) {
			int rankIdx = isWhitePersp ? rank(i) : 7 - rank(i);
			s += (" " + std::string(1, std::string("87654321")[rankIdx]) + " |");
		}

		if (chImpl->_board[i]) {
			PieceSymbol p = chImpl->_board[i].type;
			Color c = chImpl->_board[i].color;
			char symbol = c == WHITE ? static_cast<char>(std::toupper(pieceToChar(p))) : static_cast<char>(std::tolower(pieceToChar(p)));
			s += " " + std::string(1, symbol) + " ";
		}
		else {
			s += " . ";
		}

		if ((i + step) & 0x88) {
			s += "|\n";
			s += "   +---+---+---+---+---+---+---+---+\n";
			i += (isWhitePersp ? 8 : -8);
		}
		else {
			s += "|";
		}
	}

	// s += "   +---+---+---+---+---+---+---+---+\n";
	s += "     ";

	if (isWhitePersp) {
		s += "a   b   c   d   e   f   g   h";
	}
	else {
		s += "h   g   f   e   d   c   b   a";
	}

	return s;
}

std::string Chess::fen() {
	int empty = 0;
	std::string fen = "";

	for (int i = 0; i <= 119; i++) {
		if (chImpl->_board[i].type != PNONE) {
			if (empty > 0) {
				fen += std::to_string(empty);
				empty = 0;
			}
			Color c = chImpl->_board[i].color;
			PieceSymbol type = chImpl->_board[i].type;

			fen += (c == WHITE) ? static_cast<char>(std::toupper(pieceToChar(type))) : static_cast<char>(std::tolower(pieceToChar(type)));
		}
		else {
			empty++;
		}

		if ((i + 1) & 0x88) {
			if (empty > 0) {
				fen += std::to_string(empty);
			}
			if (i != 119) {
				fen += '/';
			}

			empty = 0;
			i += 8;
		}
	}

	std::string castling = "";
	if (chImpl->_castling[WHITE] & BITS_KSIDE_CASTLE) {
		castling += 'K';
	}
	if (chImpl->_castling[WHITE] & BITS_QSIDE_CASTLE) {
		castling += 'Q';
	}
	if (chImpl->_castling[BLACK] & BITS_KSIDE_CASTLE) {
		castling += 'k';
	}
	if (chImpl->_castling[BLACK] & BITS_QSIDE_CASTLE) {
		castling += 'q';
	}

	castling = castling.empty() ? "-" : castling;

	std::string epSquare = "-";

	if (chImpl->_epSquare != -1) {
		Square bigPawnSquare = static_cast<Square>(chImpl->_epSquare + (chImpl->_turn == WHITE ? 16 : -16));
		std::vector<int> squares = { static_cast<int>(bigPawnSquare) + 1, static_cast<int>(bigPawnSquare) - 1 };
		for (auto& sq : squares) {
			if (sq & 0x88) {
				continue;
			}
			Color ct = chImpl->_turn;
			Piece p = chImpl->_board[sq];
			if (
				p &&
				p.color == ct &&
				p.type == PAWN
				) {
				chImpl->_makeMove({
					ct,
					sq,
					chImpl->_epSquare,
					PAWN,
					PAWN,
					PNONE,
					BITS_EP_CAPTURE
					});
				bool isLegal = !chImpl->_isKingAttacked(ct);
				chImpl->_undoMove();

				if (isLegal) {
					epSquare = squareToString(algebraic(chImpl->_epSquare));
					break;
				}
			}
		}
	}
	std::vector<std::string> elements = { fen, std::string(chImpl->_turn == WHITE ? "w" : "b"), castling, epSquare, std::to_string(chImpl->_halfMoves), std::to_string(chImpl->_moveNumber) };
	return join(elements, " ");
}

void Chess::load(std::string fen, bool skipValidation, bool preserveHeaders) {
	std::vector<std::string> tokens = splitWithRegex(fen, "\\s+");
	if (tokens.size() >= 2 && tokens.size() < 6) {
		std::vector<char> adjustments = { '-', '-', '0', '1' };
		int sliceLength = std::max(0, 6 - static_cast<int>(tokens.size()));
		std::vector<std::string> slicedAdjustments;
		std::transform(
			adjustments.end() - sliceLength, adjustments.end(),
			std::back_inserter(slicedAdjustments),
			[](char c) { return std::string(1, c); }
		);
		std::vector<std::string> result = tokens;
		result.insert(result.end(), slicedAdjustments.begin(), slicedAdjustments.end());
		std::ostringstream oss;
		for (size_t i = 0; i < result.size(); ++i) {
			if (i > 0) oss << " ";
			oss << result[i];
		}
		fen = oss.str();
	}
	tokens = splitWithRegex(fen, R"(\s+)");

	if (!skipValidation) {
		std::pair<bool, std::string> result = validateFen(fen);
		if (!result.first) {
			throw std::runtime_error(result.second.c_str());
		}
	}
	const std::string position = tokens[0];
	int squ = 0;

	clear(preserveHeaders);

	for (int i = 0; i < static_cast<int>(position.size()); i++) {
		const std::string p = std::string(1, static_cast<char>(position.at(i)));

		if (p == "/") {
			squ += 8;
		}
		else if (isDigit(p)) {
			squ += std::stoi(p);
		}
		else {
			const Color color = p[0] < 'a' ? WHITE : BLACK;
			try {
				chImpl->_put(charToSymbol(static_cast<char>(std::tolower(p[0]))), color, algebraic(squ));
			}
			catch (const std::exception& e) {
				std::cout << e.what() << '\n';
				throw std::exception();
			}
			squ++;
		}
	}
	chImpl->_turn = tokens[1][0] == 'w' ? WHITE : BLACK;

	auto kingSideCastleWhite = std::find(tokens[2].begin(), tokens[2].end(), 'K');
	auto queenSideCastleWhite = std::find(tokens[2].begin(), tokens[2].end(), 'Q');
	auto kingSideCastleBlack = std::find(tokens[2].begin(), tokens[2].end(), 'k');
	auto queenSideCastleBlack = std::find(tokens[2].begin(), tokens[2].end(), 'q');

	if (kingSideCastleWhite != tokens[2].end())
		chImpl->_castling.at(WHITE) |= BITS_KSIDE_CASTLE;
	if (queenSideCastleWhite != tokens[2].end())
		chImpl->_castling.at(WHITE) |= BITS_QSIDE_CASTLE;
	if (kingSideCastleBlack != tokens[2].end())
		chImpl->_castling.at(BLACK) |= BITS_KSIDE_CASTLE;
	if (queenSideCastleBlack != tokens[2].end())
		chImpl->_castling.at(BLACK) |= BITS_QSIDE_CASTLE;

	chImpl->_epSquare = tokens[3] == "-" ? -1 : Ox88.at((int)(stringToSquare(tokens[3])));
	chImpl->_halfMoves = std::stoi(tokens[4]);
	chImpl->_moveNumber = std::stoi(tokens[5]);

	chImpl->_updateSetup(fen);
	chImpl->_incPositionCount(fen);
}

int Chess::moveNumber() {
	return chImpl->_moveNumber;
}

bool Chess::inCheck() {
	return isCheck();
}

bool Chess::isCheckmate() {
	return isCheck() && chImpl->_moves().size() == 0;
}

bool Chess::isStalemate() {
	return !isCheck() && chImpl->_moves().size() == 0;
}

bool Chess::isDraw() {
	return (
		chImpl->_halfMoves >= 100 ||
		isStalemate() ||
		inSufficientMaterial() ||
		isThreefoldRepetition()
		);
}

bool Chess::isGameOver() {
	return isCheckmate() || isStalemate() || isDraw();
}

std::vector<std::string> Chess::historys() {
	auto hs = history(false);
	std::vector<std::string> result;

	for (auto& m : hs) {
		result.push_back(std::get<std::string>(m));
	}
	return result;
}

std::vector<move> Chess::historym() {
	auto hs = history(true);
	std::vector<move> result;

	for (auto& m : hs) {
		result.push_back(std::get<move>(m));
	}
	return result;
}

std::vector<std::tuple<std::string, move>> Chess::history(bool verbose) {
	std::vector<InternalMove> reservedHistory;
	std::vector<std::tuple<std::string, move>> moveHistory;

	while (chImpl->_history.size() > 0) {
		reservedHistory.push_back(chImpl->_undoMove());
	}

	while (true) {
		if (reservedHistory.empty()) return moveHistory;

		auto m = reservedHistory.back();
		reservedHistory.pop_back();

		if (!m) break;

		if (verbose) {
			moveHistory.push_back(std::tuple<std::string, move>("", chImpl->_makePretty(m)));
		}
		else {
			moveHistory.push_back(std::tuple<std::string, move>(chImpl->_moveToSan(m, chImpl->_moves()), move()));
		}
		chImpl->_makeMove(m);
	}

	return moveHistory;
}

std::string moveToUci(move m) {
	if (m.from == EMPTY || m.to == EMPTY) return "";
	return (
		squareToString(m.from) +
		squareToString(m.to) +
		(m.promotion == PNONE ? "" : std::string(1, pieceToChar(m.promotion)))
	);
}