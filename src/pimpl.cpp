#include "privImpls.h"

/* Class definitions start here */

Chess::Chess(std::string fen) : chImpl(new chrImpl(*this)) { load(fen); }
Chess::Chess() : chImpl(new chrImpl(*this)) { load(DEFAULT_POSITION); }
Chess::~Chess() { delete chImpl; }

void Chess::chrImpl::_updateSetup(std::string fen) {
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

bool Chess::chrImpl::_put(pieceSymbol type, color color, square sq) {
	auto iter = std::find(SYMBOLS.begin(), SYMBOLS.end(), std::tolower(ptoc.at(type)));
	if (iter == SYMBOLS.end()) {
		return false;
	}
	if (Ox88.count(sq) == 0) {
		return false;
	}
	const int squ = Ox88[sq];

	if (type == KING && !(_kings.at(color) == EMPTY || _kings.at(color) == squ)) {
		return false;
	}

	const std::optional<piece> currentPieceOnsquare = _board[squ];

	if (currentPieceOnsquare && currentPieceOnsquare.value().type == KING) {
		_kings[currentPieceOnsquare.value().color] = EMPTY;
	}

	_board[squ] = { color, type };

	if (type == KING) {
		_kings.at(color) = squ;
	}
	return true;
}

void Chess::chrImpl::_updateCastlingRights() {
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
	if (!blackKingInPlace || _board[Ox88.at(square::a8)].value().type != ROOK || _board[Ox88.at(square::a8)].value().color != BLACK) {
		_castling.at(color::b) &= ~BITS.at("QSIDE_CASTLE");
	}
	if (!blackKingInPlace || _board[Ox88.at(square::h8)].value().type != ROOK || _board[Ox88.at(square::h8)].value().color != BLACK) {
		_castling.at(color::b) &= ~BITS.at("KSIDE_CASTLE");
	}
}

void Chess::chrImpl::_updateEnPassantSquare() {
	if (_epSquare == EMPTY) return;

	const square startsquare = static_cast<square>(_epSquare + (_turn == WHITE ? -16 : 16));
	const square currentsquare = static_cast<square>(_epSquare + (_turn == WHITE ? -16 : 16));

	const std::array<int, 2> attackers = { static_cast<int>(currentsquare) + 1, static_cast<int>(currentsquare) - 1 };

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

bool Chess::chrImpl::_attacked(color c, int sq) {
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
	}
	return false;
}

bool Chess::chrImpl::_isKingAttacked(color c) {
	const int sq = _kings.at(c);
	return static_cast<int>(sq) == -1 ? false : _attacked(swapColor(c), sq);
}

std::vector<internalMove> Chess::chrImpl::_moves(std::optional<bool> legal, std::optional<pieceSymbol> p, std::optional<std::string> sq) {
	std::vector<internalMove> moves;
	color us = _turn;
	color them = swapColor(us);

	std::optional<square> forSquare = sq ? std::optional<square>(stringToSquare(sq.value())) : std::nullopt;
	std::optional<pieceSymbol> forPiece = p;

	int firstSquare = Ox88.at(square::a8);
	int lastSquare = Ox88.at(square::h1);
	bool singleSquare = false;

	if (forSquare) {
		if (Ox88.count(forSquare.value()) == 0) {
			return {};
		}
		else {
			firstSquare = Ox88.at(forSquare.value());
			lastSquare = firstSquare;
			singleSquare = true;
		}
	}

	for (int from = firstSquare; from <= lastSquare; from++) {
		if (from & 0x88) {
			from += 7;
			continue;
		}

		if (!_board.at(from) || _board.at(from).value().color == them)
			continue;

		pieceSymbol type = _board.at(from).value().type;

		int to;

		if (type == PAWN) {
			if (forPiece && forPiece != type) continue;

			to = from + PAWN_OFFSETS.at(us)[0];
			if (!_board.at(to)) {
				addMove(moves, us, from, to, PAWN);

				to = from + PAWN_OFFSETS.at(us)[1];
				if (SECOND_RANK.at(us) == rank(from) && !_board.at(to)) {
					addMove(moves, us, from, to, PAWN, std::nullopt, BITS.at("BIG_PAWN"));
				}
			}

			for (int j = 2; j < 4; j++) {
				to = from + PAWN_OFFSETS.at(us)[j];
				if (to & 0x88) continue;

				if (_board.at(to).has_value() && _board.at(to).value().color == them) {
					addMove(
						moves,
						us,
						from,
						to,
						PAWN,
						_board[to].value().type,
						BITS.at("CAPTURE")
					);
				}
				else if (to == _epSquare) {
					addMove(moves, us, from, to, PAWN, PAWN, BITS.at("EP_CAPTURE"));
				}
			}
		}
		else {
			if (forPiece && forPiece.value() != type) continue;

			for (int j = 0, len = PIECE_OFFSETS.at(type).size(); j < len; j++) {
				const int offset = PIECE_OFFSETS.at(type)[j];
				to = from;

				while (true) {
					to += offset;
					if (to & 0x88) break;

					if (!_board.at(to)) {
						addMove(moves, us, from, to, type);
					}
					else {
						if (_board.at(to).value().color == us) break;

						addMove(
							moves,
							us,
							from,
							to,
							type,
							_board[to].value().type,
							BITS.at("CAPTURE")
						);
						break;
					}

					if (type == KNIGHT || type == KING) break;
				}
			}
		}
	}

	if (!forPiece || forPiece.value() == KING) {
		if (!singleSquare || lastSquare == _kings.at(us)) {
			if (_castling.at(us) & BITS.at("KSIDE_CASTLE")) {
				const int castlingFrom = _kings.at(us);
				const int castlingTo = castlingFrom + 2;

				if (
					!_board.at(castlingFrom + 1) &&
					!_board.at(castlingTo) &&
					!_attacked(them, _kings.at(us)) &&
					!_attacked(them, castlingFrom + 1) &&
					!_attacked(them, castlingTo)
				) {
					addMove(
						moves,
						us,
						_kings.at(us),
						castlingTo,
						KING,
						std::nullopt,
						BITS.at("KSIDE_CASTLE")
					);
				}
			}

			if (_castling[us] & BITS.at("QSIDE_CASTLE")) {
				const int castlingFrom = _kings[us];
				const int castlingTo = castlingFrom - 2;

				if (
					!_board.at(castlingFrom - 1) &&
					!_board.at(castlingFrom - 2) &&
					!_board.at(castlingFrom - 3) &&
					!_attacked(them, _kings.at(us)) &&
					!_attacked(them, castlingFrom - 1) &&
					!_attacked(them, castlingTo)
				) {
					addMove(
						moves,
						us,
						_kings.at(us),
						castlingTo,
						KING,
						std::nullopt,
						BITS.at("QSIDE_CASTLE")
					);
				}
			}
		}
	}

	 // return all pseudo-legal moves (this includes moves that allow the king
	 // to be captured)
	 
	if (!legal || _kings.at(us) == EMPTY) {
		return moves;
	}

	std::vector<internalMove> legalMoves = {};

	for (int i = 0; i < static_cast<int>(moves.size()); i++) {
		_makeMove(moves[i]);
		if (!_isKingAttacked(us)) {
			legalMoves.push_back(moves[i]);
		}
		_undoMove();
	}

	return legalMoves;
}

std::string Chess::chrImpl::_moveToSan(internalMove m, std::vector<internalMove> moves) {
	std::string output = "";

	if (m.flags & BITS.at("KSIDE_CASTLE")) {
		output = "O-O";
	}
	else if (m.flags & BITS.at("QSIDE_CASTLE")) {
		output = "O-O-O";
	}
	else {
		if (m.piece != PAWN) {
			std::string disambiguator = getDisambiguator(m, moves);
			output += std::string(1, std::toupper(ptoc.at(m.piece))) + disambiguator;
		}
		if (m.flags & (BITS.at("CAPTURE") | BITS.at("EP_CAPTURE"))) {
			if (m.piece == PAWN) {
				output += squareToString(algebraic(m.from))[0];
			}
			output += 'x';
		}

		output += squareToString(algebraic(m.to));

		if (m.promotion) {
			output += '=' + std::toupper(ptoc.at(m.promotion.value()));
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

void Chess::chrImpl::_push(internalMove move) {
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

void Chess::chrImpl::_makeMove(internalMove m) {
	const color us = _turn;
	const color them = swapColor(us);
	_push(m);

	_board[m.to] = _board[m.from];
	_board[m.from] = std::nullopt;

	if (m.flags & BITS.at("EP_CAPTURE")) {
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
		_kings.at(us) = m.to;

		if (m.flags & BITS.at("KSIDE_CASTLE")) {
			const int castlingTo = m.to - 1;
			const int castlingFrom = m.to + 1;
			_board[castlingTo] = _board[castlingFrom];
			_board[m.from] = std::nullopt;
		}
		else if (m.flags & BITS.at("QSIDE_CASTLE")) {
			const int castlingTo = m.to + 1;
			const int castlingFrom = m.to - 2;
			_board[castlingTo] = _board[castlingFrom];
			_board[m.from] = std::nullopt;
		}

		_castling.at(us) = 0;
	}
	if (_castling.at(us) != 0) {
		for (int i = 0, len = static_cast<int>(ROOKS.at(us).size()); i < len; i++) {
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
		for (int i = 0, len = static_cast<int>(ROOKS.at(us).size()); i < len; i++) {
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

std::optional<internalMove> Chess::chrImpl::_undoMove() {
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
	_board[m.to] = std::nullopt;

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
			_board[m.to] = { them, m.captured.value() };
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
		_board[castlingFrom] = std::nullopt;
	}
	return m;
}

std::optional<internalMove> Chess::chrImpl::_moveFromSan(std::string move, bool strict) {
	const std::string cleanMove = strippedSan(move);

	pieceSymbol pieceType = inferPieceType(cleanMove).value();
	std::vector<internalMove> moves = _moves(true, pieceType);

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
	std::optional<square> from;
	std::optional<square> to;
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

	pieceType = inferPieceType(cleanMove).value();
	try {
		moves = _moves(true, p ? strPchrs.at(std::tolower(p.value().at(0))) : pieceType);
	}
	catch (const std::exception& e) {
		std::cout << e.what() + '\n';
	}
	if (!to) {
		return std::nullopt;
	}
	for (int i = 0, len = static_cast<int>(moves.size()); i < len; i++) {
		if (!from) {
			std::string currentMove = replaceSubstring(strippedSan(_moveToSan(moves[i], moves)), "x", "");
			if (cleanMove == currentMove) {
				return moves[i];
			}
		}
		else if ((!p || strPchrs.at(std::tolower(p.value()[0])) == moves[i].piece) &&
			Ox88.at(from.value()) == moves[i].from && 
			Ox88.at(to.value()) == moves[i].to &&
			(!promotion.has_value() || strPchrs.at(std::tolower(promotion.value()[0])) == moves[i].promotion)) {
			return moves[i];
		}
		else if (overlyDisambiguated) {
			square sq = algebraic(moves[i].from);
			if ((!p || strPchrs.at(std::tolower(p.value()[0])) == moves[i].piece) &&
				Ox88.at(to.value()) == moves[i].to &&
				(from.value() == sq) &&
				Ox88.at(from.value()) == moves[i].from &&
				Ox88.at(to.value()) == moves[i].to &&
				(!promotion || strPchrs.at(std::tolower(promotion.value()[0])) == moves[i].promotion)) {
				return moves[i];
			}
		}
	}
	return std::nullopt;
}

int Chess::chrImpl::_getPositionCount(std::string fen) {
	std::string trimmedFen = trimFen(fen);
	return _positionCount.at(trimmedFen).has_value() ? _positionCount.at(trimmedFen).value() : 0;
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

move Chess::chrImpl::_makePretty(internalMove uglyMove) {
	std::string prettyFlags = "";
	color c = uglyMove.color;
	pieceSymbol p = uglyMove.piece;
	int from = uglyMove.from;
	int to = uglyMove.to;
	int flags = uglyMove.flags;
	std::optional<pieceSymbol> cpd = uglyMove.captured;
	std::optional<pieceSymbol> promotion = uglyMove.promotion;

	for (auto& val : BITS) {
		if (val.second & flags) {
			prettyFlags += FLAGS.at(val.first);
		}
	}

	const square fromAlgebraic = algebraic(from);
	const square toAlgebraic = algebraic(to);

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
		m.lan += ptoc.at(promotion.value());
	}
	return m;
}

void Chess::chrImpl::_pruneComments() {
	std::vector<std::optional<internalMove>> reservedHistory = {};
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
		std::optional<internalMove> m = reservedHistory[reservedHistory.size() - 1];
		if (!m) break;
		_makeMove(m.value());
		copyComment(ch.fen());
	}
	_comments = currentComments;
}

