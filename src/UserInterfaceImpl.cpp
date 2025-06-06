#include "InternalImpl.h"

// Front-end implementations for the exposed user interface

void Chess::reset() {
	load(DEFAULT_POSITION);
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
	std::vector<std::optional<InternalMove>> reservedHistory;
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
		std::optional<InternalMove> m = reservedHistory.back();
		reservedHistory.pop_back();

		if (!m) break;

		if (chImpl->_history.size() == 0 && m.value().color == Color::b) {
			const std::string prefix = std::to_string(chImpl->_moveNumber) + ". ...";
			moveString = !moveString.empty() ? moveString + " " + prefix : prefix;
		}
		else if (m.value().color == Color::w) {
			if (!moveString.empty()) {
				moves.push_back(moveString);
			}
			moveString = std::to_string(chImpl->_moveNumber) + ".";
		}
		moveString = moveString + " " + chImpl->_moveToSan(m.value(), chImpl->_moves(true));
		chImpl->_makeMove(m.value());
	}
	if (!moveString.empty()) {
		moves.push_back(appendComment(moveString));
	}
	if (chImpl->_header.count("Result") > 0) {
		moves.push_back(chImpl->_header.at("Result"));
	}
	if (maxWidth == 0) {
		auto resultStr = Helper::join(result, "").append(Helper::join(moves, " "));
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
		for (const auto& token : Helper::split(move, ' ')) {
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
	return Helper::join(result, "");
}

std::vector<PieceSymbol> Chess::getAttackingPieces(Color c, Square sq) {
	return chImpl->_getAttackingPiece(c, Ox88.at((int)(sq)));
}

void Chess::loadPgn(std::string pgn, bool strict, std::string newlineChar) {
	auto mask = [&](std::string str) -> std::string {
		return std::regex_replace(str, std::regex(R"(\\)"), R"(\)");
		};
	auto parsePgnHeader = [&](std::string header) -> std::map<std::string, std::string> {
		std::map<std::string, std::string> headerObj = {};
		std::vector<std::string> headers = Helper::splitWithRegex(header, newlineChar);
		std::string key = "";
		std::string value = "";

		for (int i = 0; i < static_cast<int>(headers.size()); i++) {
			const std::regex reg = std::regex(R"(^\s*\[\s*([A-Za-z]+)\s*\"(.*)\"\s*\]\s*$)");
			key = std::regex_replace(headers[i], reg, "$1");
			value = std::regex_replace(headers[i], reg, "$2");
			if (Helper::trim(key).size() > 0) {
				headerObj.at(key) = value;
			}
		}
		return headerObj;
		};
	pgn = Helper::trim(pgn);
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
			lowerStr += std::tolower(c);
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
			hexStream << "%" << std::setw(2) << std::setfill('0') << std::hex << (int)c;
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
		std::string processed = Helper::replaceSubstring(pgn, headerString, "");
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

	auto moves = Helper::splitWithRegex(Helper::trim(ms), R"(\s+)");

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
			chImpl->_makeMove(m.value());
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
			char symbol = c == WHITE ? 
				static_cast<char>(std::toupper(Helper::pieceToChar(p))) :
			 	static_cast<char>(std::tolower(Helper::pieceToChar(p)));

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
		if (chImpl->_board[i]) {
			if (empty > 0) {
				fen += std::to_string(empty);
				empty = 0;
			}
			Color c = chImpl->_board[i].color;
			PieceSymbol type = chImpl->_board[i].type;

			fen += c == WHITE ? std::toupper(Helper::pieceToChar(type)) : std::tolower(Helper::pieceToChar(type));
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
	if (chImpl->_castlings & CASTLE_WK) {
		castling += 'K';
	}
	if (chImpl->_castlings & CASTLE_WQ) {
		castling += 'Q';
	}
	if (chImpl->_castlings & CASTLE_BK) {
		castling += 'k';
	}
	if (chImpl->_castlings & CASTLE_BQ) {
		castling += 'q';
	}

	castling = castling.empty() ? "-" : castling;

	std::string epSquare = "-";

	if (chImpl->_epSquare != EMPTY) {
		Square bigPawnSquare = static_cast<Square>(chImpl->_epSquare + (chImpl->_turn == WHITE ? 16 : -16));
		std::vector<int> squares = { static_cast<int>(bigPawnSquare) + 1, static_cast<int>(bigPawnSquare) - 1 };
		for (auto& sq : squares) {
			if (sq & 0x88) {
				continue;
			}
			Color ct = chImpl->_turn;
			std::optional<Piece> p = chImpl->_board[sq];
			if (
				p &&
				p.value().color == ct &&
				p.value().type == PAWN
				) {
				chImpl->_makeMove({
					ct,
					sq,
					chImpl->_epSquare,
					PAWN,
					PAWN,
					PieceSymbol::NONE,
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
	return Helper::join(elements, " ");
}

void Chess::load(std::string fen, bool skipValidation, bool preserveHeaders) {
	std::vector<std::string> tokens = Helper::splitWithRegex(fen, "\\s+");
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
	tokens = Helper::splitWithRegex(fen, R"(\s+)");

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
		else if (Helper::isDigit(p)) {
			squ += std::stoi(p);
		}
		else {
			const Color color = p[0] < 'a' ? WHITE : BLACK;
			try {
				chImpl->_put(Helper::charToSymbol(std::tolower(p[0])), color, algebraic(squ));
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
		chImpl->_castlings |= CASTLE_WK;
	if (queenSideCastleWhite != tokens[2].end())
		chImpl->_castlings |= CASTLE_WQ;
	if (kingSideCastleBlack != tokens[2].end())
		chImpl->_castlings |= CASTLE_BK;
	if (queenSideCastleBlack != tokens[2].end())
		chImpl->_castlings |= CASTLE_BQ;

	chImpl->_epSquare = tokens[3] == "-" ? EMPTY : Ox88.at((int)(stringToSquare(tokens[3])));
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

std::vector<std::string> Chess::history_s() {
	auto hs = history(false);
	std::vector<std::string> result;

	for (auto& m : hs) {
		result.push_back(std::get<std::string>(m));
	}
	return result;
}

std::vector<Move> Chess::history_m() {
	auto hs = history(true);
	std::vector<Move> result;

	for (auto& m : hs) {
		result.push_back(std::get<Move>(m));
	}
	return result;
}

std::vector<std::variant<std::string, Move>> Chess::history(bool verbose) {
	std::vector<std::optional<InternalMove>> reservedHistory;
	std::vector<std::variant<std::string, Move>> moveHistory;

	while (chImpl->_history.size() > 0) {
		reservedHistory.push_back(chImpl->_undoMove());
	}

	while (true) {
		if (reservedHistory.empty()) return moveHistory;

		auto m = reservedHistory.back();
		reservedHistory.pop_back();

		if (!m) break;

		if (verbose) {
			moveHistory.push_back(chImpl->_makePretty(m.value()));
		}
		else {
			moveHistory.push_back(chImpl->_moveToSan(m.value(), chImpl->_moves()));
		}
		chImpl->_makeMove(m.value());
	}

	return moveHistory;
}

Color Chess::turn() {
	return chImpl->_turn;
}

std::optional<Piece> Chess::remove(Square sq) {
	Piece p = get(sq);
	chImpl->_board[Ox88.at((int)(sq))] = Piece();
	if (p && p.type == KING) {
		chImpl->_kings[p.color] = EMPTY;
	}
	else if (!p) {
		return std::nullopt;
	}

	chImpl->_updateCastlingRights();
	chImpl->_updateEnPassantSquare();
	chImpl->_updateSetup(fen());

	return p;
}

std::optional<Move> Chess::undo() {
	std::optional<InternalMove> m = chImpl->_undoMove();
	if (m) {
		Move prettyMove = chImpl->_makePretty(m.value());
		chImpl->_decPositionCount(prettyMove.after);
		return prettyMove;
	}
	return std::nullopt;
}

std::optional<std::string> Chess::squareColor(Square sq) {
	if (Helper::isValid8x8(sq)) {
		int squ = Ox88.at((int)(sq));
		return (rank(squ) + file(squ)) % 2 == 0 ? "light" : "dark";
	}
	return std::nullopt;
}

Piece Chess::get(Square sq) {
	return chImpl->_board[static_cast<int>(sq)] ? chImpl->_board[static_cast<int>(sq)] : Piece();
}

Move Chess::makeMove(const std::variant<std::string, MoveOption>& moveArg, bool strict) {
	std::optional<InternalMove> moveObj = std::nullopt;
	if (std::holds_alternative<std::string>(moveArg)) {
		moveObj = chImpl->_moveFromSan(std::get<std::string>(moveArg), strict);
	}
	else {
		std::vector<InternalMove> moves = chImpl->_moves();
		MoveOption o = std::get<MoveOption>(moveArg);
		Move m = {
			chImpl->_turn,
			stringToSquare(o.from),
			stringToSquare(o.to),
			PieceSymbol::NONE,
			PieceSymbol::NONE,
			o.promotion ? PieceSymbol(Helper::charToSymbol(o.promotion.value()[0])) : PieceSymbol::NONE
		};
		for (int i = 0; i < static_cast<int>(moves.size()); i++) {
			if (
				m.from == algebraic(moves[i].from) &&
				m.to == algebraic(moves[i].to) &&
				(moves[i].promotion == PieceSymbol::NONE || m.promotion == moves[i].promotion)
				) {
				moveObj = moves[i];
				break;
			}
		}
	}

	if (!moveObj) {
		if (std::holds_alternative<std::string>(moveArg))
			throw std::runtime_error("Invalid move: " + std::get<std::string>(moveArg));
		else
			throw std::runtime_error("Invalid move: from " + std::get<MoveOption>(moveArg).from + "to " + std::get<MoveOption>(moveArg).to);
	}

	Move prettyMove = chImpl->_makePretty(moveObj.value());

	chImpl->_makeMove(moveObj.value());
	chImpl->_incPositionCount(prettyMove.after);
	return prettyMove;
}

Move Chess::makeMove(const Move& move) {
	return makeMove(move.san, false);
}

void classifyMoveFlags(const InternalMove& move, PerftStats& stats) {
	int f = move.flags;

	if (f & BITS_CAPTURE) ++stats.captures;
	if (f & BITS_EP_CAPTURE) ++stats.enPassants;
	if (f & BITS_PROMOTION) ++stats.promotions;
	if (f & (BITS_KSIDE_CASTLE | BITS_QSIDE_CASTLE)) ++stats.castles;
}

uint64_t Chess::perft(int depth) {
	if (depth == 0) return 1;

	const auto moves = chImpl->_moves(false);
	if (depth == 1) {
		int count = 0;
		Color c = chImpl->_turn;
		for (const auto& m : moves) {
			chImpl->_makeMove(m);
			if (!chImpl->_isKingAttacked(c))
				++count;
			chImpl->_undoMove();
		}
		return count;
	}

	uint64_t nodes = 0;
	Color c = chImpl->_turn;
	for (const auto& m : moves) {
		chImpl->_makeMove(m);
		if (!chImpl->_isKingAttacked(c))
			nodes += perft(depth - 1);
		chImpl->_undoMove();
	}
	return nodes;
}

std::pair<bool, bool> Chess::getCastlingRights(Color c) {
	return {
		(chImpl->_castlings & CASTLE_KSIDE(c)) != 0,
		(chImpl->_castlings & CASTLE_QSIDE(c)) != 0
	};
}

void Chess::clear(std::optional<bool> preserveHeaders) {
	chImpl->_board = std::array<Piece, 128>();
	chImpl->_kings = KingPositions();
	chImpl->_turn = WHITE;
	chImpl->_castlings = 0;
	chImpl->_epSquare = EMPTY;
	chImpl->_halfMoves = 0;
	chImpl->_moveNumber = 1;
	chImpl->_history = {};
	chImpl->_comments = {};
	chImpl->_header = preserveHeaders ? chImpl->_header : std::map<std::string, std::string>();
	chImpl->_positionCount = {};

	chImpl->_header.erase("SetUp");
	chImpl->_header.erase("FEN");
}

void Chess::removeHeader(std::string key) {
	if (chImpl->_header.count(key) > 0) {
		chImpl->_header.erase(key);
	}
}

std::map<std::string, std::string> Chess::header(std::vector<std::string> args ...) {
	for (int i = 0; i < static_cast<int>(args.size()); i += 2) {
		chImpl->_header[args[i]] = args[i + 1];
	}
	return chImpl->_header;
}

std::vector<std::vector<std::optional<std::tuple<Square, PieceSymbol, Color>>>> Chess::board() {
	std::vector<std::vector<std::optional<std::tuple<Square, PieceSymbol, Color>>>> output = {};
	std::vector<std::optional<std::tuple<Square, PieceSymbol, Color>>> row = {};

	for (int i = Ox88.at((int)(Square::a8)); i <= Ox88.at((int)(Square::h1)); i++) {
		if (!chImpl->_board[i]) {
			row.push_back(std::nullopt);
		}
		else {
			row.push_back(std::tuple<Square, PieceSymbol, Color>{
				algebraic(i),
					chImpl->_board[i].type,
					chImpl->_board[i].color
			});
		}
		if ((i + 1) & 0x88) {
			std::vector<std::optional<std::tuple<Square, PieceSymbol, Color>>> trow;
			for (const auto& elem : row) {
				if (elem.has_value()) {
					trow.push_back(elem.value());
				}
			}
			output.push_back(std::move(trow));
			row = {};
			i += 8;
		}
	}
	return output;
}

bool Chess::isAttacked(Square sq, Color attackedBy) {
	return chImpl->_attacked(attackedBy, Ox88.at((int)(sq)));
}

bool Chess::isCheck() {
	return chImpl->_isKingAttacked(chImpl->_turn);
}

bool Chess::inSufficientMaterial() {
	/*
	 * k.b. vs k.b. (of opposite colors) with mate in 1:
	 * 8/8/8/8/1b6/8/B1k5/K7 b - - 0 1
	 *
	 * k.b. vs k.n. with mate in 1:
	 * 8/8/8/8/1n6/8/B7/K1k5 b - - 2 1
	 */
	std::map<PieceSymbol, int> pieces = {
		{BISHOP, 0},
		{KNIGHT, 0},
		{ROOK, 0},
		{QUEEN, 0},
		{KING, 0},
		{PAWN, 0},
	};
	std::vector<int> bishops = {};
	int numPieces = 0;
	int squareColor = 0;
	for (int i = 0; i <= 119; i++) {
		squareColor = (squareColor + 1) % 2;
		if (i & 0x88) {

			i += 7;
			continue;
		}

		if (chImpl->_board[i]) {
			pieces.at(chImpl->_board[i].type) = pieces.count(chImpl->_board[i].type) > 0 ? pieces.at(chImpl->_board[i].type) + 1 : 1;
			if (chImpl->_board[i].type == BISHOP) {
				bishops.push_back(squareColor);
			}
			numPieces++;
		}
	}

	if (numPieces == 2) {
		return true;
	}
	else if (numPieces == 3 && (pieces.at(BISHOP) == 1 || pieces.at(KNIGHT) == 1)) {
		return true;
	}
	else if (numPieces == pieces.at(BISHOP) + 2) {
		int sum = 0;
		const int len = static_cast<int>(bishops.size());
		for (int i = 0; i < len; i++) {
			sum += bishops[i];
		}
		if (sum == 0 || sum == len) {
			return true;
		}
	}
	return false;
}

bool Chess::isThreefoldRepetition() {
	return chImpl->_getPositionCount(fen()) >= 3;
}

bool Chess::put(PieceSymbol type, Color c, Square sq) {
	if (chImpl->_put(type, c, sq)) {
		chImpl->_updateCastlingRights();
		chImpl->_updateEnPassantSquare();
		chImpl->_updateSetup(fen());
		return true;
	}
	return false;
}

std::vector<Move> Chess::getMoves(bool verbose, std::string sq, PieceSymbol piece) {
	std::vector<InternalMove> generatedMoves = chImpl->_moves(true, piece, sq);

	std::vector<Move> result;
	for (const auto& internal: generatedMoves) {
		Move mv;

		mv.color = internal.color;
		mv.from = algebraic(internal.from);
		mv.to = algebraic(internal.to);
		mv.piece = internal.piece;
		mv.captured = internal.captured;
		mv.promotion = internal.promotion;
		mv.flags = internal.flags;

		if (verbose) {
			mv.san = chImpl->_makePretty(internal).san;
		}
		else {
			mv.san = chImpl->_moveToSan(internal, generatedMoves);
		}

		result.push_back(mv);
	}
	return result;
}

std::vector<std::string> Chess::getMoves() {
	std::vector<InternalMove> generatedMoves = chImpl->_moves(true);

	std::vector<std::string> result;

	for (const auto& internal: generatedMoves) {
		result.push_back(chImpl->_moveToSan(internal, generatedMoves));
	}

	return result;
}
std::string Chess::getComment() {
	return chImpl->_comments.at(fen());
}

std::string Chess::deleteComment() {
	const std::string comment = chImpl->_comments.at(fen());
	chImpl->_comments.erase(fen());
	return comment;
}

//std::vector<std::pair<std::string, std::string>> Chess::getComments() {
//	chImpl->_pruneComments();
//}