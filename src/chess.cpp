#include "../include/chess.h"
#include <regex>
#include <cctype>
#include <sstream>
#include <cmath>
#include <tuple>
#include <functional>
#include <algorithm>

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
	std::sregex_token_iterator it(str.begin(), str.end(), re, -1); 	std::sregex_token_iterator end;

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

/* Class definitions start here */

void Chess::_updateSetup(std::string fen) {
	if (_history.size() > 0) return;

	if (fen != DEFAULT_POSITION) {
		_header.at("SetUp") = '1';
		_header.at("FEN") = fen;
	}
	else {
		_header.erase("SetUp");
		_header.erase("FEN");
	}
}

void Chess::reset() {
	load(DEFAULT_POSITION);
}

bool Chess::_put(pieceSymbol type, color color, square sq) {
	if (SYMBOLS.at(std::tolower(ptoc.at(type))) == -1) {
		return false;
	}
	if (Ox88.count(sq) > 0) {
		return false;
	}
	const square squ = static_cast<square>(Ox88[sq]);

	if (type == KING && !(_kings.at(color) == EMPTY || _kings.at(color) == static_cast<int>(sq))) {
		return false;
	}

	const std::optional<piece> currentPieceOnsquare = _board[static_cast<int>(sq)];
	 
	if (currentPieceOnsquare.has_value() && currentPieceOnsquare.value().type == KING) {
		_kings[currentPieceOnsquare.value().color] = EMPTY;
	}

	_board[static_cast<int>(sq)] = {color, type};

	if (type == KING) {
		_kings[color] = static_cast<int>(sq);
	}
	return true;
}

void Chess::_updateCastlingRights() {
	const bool whiteKingInPlace =
		!_board.empty() &&
		_board[Ox88.at(square::e1)].value().type == KING &&
		_board[Ox88.at(square::e1)].value().color == WHITE;
	const bool blackKingInPlace =
		!_board.empty() &&
		_board[Ox88.at(square::e8)].value().type == KING &&
		_board[Ox88.at(square::e8)].value().color == BLACK;
	if (!whiteKingInPlace || _board[Ox88.at(square::a1)].value().type != ROOK || _board[Ox88.at(square::a1)].value().color != WHITE) {
		_castling.at(color::w) &= ~BITS.at("QSIDE_CASTLE");
	}
	if (!whiteKingInPlace || _board[Ox88.at(square::h1)].value().type != ROOK || _board[Ox88.at(square::h1)].value().color != WHITE) {
		_castling.at(color::w) &= ~BITS.at("KSIDE_CASTLE");
	}
	if (!whiteKingInPlace || _board[Ox88.at(square::a8)].value().type != ROOK || _board[Ox88.at(square::a8)].value().color != BLACK) {
		_castling.at(color::b) &= ~BITS.at("QSIDE_CASTLE");
	}
	if (!whiteKingInPlace || _board[Ox88.at(square::h8)].value().type != ROOK || _board[Ox88.at(square::h8)].value().color != BLACK) {
		_castling.at(color::b) &= ~BITS.at("KSIDE_CASTLE");
	}
}

void Chess::_updateEnPassantSquare() {
	if (_epSquare == EMPTY) return;

	const square startsquare = static_cast<square>(_epSquare + (_turn == WHITE ? -16 : 16));
	const square currentsquare = static_cast<square>(_epSquare + (_turn == WHITE ? -16 : 16));

	const std::array<int, 2> attackers = {static_cast<int>(currentsquare) + 1, static_cast<int>(currentsquare) - 1};

	const std::optional<piece> stsq = _board[static_cast<int>(startsquare)];
	const std::optional<piece> epsq = _board[_epSquare];
	if (stsq.has_value() || epsq.has_value() ||
		_board[static_cast<int>(currentsquare)].value().color != swapColor(_turn) ||
		_board[static_cast<int>(currentsquare)].value().type != PAWN) {
		_epSquare = EMPTY;
		return;
	}

	auto canCapture = [&](int square) -> bool {
		return !(square & 0x88) &&
				_board[square].value().color == _turn &&
				_board[square].value().type == PAWN;
	};
	if (std::any_of(attackers.begin(), attackers.end(), canCapture)) {
		_epSquare = EMPTY;
	}
}

bool Chess::_attacked(color c, square sq) {
	for (int i = Ox88.at(square::a8); i <= Ox88.at(square::h1); i++) {
		if (i & 0x88) {
			i += 7;
			continue;
		}
		std::optional<piece> currentSq = _board[i];
		if (!currentSq.has_value() || currentSq.value().color != c)
			continue;
		const piece p = currentSq.value();
		const int diff = i - static_cast<int>(sq);

		if (diff == 0) continue;
		
		const int index = diff + 119;
		if (ATTACKS[index] & PIECE_MASKS.at(p.type)) {
			if (p.type == PAWN) {
				if (diff > 0) {
					if (p.color == WHITE) return true;
				}
				else {
					if (p.color == BLACK) return true;
				}
				continue;
			}
		}

		if (p.type == pieceSymbol::n || p.type == pieceSymbol::k) return true;

		const int offset = RAYS[index];
		int j = i + offset;
		
		bool blocked = false;
		while (j != static_cast<int>(sq)) {
			std::optional<piece> pos = _board[j];
			if (pos.has_value()) {
				blocked = true;
				break;
			}
			j += offset;
		}
		if (!blocked) return true;
	}
	return false;
}

bool Chess::_isKingAttacked(color c) {
	const square sq = static_cast<square>(_kings.at(c));
	return static_cast<int>(sq) == -1 ? false : _attacked(swapColor(c), sq);
}
std::vector<internalMove> Chess::_moves(std::optional<bool> legal, std::optional<pieceSymbol> piece, std::optional<square> sq) {
	std::vector<internalMove> moves;
	color us = _turn;
	color them = swapColor(us);

	square firstSquare = sq ? *sq : square::a1;
	square lastSquare = sq ? *sq : square::h8;
	bool singleSquare = false;

	if (sq) {
		singleSquare = true;
	}

	// Iterate over the board squares
	for (int from = static_cast<int>(firstSquare); from <= static_cast<int>(lastSquare); ++from) {
		// Skip invalid squares (0x88 issue)
		if (from & 0x88) {
			from += 7;
			continue;
		}

		// Skip empty square or opponent's pieces (unless it's a capture)
		if (!_board[from] || _board[from]->color == them) {
			continue;
		}

		auto pieceType = _board[from]->type;

		// Skip if the piece doesn't match the `piece` argument, if provided
		if (piece && *piece != pieceType) {
			continue;
		}

		// Handle the specific piece moves (PAWN, KNIGHT, etc.)
		if (pieceType == pieceSymbol::p) {
			if (piece) {
				if (*piece != pieceSymbol::p) continue;
			}

			// Handle pawn's non-capturing and capturing moves
			for (auto offset : PAWN_OFFSETS.at(us)) {
				int to = from + offset;
				if (to & 0x88) continue;  // Skip invalid squares

				if (&_board[to].value() == nullptr) {
					addMove(moves, us, from, to, pieceSymbol::p, std::nullopt);
				}
				// Handle pawn captures (both normal and en passant)
				for (int j = 2; j < 4; ++j) {
					to = from + PAWN_OFFSETS.at(us)[j];
					if (to & 0x88) continue;

					if (_board[to] && _board[to]->color == them) {
						addMove(moves, us, from, to, pieceSymbol::p, _board[to]->type, BITS.at("CAPTURE"));
					}
					else if (to == _epSquare) {
						addMove(moves, us, from, to, pieceSymbol::p, pieceSymbol::p, BITS.at("EP_CAPTURE"));
					}
				}
			}
		}
		else {
			// Handle non-pawn piece moves (KNIGHT, ROOK, etc.)
			if (piece) {
				if (*piece != pieceType) continue;
			}

			// Loop through offsets for the piece type
			for (const auto& offset : PIECE_OFFSETS.at(pieceType)) {
				int to = from;

				// Handle sliding pieces (ROOK, BISHOP, QUEEN)
				while (true) {
					to += offset;
					if (to & 0x88) break;  // Skip invalid squares

					if (!_board[to]) {
						addMove(moves, us, from, to, pieceType, std::nullopt);
					}
					else {
						// Stop if own piece (except for knights and kings)
						if (_board[to]->color == us) break;

						addMove(moves, us, from, to, pieceType, _board[to]->type, BITS.at("CAPTURE"));
						break;  // Stop after capture
					}

					// Knights and Kings move differently (don't slide)
					if (pieceType == pieceSymbol::n || pieceType == pieceSymbol::k) break;
				}
			}
		}
	}

	// Handle special cases (e.g., castling)
	if (piece == std::nullopt || *piece == pieceSymbol::k) {
		// Handle king-side and queen-side castling
		if (_castling[us] & BITS.at("KSIDE_CASTLE")) {
			// logic for castling
		}
		if (_castling[us] & BITS.at("QSIDE_CASTLE")) {
			// logic for castling
		}
	}

	// If 'legal' is provided, filter out illegal moves
	if (legal && *legal) {
		std::vector<internalMove> legalMoves;
		for (const auto& move : moves) {
			_makeMove(move);
			if (!_isKingAttacked(us)) {
				legalMoves.push_back(move);
			}
			_undoMove();
		}
		return legalMoves;
	}

	return moves;  // Return all moves (legal and pseudo-legal)
}


void Chess::_push(internalMove move) {
	_history.push_back({
		move,
		{{color::b, _kings.at(color::b)}, {color::w, _kings.at(color::w)}},
		_turn,
		{{color::b, _castling.at(color::b)}, {color::w, _castling.at(color::w)}},
		_epSquare,
		_halfMoves,
		_moveNumber
	});
}

void Chess::_makeMove(internalMove m) {
	const color us = _turn;
	const color them = swapColor(us);
	_push(m);

	_board[m.to] = _board[m.from];
	std::remove(_board.begin(), _board.end(), m.from);

	if (m.flags & BITS.at("EP_CAPTURE")) {
		if (_turn == BLACK) {
			std::remove(_board.begin(), _board.end(), m.to - 16);
		}
		else {
			std::remove(_board.begin(), _board.end(), m.to + 16);
		}
	}
	if (m.promotion) {
		_board[m.to] = { us, m.promotion.value()};
	}
	if (_board[m.to].value().type == KING) {
		_kings.at(us) = m.to;

		if (m.flags & BITS.at("KSIDE_CASTLE")) {
			const int castlingTo = m.to - 1;
			const int castlingFrom = m.to + 1;
			_board[castlingTo] = _board[castlingFrom];
			std::remove(_board.begin(), _board.end(), m.from);
		}
		else if (m.flags & BITS.at("QSIDE_CASTLE")) {
			const int castlingTo = m.to + 1;
			const int castlingFrom = m.to - 2;
			_board[castlingTo] = _board[castlingFrom];
			std::remove(_board.begin(), _board.end(), m.from);
		}

		_castling.at(us) = 0;
	}
	if (_castling.at(us) != 0) {
		for (int i = 0, len = ROOKS.at(us).size(); i < len; i++) {
			if (
				m.from == ROOKS.at(us)[i].square &&
				_castling.at(us) & ROOKS.at(us)[i].flag
				) {
				_castling.at(us) ^= ROOKS.at(us)[i].flag;
				break;
			}
		}
	}
	if (_castling.at(them) != 0) {
		for (int i = 0, len = ROOKS.at(us).size(); i < len; i++) {
			if (
				m.from == ROOKS.at(them)[i].square &&
				_castling.at(them) & ROOKS.at(them)[i].flag
				) {
				_castling.at(them) ^= ROOKS.at(them)[i].flag;
				break;
			}
		}
	}
	if (m.flags & BITS.at("BIG_PAWN")) {
		if (us == BLACK) {
			_epSquare = m.to - 16;
		}
		else {
			_epSquare = m.to + 16;
		}
	}
	else {
		_epSquare = EMPTY;
	}
	if (us == BLACK) {
		_moveNumber++;
	}
	_turn = them;
}

std::optional<internalMove> Chess::_undoMove() {
	const History old = _history.back();
	_history.pop_back();

	const internalMove m = old.move;

	_kings = old.kings;
	_turn = old.turn;
	_castling = old.castling;
	_epSquare = old.epSquare;
	_halfMoves = old.halfMoves;
	_moveNumber = old.moveNumber;

	const color us = _turn;
	const color them = swapColor(us);

	_board[m.from] = _board[m.to];
	_board[m.from].value().type = m.piece;
	std::remove(_board.begin(), _board.end(), m.to);

	if (m.captured) {
		if (m.flags & BITS.at("EP_CAPTURE")) {
			int index;
			if (us == BLACK) {
				index = m.to - 16;
			}
			else {
				index = m.to + 16;
			}
			_board[index] = { them, PAWN };
		}
		else {
			_board[m.to] = {them, m.captured.value()};
		}
	}
	if (m.flags & (BITS.at("KSIDE_CASTLE") | BITS.at("QSIDE_CASTLE"))) {
		int castlingTo, castlingFrom;
		if (m.flags & BITS.at("KSIDE_CASTLE")) {
			castlingTo = m.to + 1;
			castlingFrom = m.to - 1;
		}
		else {
			castlingTo = m.to - 2;
			castlingFrom = m.to + 1;
		}
		_board[castlingTo] = _board[castlingFrom];
		std::remove(_board.begin(), _board.end(), castlingFrom);
	}
	return m;
}

void Chess::clear(std::optional<bool> preserveHeaders) {
	_board = std::array<std::optional<piece>, 128>();
	_kings = { { color::w, EMPTY }, { color::b, EMPTY } };
	_turn = WHITE;
	_castling = { {color::w, 0}, {color::b, 0} };
	_epSquare = EMPTY;
	_halfMoves = 0;
	_moveNumber = 1;
	_history = {};
	_comments = {};
	_header = preserveHeaders ? _header : std::map<std::string, std::string>();
	_positionCount = {};

	_header.erase("SetUp");
	_header.erase("FEN");
}

void Chess::removeHeader(std::string key) {
	if (_header.count(key) > 0) {
		_header.erase(key);
	}
}

std::map<pieceSymbol, char>  ptoc = {
	{PAWN, 'p'}, {ROOK, 'r'}, {BISHOP, 'b'}, {KNIGHT, 'n'}, {QUEEN, 'q'}, {KING, 'k'}
};

void Chess::load(std::string fen, bool skipValidation, bool preserveHeaders) {
	std::vector<std::string> tokens = splitWithRegex(fen, "\\s+");
	if (tokens.size() >= 2 && tokens.size() < 6) {
		std::vector<char> adjustments = { '-', '-', '0', '1' };
		int sliceLength = std::max(0, 6 - static_cast<int>(tokens.size()));
		std::vector<std::string> slicedAdjustments(adjustments.end() - sliceLength, adjustments.end());
		std::vector<std::string> result = tokens;
		result.insert(result.end(), slicedAdjustments.begin(), slicedAdjustments.end());
		std::ostringstream oss;
		for (size_t i = 0; i < result.size(); ++i) {
			if (i > 0) oss << " ";
			oss << result[i];
		}
		fen = oss.str();
	}
	tokens = splitWithRegex(fen, "\\s+");

	if (!skipValidation) {
		std::pair<bool, std::string> result = validateFen(fen);
		if (!result.first) {
			throw std::exception(result.second.c_str());
		}
	}
	const std::string position = tokens[0];
	int square = 0;

	clear(preserveHeaders);

	for (int i = 0; i < position.size(); i++) {
		const std::string piece = std::string(1, position.at(i));

		if (piece == "/") {
			square += 8;
		}
		else if (isDigit(piece)) {
			square += std::stoi(piece);
		}
		else {
			const color color = piece[0] < 'a' ? WHITE : BLACK;
			_put(strPchrs.at(piece[0]), color, algebraic(square));
			square++;
		}
	}
}

bool Chess::isAttacked(square sq, color attackedBy) {
	return _attacked(attackedBy, static_cast<square>(Ox88.at(sq)));
}

bool Chess::isCheck() {
	return _isKingAttacked(_turn);
}

bool Chess::inCheck() {
	return isCheck();
}

bool Chess::isCheckmate() {
	return isCheck() && _moves().size() == 0;
}

bool Chess::isStalemate() {
	return !isCheck() && _moves().size() == 0;
}

bool Chess::inSufficientMaterial() {
	/*
	 * k.b. vs k.b. (of opposite colors) with mate in 1:
	 * 8/8/8/8/1b6/8/B1k5/K7 b - - 0 1
	 *
	 * k.b. vs k.n. with mate in 1:
	 * 8/8/8/8/1n6/8/B7/K1k5 b - - 2 1
	 */
	std::map<pieceSymbol, int> pieces = {
		{pieceSymbol::b, 0},
		{pieceSymbol::n, 0},
		{pieceSymbol::r, 0},
		{pieceSymbol::q, 0},
		{pieceSymbol::k, 0},
		{pieceSymbol::p, 0},
	};
	std::vector<int> bishops = {};
	int numPieces = 0;
	int squareColor = 0;

	for (int i = Ox88.at(square::a8); i <= Ox88.at(square::h1); i++) {
		squareColor = (squareColor + 1) % 2;
		if (i & 0x88) {
			i += 7;
			continue;
		}

		if (_board[i].has_value()) {
			pieces.at(_board[i].value().type) = pieces.count(_board[i].value().type) > 0 ? pieces.at(_board[i].value().type) + 1 : 1;
			if (_board[i].value().type == BISHOP) {
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
		const int len = bishops.size();
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
	return _getPositionCount(fen()) >= 3;
}

bool Chess::isDraw() {
	return (
		_halfMoves >= 100 ||
		isStalemate() ||
		inSufficientMaterial() ||
		isThreefoldRepetition()
	);
}

bool Chess::isGameOver() {
	return isCheckmate() || isStalemate() || isDraw();
}

std::vector<move> Chess::moves(std::optional<square> sq, std::optional<pieceSymbol> piece, bool verbose) {
	// 1. Generate internal moves based on square and piece filtering
	std::vector<internalMove> generatedMoves = _moves(true, piece, sq); // Assuming _moves filters moves

	std::vector<move> result;

	// 2. Convert each internalMove to a move (for SAN, LAN, etc.)
	for (const auto& internal: generatedMoves) {
		move mv;

		// Copy relevant data from internalMove to move
		mv.color = internal.color;
		mv.from = static_cast<square>(internal.from); // Assuming square is an integer
		mv.to = static_cast<square>(internal.to); // Assuming square is an integer
		mv.piece = internal.piece;
		mv.captured = internal.captured;
		mv.promotion = internal.promotion;
		mv.flags = internal.flags;

		// 3. Apply verbose or SAN/LAN formatting
		if (verbose) {
			mv.san = _makePretty(internal).san; // Custom function for verbose format
		}
		else {
			mv.san = _moveToSan(internal, generatedMoves); // Standard algebraic notation
		}

		result.push_back(mv);
	}

	return result;
}

