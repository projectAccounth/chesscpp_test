#include "InternalImpl.h"


/* Class definitions start here */

Chess::Chess(std::string fen) : chImpl(new chrImpl(*this)) { load(fen); }
Chess::Chess() : chImpl(new chrImpl(*this)) { load(DEFAULT_POSITION); }
Chess::~Chess() { delete chImpl; }


void Chess::chrImpl::_updateSetup(std::string fen) {
	if (_history.size() > 0) return;

	if (fen != DEFAULT_POSITION) {
		if (_header.count("SetUp") == 0) _header.insert({ "SetUp", "1" }); else _header.at("SetUp") = "1";
		if (_header.count("FEN") == 0) _header.insert({ "FEN", fen }); else _header.at("FEN") = fen;
	}
	else {
		_header.erase("SetUp");
		_header.erase("FEN");
	}
}

bool Chess::chrImpl::_put(PieceSymbol type, Color color, Square sq) {
	auto iter = std::find(SYMBOLS.begin(), SYMBOLS.end(), std::tolower(Helper::pieceToChar(type)));
	if (iter == SYMBOLS.end()) {
		return false;
	}
	if (!Helper::isValid8x8(sq)) {
		return false;
	}
	const int squ = Ox88.at((int)(sq));

	if (type == KING && !(_kings[color] == EMPTY || _kings[color] == squ)) {
		return false;
	}

	const Piece currentPieceOnSquare = _board[squ];

	if (currentPieceOnSquare && currentPieceOnSquare.type == KING) {
		_kings[currentPieceOnSquare.color] = EMPTY;
	}

	_board[squ] = { color, type };

	if (type == KING) {
		_kings[color] = squ;
	}
	return true;
}

void Chess::chrImpl::_updateCastlingRights() {
	const bool whiteKingInPlace =
		!_board.empty() &&
		_board[Ox88.at((int)(Square::e1))].type == KING &&
		_board[Ox88.at((int)(Square::e1))].color == WHITE;
	const bool blackKingInPlace =
		!_board.empty() &&
		_board[Ox88.at((int)(Square::e8))].type == KING &&
		_board[Ox88.at((int)(Square::e8))].color == BLACK;
	auto isPieceRook = [&](Color c, Square sq) -> bool {
		return _board[Ox88.at((int)(sq))].type == ROOK && _board[Ox88.at((int)(sq))].color == c;
	};
	if (!whiteKingInPlace || !isPieceRook(WHITE, Square::a1)) {
		_castlings &= ~CASTLE_WQ;
	}
	if (!whiteKingInPlace || !isPieceRook(WHITE, Square::h1)) {
		_castlings &= ~CASTLE_WK;
	}
	if (!blackKingInPlace || !isPieceRook(BLACK, Square::a8)) {
		_castlings &= ~CASTLE_BQ;
	}
	if (!blackKingInPlace || !isPieceRook(BLACK, Square::h8)) {
		_castlings &= ~CASTLE_BK;
	}
}

void Chess::chrImpl::_updateEnPassantSquare() {
	if (_epSquare == EMPTY) return;

	const Square startsquare = static_cast<Square>(_epSquare + (_turn == WHITE ? -16 : 16));
	const Square currentsquare = static_cast<Square>(_epSquare + (_turn == WHITE ? -16 : 16));

	const std::array<int, 2> attackers = { static_cast<int>(currentsquare) + 1, static_cast<int>(currentsquare) - 1 };

	const std::optional<Piece> stsq = _board[static_cast<int>(startsquare)];
	const std::optional<Piece> epsq = _board[_epSquare];
	if (stsq.has_value() || epsq.has_value() ||
		_board[static_cast<int>(currentsquare)].color != Helper::swapColor(_turn) ||
		_board[static_cast<int>(currentsquare)].type != PAWN) {
		_epSquare = EMPTY;
		return;
	}

	auto canCapture = [&](int square) -> bool {
		return !(square & 0x88) &&
			_board[square].color == _turn &&
			_board[square].type == PAWN;
		};
	if (std::any_of(attackers.begin(), attackers.end(), canCapture)) {
		_epSquare = EMPTY;
	}
}

std::vector<PieceSymbol> Chess::chrImpl::_getAttackingPiece(Color c, int sq) {
	std::vector<PieceSymbol> pieces;
	for (int i = 0; i <= 119; i++) {
		if (i & 0x88) {
			i += 7;
			continue;
		}
		const auto& currentSq = _board[i];
		if (!currentSq || currentSq.color != c)
			continue;
		const Piece& p = currentSq;
		const int& diff = i - sq;

		if (diff == 0) continue;

		const int index = diff + 119;
		if (ATTACKS[index] & PIECE_MASKS.at(p.type)) {
			if (p.type == PAWN) {
				if (diff > 0) {
					if (p.color == WHITE) pieces.push_back(p.type);
				}
				else {
					if (p.color == BLACK) pieces.push_back(p.type);
				}
				continue;
			}

			if (p.type == KNIGHT || p.type == KING) pieces.push_back(p.type);

			const int& offset = RAYS[index];
			int j = i + offset;

			bool blocked = false;
			while (j != sq) {
				const auto& pos = _board[j];
				if (pos) {
					blocked = true;
					break;
				}
				j += offset;
			}
			if (!blocked) pieces.push_back(p.type);
		}
	}
	return pieces;
}

bool Chess::chrImpl::_attacked(Color c, int sq) {
	for (int i = 0; i <= 119; i++) {
		if (i & 0x88) {
			i += 7;
			continue;
		}
		const auto& currentSq = _board[i];
		if (!currentSq || currentSq.color != c)
			continue;
		const Piece& p = currentSq;
		const int& diff = i - sq;

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

			if (p.type == KNIGHT || p.type == KING) return true;

			const int& offset = RAYS[index];
			int j = i + offset;

			bool blocked = false;
			while (j != sq) {
				const auto& pos = _board[j];
				if (pos) {
					blocked = true;
					break;
				}
				j += offset;
			}
			if (!blocked) return true;
		}
	}
	return false;
}

bool Chess::chrImpl::_isKingAttacked(Color c) {
	const int sq = _kings[c];
	return sq == -1 ? false : _attacked(Helper::swapColor(c), sq);
}

std::vector<InternalMove> Chess::chrImpl::_moves(const bool& legal, const PieceSymbol& p, const std::string& sq) {
	std::vector<InternalMove> moves;
	moves.reserve(128);
	const Color& us = _turn;
	const Color& them = us == WHITE ? BLACK : WHITE;

	const Square& forSquare = !sq.empty() ? stringToSquare(sq) : Square::NONE;
	const PieceSymbol& forPiece = p;

	int firstSquare = 0;
	int lastSquare = 119;
	bool singleSquare = false;

	PieceSymbol pieceType;

	if (forSquare != Square::NONE) {
		if (!Helper::isValid8x8(forSquare)) {
			return {};
		}
		else {
			firstSquare = Ox88.at((int)(forSquare));
			lastSquare = firstSquare;
			singleSquare = true;
		}
	}

	for (int from = firstSquare; from <= lastSquare; from++) {
		if (from & 0x88) {
			from += 7;
			continue;
		}

		if (!_board[from] || _board[from].color == them) continue;

		pieceType = _board[from].type;

		int to;

		if (pieceType == PAWN) {
			if (forPiece != PieceSymbol::NONE && forPiece != pieceType) continue;

			to = from + PAWN_OFFSETS.at(us)[0];
			if (!_board[to]) {
				Helper::addMove(moves, us, from, to, PAWN);

				to = from + PAWN_OFFSETS.at(us)[1];
				if (SECOND_RANK.at(us) == rank(from) && !_board[to]) {
					Helper::addMove(moves, us, from, to, PAWN, PieceSymbol::NONE, BITS_BIG_PAWN);
				}
			}

			for (int j = 2; j < 4; j++) {
				to = from + PAWN_OFFSETS.at(us)[j];
				if (to & 0x88) continue;

				if (_board[to] && _board[to].color == them) {
					Helper::addMove(
						moves,
						us,
						from,
						to,
						PAWN,
						_board[to].type,
						BITS_CAPTURE
					);
				}
				else if (to == _epSquare) {
					Helper::addMove(moves, us, from, to, PAWN, PAWN, BITS_EP_CAPTURE);
				}
			}
		}
		else {
			if (forPiece != PieceSymbol::NONE && forPiece != pieceType) continue;

			for (int j = 0, len = static_cast<int>(PIECE_OFFSETS.at(pieceType).size()); j < len; j++) {
				const int offset = PIECE_OFFSETS.at(pieceType)[j];
				to = from;

				while (true) {
					to += offset;
					if (to & 0x88) break;

					if (!_board[to]) {
						Helper::addMove(moves, us, from, to, pieceType);
					}
					else {
						if (_board[to].color == us) break;

						Helper::addMove(
							moves,
							us,
							from,
							to,
							pieceType,
							_board[to].type,
							BITS_CAPTURE
						);
						break;
					}

					if (pieceType == KNIGHT || pieceType == KING) break;
				}
			}
		}
	}

	if (forPiece == PieceSymbol::NONE || forPiece == KING) {
		if (!singleSquare || lastSquare == _kings[us]) {
			if (_castlings & CASTLE_KSIDE(us)) {
				const int castlingFrom = _kings[us];
				const int castlingTo = castlingFrom + 2;
				const bool canCastleKSide =
					!_board[castlingFrom + 1] &&
					!_board[castlingTo] &&
					!_attacked(them, _kings[(us)]) &&
					!_attacked(them, castlingFrom + 1) &&
					!_attacked(them, castlingTo);
				if (canCastleKSide) {
					Helper::addMove(
						moves,
						us,
						_kings[us],
						castlingTo,
						KING,
						PieceSymbol::NONE,
						BITS_KSIDE_CASTLE
					);
				}
			}

			if (_castlings & CASTLE_QSIDE(us)) {
				const int castlingFrom = _kings[us];
				const int castlingTo = castlingFrom - 2;
				const bool canCastleQSide =
					!_board[castlingFrom - 1] &&
					!_board[castlingFrom - 2] &&
					!_board[castlingFrom - 3] &&
					!_attacked(them, _kings[us]) &&
					!_attacked(them, castlingFrom - 1) &&
					!_attacked(them, castlingTo);
				if (canCastleQSide) {
					Helper::addMove(
						moves,
						us,
						_kings[us],
						castlingTo,
						KING,
						PieceSymbol::NONE,
						BITS_QSIDE_CASTLE
					);
				}
			}
		}
	}

	 // return all pseudo-legal moves (this includes moves that allow the king
	 // to be captured)
	 
	if (!legal || _kings[us] == EMPTY) {
		return moves;
	}

	std::vector<InternalMove> legalMoves = {};
	legalMoves.reserve(64);

	for (int i = 0; i < static_cast<int>(moves.size()); i++) {
		_makeMove(moves[i]);
		if (!_isKingAttacked(us)) {
			legalMoves.push_back(moves[i]);
		}
		_undoMove();
	}

	return legalMoves;
}

void Chess::chrImpl::_push(const InternalMove& move) {
	_history.push_back({
		move,
		_kings,
		_turn,
		_castlings,
		_epSquare,
		_halfMoves,
		_moveNumber
	});
}

void Chess::chrImpl::_makeMove(const InternalMove& m) {
	const Color& us = _turn;
	const Color& them = Helper::swapColor(us);
	_push(m);

	_board[m.to] = _board[m.from];
	_board[m.from] = Piece();

	if (m.flags & BITS_EP_CAPTURE) {
		if (_turn == BLACK) {
			_board[m.to - 16] = Piece();
		}
		else {
			_board[m.to + 16] = Piece();
		}
	}
	if (m.promotion != PieceSymbol::NONE) {
		_board[m.to] = Piece(us, m.promotion);
	}
	if (_board[m.to].type == KING) {
		_kings[us] = m.to;

		if (m.flags & BITS_KSIDE_CASTLE) {
			const int castlingTo = m.to - 1;
			const int castlingFrom = m.to + 1;
			_board[castlingTo] = _board[castlingFrom];
			_board[castlingFrom] = Piece();
		}
		else if (m.flags & BITS_QSIDE_CASTLE) {
			const int castlingTo = m.to + 1;
			const int castlingFrom = m.to - 2;
			_board[castlingTo] = _board[castlingFrom];
			_board[castlingFrom] = Piece();
		}
		_castlings &= ~(CASTLE_KSIDE(us) | CASTLE_QSIDE(us));
	}

	switch (m.from) {
	case 0:  _castlings &= ~CASTLE_WQ; break;
	case 7:  _castlings &= ~CASTLE_WK; break;
	case 56: _castlings &= ~CASTLE_BQ; break;
	case 63: _castlings &= ~CASTLE_BK; break;
	}

	switch (m.from) {
	case 0:  _castlings &= ~CASTLE_WQ; break;
	case 7:  _castlings &= ~CASTLE_WK; break;
	case 56: _castlings &= ~CASTLE_BQ; break;
	case 63: _castlings &= ~CASTLE_BK; break;
	}

	if (m.flags & BITS_BIG_PAWN) {
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
	if (m.piece == PAWN) {
		_halfMoves = 0;
	}
	else if (m.flags & (BITS_CAPTURE | BITS_EP_CAPTURE)) {
		_halfMoves = 0;
	}
	else {
		_halfMoves++;
	}

	if (us == BLACK) {
		_moveNumber++;
	}
	_turn = them;
}

InternalMove Chess::chrImpl::_undoMove() {
	const std::optional<History>& old = _history.back();
	_history.pop_back();

	if (!old) return InternalMove();

	const InternalMove& m = old.value().move;

	_kings = old.value().kings;
	_turn = old.value().turn;
	_castlings = old.value().castling;
	_epSquare = old.value().epSquare;
	_halfMoves = old.value().halfMoves;
	_moveNumber = old.value().moveNumber;

	const Color& us = _turn;
	const Color& them = Helper::swapColor(us);

	_board[m.from] = _board[m.to];
	_board[m.from].type = m.piece;
	_board[m.to] = Piece();

	if (m.captured != PieceSymbol::NONE) {
		if (m.flags & BITS_EP_CAPTURE) {
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
			_board[m.to] = { them, m.captured };
		}
	}
	if (m.flags & (BITS_KSIDE_CASTLE | BITS_QSIDE_CASTLE)) {
		int castlingTo, castlingFrom;
		if (m.flags & BITS_KSIDE_CASTLE) {
			castlingTo = m.to + 1;
			castlingFrom = m.to - 1;
		}
		else {
			castlingTo = m.to - 2;
			castlingFrom = m.to + 1;
		}
		_board[castlingTo] = _board[castlingFrom];
		_board[castlingFrom] = Piece();
	}
	return m;
}

std::string Chess::chrImpl::_moveToSan(InternalMove m, std::vector<InternalMove> moves) {
	std::string output = "";

	if (m.flags & BITS_KSIDE_CASTLE) {
		output = "O-O";
	}
	else if (m.flags & BITS_QSIDE_CASTLE) {
		output = "O-O-O";
	}
	else {
		if (m.piece != PAWN) {
			std::string disambiguator = Helper::getDisambiguator(m, moves);
			output += std::string(1, std::toupper(Helper::pieceToChar(m.piece))) + disambiguator;
		}
		if (m.flags & (BITS_CAPTURE | BITS_EP_CAPTURE)) {
			if (m.piece == PAWN) {
				output += squareToString(algebraic(m.from))[0];
			}
			output += 'x';
		}

		output += squareToString(algebraic(m.to));

		if (m.promotion != PieceSymbol::NONE) {
			output += std::string(1, '=') + std::string(1, std::toupper(Helper::pieceToChar(m.promotion)));
		}
	}

	_makeMove(m);

	if (ch.isCheck()) {
		if (ch.isCheckmate()) {
			output += '#';
		}
		else {
			output += '+';
		}
	}
	_undoMove();

	return output;
}

std::optional<InternalMove> Chess::chrImpl::_moveFromSan(std::string move, bool strict) {
	const std::string cleanMove = Helper::strippedSan(move);

	PieceSymbol pieceType = Helper::inferPieceType(cleanMove);
	std::vector<InternalMove> moves = _moves(true, pieceType);

	for (int i = 0; i < static_cast<int>(moves.size()); i++) {
		if (cleanMove == Helper::strippedSan(_moveToSan(moves[i], moves))) {
			return moves[i];
		}
	}

	if (strict) {
		return std::nullopt;
	}

	std::smatch matches;
	std::optional<std::string> p;
	Square from;
	Square to;
	std::optional<std::string> promotion;

	bool overlyDisambiguated = false;

	std::regex case1(R"(([pnbrqkPNBRQK])?([a-h][1-8])x?-?([a-h][1-8])([qrbnQRBN])?)");
	std::regex case2(R"(([pnbrqkPNBRQK])?([a-h]?[1-8]?)x?-?([a-h][1-8])([qrbnQRBN])?)");

	bool hasMatch = std::regex_match(cleanMove, matches, case1);

	std::string fromSq, toSq;

	if (hasMatch) {
		p = matches[1];
		fromSq = matches[2];
		toSq = matches[3];
		promotion = matches[4];

		to = stringToSquare(toSq);
		from = fromSq.size() == 1 ? stringToSquare(fromSq + toSq[1]) : stringToSquare(fromSq);

		if (fromSq.size() == 1) {
			overlyDisambiguated = true;
		}
	}
	else {
		hasMatch = std::regex_match(cleanMove, matches, case2);
		if (hasMatch) {
			p = matches[1];
			fromSq = matches[2];
			toSq = matches[3];
			promotion = matches[4];

			to = stringToSquare(toSq);

			if (fromSq.size() == 1) {
				overlyDisambiguated = true;
				from = stringToSquare(fromSq + toSq[1]);
			}
		}
	}

	pieceType = Helper::inferPieceType(cleanMove);
	moves = _moves(true, (p && p.value() != "") ? Helper::charToSymbol(std::tolower(p.value().at(0))) : pieceType);
	if (to == Square::NONE) {
		return std::nullopt;
	}
	to = stringToSquare(toSq);
	for (int i = 0, len = static_cast<int>(moves.size()); i < len; i++) {
		if (from == Square::NONE) {
			std::string moveStr = Helper::strippedSan(_moveToSan(moves[i], moves));
			std::string currentMove = Helper::replaceSubstring(moveStr, "x", "");
			if (cleanMove == currentMove) {
				return moves[i];
			}
		}
		else if ((!p || Helper::charToSymbol(std::tolower(p.value()[0])) == moves[i].piece) &&
			Ox88.at((int)(from)) == moves[i].from && 
			Ox88.at((int)(to)) == moves[i].to &&
			(!promotion.has_value() || Helper::charToSymbol(std::tolower(promotion.value()[0])) == moves[i].promotion)) {
			return moves[i];
		}
		else if (overlyDisambiguated) {
			Square sq = algebraic(moves[i].from);
			if ((!p || Helper::charToSymbol(std::tolower(p.value()[0])) == moves[i].piece) &&
				Ox88.at((int)(to)) == moves[i].to &&
				(from == sq) &&
				Ox88.at((int)(from)) == moves[i].from &&
				Ox88.at((int)(to)) == moves[i].to &&
				(!promotion || Helper::charToSymbol(std::tolower(promotion.value()[0])) == moves[i].promotion)) {
				return moves[i];
			}
		}
	}
	return std::nullopt;
}

int Chess::chrImpl::_getPositionCount(std::string fen) {
	std::string trimmedFen = Helper::trimFen(fen);
	return _positionCount.count(trimmedFen) > 0 ? _positionCount.at(trimmedFen).value() : 0;
}

void Chess::chrImpl::_incPositionCount(std::string fen) {
	std::string trimmedFen = Helper::trimFen(fen);
	if (_positionCount.count(trimmedFen) == 0) {
		_positionCount.insert({ trimmedFen, 0 });
	}
	_positionCount.at(trimmedFen).value() += 1;
}

void Chess::chrImpl::_decPositionCount(std::string fen) {
	std::string trimmedFen = Helper::trimFen(fen);
	if (!_positionCount.at(trimmedFen).has_value()) {
		_positionCount.at(trimmedFen) = std::nullopt;
	}
	else {
		_positionCount.at(trimmedFen).value() -= 1;
	}
}

Move Chess::chrImpl::_makePretty(InternalMove uglyMove) {
	std::string prettyFlags = "";
	Color c = uglyMove.color;
	PieceSymbol p = uglyMove.piece;
	int from = uglyMove.from;
	int to = uglyMove.to;
	int flags = uglyMove.flags;
	PieceSymbol cpd = uglyMove.captured;
	PieceSymbol promotion = uglyMove.promotion;
	const std::unordered_map<std::string, int> BITS = {
		{"NORMAL", 1},
		{"CAPTURE", 2},
		{"BIG_PAWN", 4},
		{"EP_CAPTURE", 8},
		{"PROMOTION", 16},
		{"KSIDE_CASTLE", 32},
		{"QSIDE_CASTLE", 64}
	};
	const std::unordered_map<std::string, char> FLAGS = {
		{"NORMAL", 'n'},
		{"CAPTURE", 'c'},
		{"BIG_PAWN", 'b'},
		{"EP_CAPTURE", 'e'},
		{"PROMOTION", 'p'},
		{"KSIDE_CASTLE", 'k'},
		{"QSIDE_CASTLE", 'q'}
	};
	for (auto& val : BITS) {
		if (val.second & flags) {
			prettyFlags += FLAGS.at(val.first);
		}
	}

	const Square fromAlgebraic = algebraic(from);
	const Square toAlgebraic = algebraic(to);

	Move m{
		c,
		fromAlgebraic,
		toAlgebraic,
		p,
		cpd,
		promotion,
		prettyFlags,
		"",
		squareToString(fromAlgebraic) + squareToString(toAlgebraic),
		ch.fen(),
		""
	};

	_makeMove(uglyMove);
	m.after = ch.fen();
	_undoMove();

	if (cpd != PieceSymbol::NONE) {
		m.captured = cpd;
	}
	if (promotion != PieceSymbol::NONE) {
		// If the move is a promotion, we add the promotion piece to the LAN
		m.promotion = promotion;
		m.lan += Helper::pieceToChar(promotion);
	}
	return m;
}

void Chess::chrImpl::_pruneComments() {
	std::vector<std::optional<InternalMove>> reservedHistory = {};
	std::map<std::string, std::string> currentComments = {};

	const auto copyComment = [&](std::string fen) -> void {
		if (_comments.count(fen) > 0) {
			currentComments.at(fen) = _comments.at(fen);
		}
		};

	while (_history.size() > 0) {
		reservedHistory.push_back(_undoMove());
	}

	copyComment(ch.fen());

	while (true) {
		std::optional<InternalMove> m = reservedHistory[reservedHistory.size() - 1];
		if (!m) break;
		_makeMove(m.value());
		copyComment(ch.fen());
	}
	_comments = currentComments;
}

