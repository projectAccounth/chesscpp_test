#include "pimpl.h"


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
	auto iter = std::find(SYMBOLS.begin(), SYMBOLS.end(), std::tolower(pieceToChar(type)));
	if (iter == SYMBOLS.end()) {
		return false;
	}
	if (!isValid8x8(sq)) {
		return false;
	}
	const int squ = Ox88.at((int)(sq));

	if (type == KING && !(_kings[color] == EMPTY || _kings[color] == squ)) {
		return false;
	}

	const std::optional<Piece> currentPieceOnsquare = _board[squ];

	if (currentPieceOnsquare && currentPieceOnsquare.value().type == KING) {
		_kings[currentPieceOnsquare.value().color] = EMPTY;
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
		_board[Ox88.at((int)(Square::e1))].value().type == KING &&
		_board[Ox88.at((int)(Square::e1))].value().color == WHITE;
	const bool blackKingInPlace =
		!_board.empty() &&
		_board[Ox88.at((int)(Square::e8))].value().type == KING &&
		_board[Ox88.at((int)(Square::e8))].value().color == BLACK;
	auto isPieceRook = [&](Color c, Square sq) -> bool {
		return _board[Ox88.at((int)(sq))].value().type == ROOK && _board[Ox88.at((int)(sq))].value().color == c;
	};
	if (!whiteKingInPlace || !isPieceRook(WHITE, Square::a1)) {
		_castling.at(Color::w) &= ~BITS_QSIDE_CASTLE;
	}
	if (!whiteKingInPlace || !isPieceRook(WHITE, Square::h1)) {
		_castling.at(Color::w) &= ~BITS_KSIDE_CASTLE;
	}
	if (!blackKingInPlace || !isPieceRook(BLACK, Square::a8)) {
		_castling.at(Color::b) &= ~BITS_QSIDE_CASTLE;
	}
	if (!blackKingInPlace || !isPieceRook(BLACK, Square::h8)) {
		_castling.at(Color::b) &= ~BITS_KSIDE_CASTLE;
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

std::vector<std::optional<PieceSymbol>> Chess::chrImpl::_getAttackingPiece(Color c, int sq) {
	std::vector<std::optional<PieceSymbol>> pieces;
	for (int i = 0; i <= 119; i++) {
		if (i & 0x88) {
			i += 7;
			continue;
		}
		std::optional<Piece> currentSq = _board[i];
		if (!currentSq.has_value() || currentSq.value().color != c)
			continue;
		const Piece p = currentSq.value();
		const int diff = i - sq;

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

			const int offset = RAYS[index];
			int j = i + offset;

			bool blocked = false;
			while (j != sq) {
				std::optional<Piece> pos = _board[j];
				if (pos.has_value()) {
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
	return !_getAttackingPiece(c, sq).empty();
}

bool Chess::chrImpl::_isKingAttacked(Color c) {
	const int sq = _kings[c];
	return sq == -1 ? false : _attacked(swapColor(c), sq);
}

std::vector<InternalMove> Chess::chrImpl::_moves(std::optional<bool> legal, std::optional<PieceSymbol> p, std::optional<std::string> sq) {
	std::vector<InternalMove> moves;
	Color us = _turn;
	Color them = swapColor(us);

	std::optional<Square> forSquare = sq ? std::optional<Square>(stringToSquare(sq.value())) : std::nullopt;
	std::optional<PieceSymbol> forPiece = p;

	int firstSquare = 0;
	int lastSquare = 119;
	bool singleSquare = false;

	if (forSquare) {
		if (!isValid8x8(forSquare.value())) {
			return {};
		}
		else {
			firstSquare = Ox88.at((int)(forSquare.value()));
			lastSquare = firstSquare;
			singleSquare = true;
		}
	}

	for (int from = firstSquare; from <= lastSquare; from++) {
		if (from & 0x88) {
			from += 7;
			continue;
		}

		if (!_board[from] || _board[from].value().color == them) continue;

		PieceSymbol type = _board[from].value().type;

		int to;

		if (type == PAWN) {
			if (forPiece && forPiece != type) continue;

			to = from + PAWN_OFFSETS.at(us)[0];
			if (!_board[to]) {
				addMove(moves, us, from, to, PAWN);

				to = from + PAWN_OFFSETS.at(us)[1];
				if (SECOND_RANK.at(us) == rank(from) && !_board[to]) {
					addMove(moves, us, from, to, PAWN, std::nullopt, BITS_BIG_PAWN);
				}
			}

			for (int j = 2; j < 4; j++) {
				to = from + PAWN_OFFSETS.at(us)[j];
				if (to & 0x88) continue;

				if (_board[to].has_value() && _board[to].value().color == them) {
					addMove(
						moves,
						us,
						from,
						to,
						PAWN,
						_board[to].value().type,
						BITS_CAPTURE
					);
				}
				else if (to == _epSquare) {
					addMove(moves, us, from, to, PAWN, PAWN, BITS_EP_CAPTURE);
				}
			}
		}
		else {
			if (forPiece && forPiece.value() != type) continue;

			for (int j = 0, len = static_cast<int>(PIECE_OFFSETS.at(type).size()); j < len; j++) {
				const int offset = PIECE_OFFSETS.at(type)[j];
				to = from;

				while (true) {
					to += offset;
					if (to & 0x88) break;

					if (!_board[to]) {
						addMove(moves, us, from, to, type);
					}
					else {
						if (_board[to].value().color == us) break;

						addMove(
							moves,
							us,
							from,
							to,
							type,
							_board[to].value().type,
							BITS_CAPTURE
						);
						break;
					}

					if (type == KNIGHT || type == KING) break;
				}
			}
		}
	}

	if (!forPiece || forPiece.value() == KING) {
		if (!singleSquare || lastSquare == _kings[us]) {
			if (_castling.at(us) & BITS_KSIDE_CASTLE) {
				const int castlingFrom = _kings[us];
				const int castlingTo = castlingFrom + 2;
				const bool canCastleKSide =
					!_board[castlingFrom + 1] &&
					!_board[castlingTo] &&
					!_attacked(them, _kings[(us)]) &&
					!_attacked(them, castlingFrom + 1) &&
					!_attacked(them, castlingTo);
				if (canCastleKSide) {
					addMove(
						moves,
						us,
						_kings[us],
						castlingTo,
						KING,
						std::nullopt,
						BITS_KSIDE_CASTLE
					);
				}
			}

			if (_castling[us] & BITS_QSIDE_CASTLE) {
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
					addMove(
						moves,
						us,
						_kings[us],
						castlingTo,
						KING,
						std::nullopt,
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

	for (int i = 0; i < static_cast<int>(moves.size()); i++) {
		_makeMove(moves[i]);
		if (!_isKingAttacked(us)) {
			legalMoves.push_back(moves[i]);
		}
		_undoMove();
	}

	return legalMoves;
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
			std::string disambiguator = getDisambiguator(m, moves);
			output += std::string(1, std::toupper(pieceToChar(m.piece))) + disambiguator;
		}
		if (m.flags & (BITS_CAPTURE | BITS_EP_CAPTURE)) {
			if (m.piece == PAWN) {
				output += squareToString(algebraic(m.from))[0];
			}
			output += 'x';
		}

		output += squareToString(algebraic(m.to));

		if (m.promotion) {
			output += std::string(1, '=') + std::string(1, std::toupper(pieceToChar(m.promotion.value())));
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

void Chess::chrImpl::_push(InternalMove move) {
	_history.push_back({
		move,
		{{Color::b, _kings[Color::b]}, {Color::w, _kings[Color::w]}},
		_turn,
		{{Color::b, _castling.at(Color::b)}, {Color::w, _castling.at(Color::w)}},
		_epSquare,
		_halfMoves,
		_moveNumber
		});
}

void Chess::chrImpl::_makeMove(InternalMove m) {
	const Color us = _turn;
	const Color them = swapColor(us);
	_push(m);

	_board[m.to] = _board[m.from];
	_board[m.from] = std::nullopt;

	if (m.flags & BITS_EP_CAPTURE) {
		if (_turn == BLACK) {
			_board[m.to - 16] = std::nullopt;
		}
		else {
			_board[m.to + 16] = std::nullopt;
		}
	}
	if (m.promotion) {
		_board[m.to] = { us, m.promotion.value() };
	}
	if (_board[m.to].value().type == KING) {
		_kings[(us)] = m.to;

		if (m.flags & BITS_KSIDE_CASTLE) {
			const int castlingTo = m.to - 1;
			const int castlingFrom = m.to + 1;
			_board[castlingTo] = _board[castlingFrom];
			_board[castlingFrom] = std::nullopt;
		}
		else if (m.flags & BITS_QSIDE_CASTLE) {
			const int castlingTo = m.to + 1;
			const int castlingFrom = m.to - 2;
			_board[castlingTo] = _board[castlingFrom];
			_board[castlingFrom] = std::nullopt;
		}

		_castling.at(us) = 0;
	}
	if (_castling.at(us)) {
		for (int i = 0; i < static_cast<int>(ROOKS.at(us).size()); i++) {
			if (
				m.from == ROOKS.at(us)[i].square &&
				_castling.at(us) & ROOKS.at(us)[i].flag
				) {
				_castling.at(us) ^= ROOKS.at(us)[i].flag;
				break;
			}
		}
	}
	if (_castling.at(them)) {
		for (int i = 0; i < static_cast<int>(ROOKS.at(us).size()); i++) {
			if (
				m.from == ROOKS.at(them)[i].square &&
				_castling.at(them) & ROOKS.at(them)[i].flag
				) {
				_castling.at(them) ^= ROOKS.at(them)[i].flag;
				break;
			}
		}
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

std::optional<InternalMove> Chess::chrImpl::_undoMove() {
	const std::optional<History> old = _history.back();
	_history.pop_back();

	if (!old) return std::nullopt;

	const InternalMove m = old.value().move;

	_kings = old.value().kings;
	_turn = old.value().turn;
	_castling = old.value().castling;
	_epSquare = old.value().epSquare;
	_halfMoves = old.value().halfMoves;
	_moveNumber = old.value().moveNumber;

	const Color us = _turn;
	const Color them = swapColor(us);

	_board[m.from] = _board[m.to];
	_board[m.from].value().type = m.piece;
	_board[m.to] = std::nullopt;

	if (m.captured) {
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
			_board[m.to] = { them, m.captured.value() };
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
		_board[castlingFrom] = std::nullopt;
	}
	return m;
}

std::optional<InternalMove> Chess::chrImpl::_moveFromSan(std::string move, bool strict) {
	const std::string cleanMove = strippedSan(move);

	std::optional<PieceSymbol> pieceType = inferPieceType(cleanMove);
	std::vector<InternalMove> moves = _moves(true, pieceType);

	for (int i = 0; i < static_cast<int>(moves.size()); i++) {
		if (cleanMove == strippedSan(_moveToSan(moves[i], moves))) {
			return moves[i];
		}
	}

	if (strict) {
		return std::nullopt;
	}

	std::smatch matches;
	std::optional<std::string> p;
	std::optional<Square> from;
	std::optional<Square> to;
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

	pieceType = inferPieceType(cleanMove);
	moves = _moves(true, (p && p.value() != "") ? charToSymbol(std::tolower(p.value().at(0))) : pieceType);
	if (!to) {
		return std::nullopt;
	}
	to = stringToSquare(toSq);
	for (int i = 0, len = static_cast<int>(moves.size()); i < len; i++) {
		if (!from) {
			std::string moveStr = strippedSan(_moveToSan(moves[i], moves));
			std::string currentMove = replaceSubstring(moveStr, "x", "");
			if (cleanMove == currentMove) {
				return moves[i];
			}
		}
		else if ((!p || charToSymbol(std::tolower(p.value()[0])) == moves[i].piece) &&
			Ox88.at((int)(from.value())) == moves[i].from && 
			Ox88.at((int)(to.value())) == moves[i].to &&
			(!promotion.has_value() || charToSymbol(std::tolower(promotion.value()[0])) == moves[i].promotion)) {
			return moves[i];
		}
		else if (overlyDisambiguated) {
			Square sq = algebraic(moves[i].from);
			if ((!p || charToSymbol(std::tolower(p.value()[0])) == moves[i].piece) &&
				Ox88.at((int)(to.value())) == moves[i].to &&
				(from.value() == sq) &&
				Ox88.at((int)(from.value())) == moves[i].from &&
				Ox88.at((int)(to.value())) == moves[i].to &&
				(!promotion || charToSymbol(std::tolower(promotion.value()[0])) == moves[i].promotion)) {
				return moves[i];
			}
		}
	}
	return std::nullopt;
}

int Chess::chrImpl::_getPositionCount(std::string fen) {
	std::string trimmedFen = trimFen(fen);
	return _positionCount.count(trimmedFen) > 0 ? _positionCount.at(trimmedFen).value() : 0;
}

void Chess::chrImpl::_incPositionCount(std::string fen) {
	std::string trimmedFen = trimFen(fen);
	if (_positionCount.count(trimmedFen) == 0) {
		_positionCount.insert({ trimmedFen, 0 });
	}
	_positionCount.at(trimmedFen).value() += 1;
}

void Chess::chrImpl::_decPositionCount(std::string fen) {
	std::string trimmedFen = trimFen(fen);
	if (!_positionCount.at(trimmedFen).has_value()) {
		_positionCount.at(trimmedFen) = std::nullopt;
	}
	else {
		_positionCount.at(trimmedFen).value() -= 1;
	}
}

move Chess::chrImpl::_makePretty(InternalMove uglyMove) {
	std::string prettyFlags = "";
	Color c = uglyMove.color;
	PieceSymbol p = uglyMove.piece;
	int from = uglyMove.from;
	int to = uglyMove.to;
	int flags = uglyMove.flags;
	std::optional<PieceSymbol> cpd = uglyMove.captured;
	std::optional<PieceSymbol> promotion = uglyMove.promotion;
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

	move m{
		c, fromAlgebraic, toAlgebraic, p, cpd, promotion, prettyFlags, "", squareToString(fromAlgebraic) + squareToString(toAlgebraic), ch.fen(), ""
	};

	_makeMove(uglyMove);
	m.after = ch.fen();
	_undoMove();

	if (cpd.has_value()) {
		m.captured = cpd;
	}
	if (promotion.has_value()) {
		m.promotion = promotion;
		m.lan += pieceToChar(promotion.value());
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

