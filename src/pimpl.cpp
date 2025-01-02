#include "pimpl.h"


/* Class definitions start here */

Chess::Chess(std::string fen) : chImpl(new chrImpl(*this)) { load(fen); }
Chess::Chess() : chImpl(new chrImpl(*this)) { load(DEFAULT_POSITION); }
Chess::~Chess() { delete chImpl; }

internalMove::operator bool() const {
	return !(
		color == color::NO_COLOR &&
		from == 0 &&
		to == 0 &&
		piece == PNONE &&
		captured == PNONE &&
		promotion == PNONE &&
		flags == 0
	);
}

History::operator bool() const {
	return !(
		move &&
		kings == std::unordered_map<color, int>() &&
		turn == color::NO_COLOR &&
		castling == std::unordered_map<color, int>() &&
		epSquare == 0 &&
		halfMoves == 0 &&
		moveNumber == 0
	);
}

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

bool Chess::chrImpl::_put(pieceSymbol type, color color, square sq) {
	auto iter = std::find(SYMBOLS.begin(), SYMBOLS.end(), std::tolower(pieceToChar(type)));
	if (iter == SYMBOLS.end()) {
		return false;
	}
	if (!isValid8x8(sq)) {
		return false;
	}
	const int squ = squareTo0x88(sq);

	if (type == KING && !(_kings[color] == static_cast<int>(EMPTY) || _kings[color] == squ)) {
		return false;
	}

	const piece currentPieceOnsquare = _board[squ];

	if (currentPieceOnsquare && currentPieceOnsquare.type == KING) {
		_kings[currentPieceOnsquare.color] = static_cast<int>(EMPTY);
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
		_board[squareTo0x88(square::e1)].type == KING &&
		_board[squareTo0x88(square::e1)].color == WHITE;
	const bool blackKingInPlace =
		!_board.empty() &&
		_board[squareTo0x88(square::e8)].type == KING &&
		_board[squareTo0x88(square::e8)].color == BLACK;
	if (!whiteKingInPlace || _board[squareTo0x88(square::a1)].type != ROOK || _board[squareTo0x88(square::a1)].color != WHITE) {
		_castling.at(color::w) &= ~BITS.at("QSIDE_CASTLE");
	}
	if (!whiteKingInPlace || _board[squareTo0x88(square::h1)].type != ROOK || _board[squareTo0x88(square::h1)].color != WHITE) {
		_castling.at(color::w) &= ~BITS.at("KSIDE_CASTLE");
	}
	if (!blackKingInPlace || _board[squareTo0x88(square::a8)].type != ROOK || _board[squareTo0x88(square::a8)].color != BLACK) {
		_castling.at(color::b) &= ~BITS.at("QSIDE_CASTLE");
	}
	if (!blackKingInPlace || _board[squareTo0x88(square::h8)].type != ROOK || _board[squareTo0x88(square::h8)].color != BLACK) {
		_castling.at(color::b) &= ~BITS.at("KSIDE_CASTLE");
	}
}

void Chess::chrImpl::_updateEnPassantSquare() {
	if (_epSquare == static_cast<int>(EMPTY)) return;

	const square startsquare = static_cast<square>(_epSquare + (_turn == WHITE ? -16 : 16));
	const square currentsquare = static_cast<square>(_epSquare + (_turn == WHITE ? -16 : 16));

	const std::array<int, 2> attackers = { static_cast<int>(currentsquare) + 1, static_cast<int>(currentsquare) - 1 };

	const piece stsq = _board[static_cast<int>(startsquare)];
	const piece epsq = _board[_epSquare];
	if (stsq || epsq ||
		_board[static_cast<int>(currentsquare)].color != swapColor(_turn) ||
		_board[static_cast<int>(currentsquare)].type != PAWN) {
		_epSquare = static_cast<int>(EMPTY);
		return;
	}

	auto canCapture = [&](int square) -> bool {
		return !(square & 0x88) &&
			_board[square].color == _turn &&
			_board[square].type == PAWN;
		};
	if (std::any_of(attackers.begin(), attackers.end(), canCapture)) {
		_epSquare = static_cast<int>(EMPTY);
	}
}

bool Chess::chrImpl::_attacked(color c, int sq) {
	for (int i = squareTo0x88(square::a8); i <= squareTo0x88(square::h1); i++) {
		if (i & 0x88) {
			i += 7;
			continue;
		}
		piece currentSq = _board[i];
		if (!currentSq || currentSq.color != c)
			continue;
		const piece p = currentSq;
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
				piece pos = _board[j];
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

bool Chess::chrImpl::_isKingAttacked(color c) {
	const int sq = _kings[c];
	return static_cast<int>(sq) == -1 ? false : _attacked(swapColor(c), sq);
}

std::vector<internalMove> Chess::chrImpl::_moves(bool legal, pieceSymbol p, std::string sq) {
	std::vector<internalMove> moves;
	color us = _turn;
	color them = swapColor(us);

	square forSquare = !sq.empty() ? stringToSquare(sq) : EMPTY;
	pieceSymbol forPiece = p;

	int firstSquare = squareTo0x88(square::a8);
	int lastSquare = squareTo0x88(square::h1);
	bool singleSquare = false;

	if (forSquare != EMPTY) {
		if (!isValid8x8(forSquare)) {
			return {};
		}
		else {
			firstSquare = squareTo0x88(forSquare);
			lastSquare = firstSquare;
			singleSquare = true;
		}
	}

	for (int from = firstSquare; from <= lastSquare; from++) {
		if (from & 0x88) {
			from += 7;
			continue;
		}

		if (!_board[from] || _board[from].color == them)
			continue;

		pieceSymbol type = _board[from].type;

		int to;

		if (type == PAWN) {
			if (forPiece != PNONE && forPiece != type) continue;

			to = from + PAWN_OFFSETS.at(us)[0];
			if (!_board[to]) {
				addMove(moves, us, from, to, PAWN);

				to = from + PAWN_OFFSETS.at(us)[1];
				if (SECOND_RANK.at(us) == rank(from) && !_board[to]) {
					addMove(moves, us, from, to, PAWN, PNONE, BITS.at("BIG_PAWN"));
				}
			}

			for (int j = 2; j < 4; j++) {
				to = from + PAWN_OFFSETS.at(us)[j];
				if (to & 0x88) continue;

				if (_board[to] && _board[to].color == them) {
					addMove(
						moves,
						us,
						from,
						to,
						PAWN,
						_board[to].type,
						BITS.at("CAPTURE")
					);
				}
				else if (to == _epSquare) {
					addMove(moves, us, from, to, PAWN, PAWN, BITS.at("EP_CAPTURE"));
				}
			}
		}
		else {
			if (forPiece != PNONE && forPiece != type) continue;

			for (int j = 0; j < static_cast<int>(PIECE_OFFSETS.at(type).size()); j++) {
				const int offset = PIECE_OFFSETS.at(type)[j];
				to = from;

				while (true) {
					to += offset;
					if (to & 0x88) break;

					if (!_board[to]) {
						addMove(moves, us, from, to, type);
					}
					else {
						if (_board[to].color == us) break;

						addMove(
							moves,
							us,
							from,
							to,
							type,
							_board[to].type,
							BITS.at("CAPTURE")
						);
						break;
					}

					if (type == KNIGHT || type == KING) break;
				}
			}
		}
	}

	if (forPiece == PNONE || forPiece == KING) {
		if (!singleSquare || lastSquare == _kings[us]) {
			if (_castling.at(us) & BITS.at("KSIDE_CASTLE")) {
				const int castlingFrom = _kings[us];
				const int castlingTo = castlingFrom + 2;

				if (
					!_board[castlingFrom + 1] &&
					!_board[castlingTo] &&
					!_attacked(them, _kings[(us)]) &&
					!_attacked(them, castlingFrom + 1) &&
					!_attacked(them, castlingTo)
				) {
					addMove(
						moves,
						us,
						_kings[us],
						castlingTo,
						KING,
						PNONE,
						BITS.at("KSIDE_CASTLE")
					);
				}
			}

			if (_castling[us] & BITS.at("QSIDE_CASTLE")) {
				const int castlingFrom = _kings[us];
				const int castlingTo = castlingFrom - 2;

				if (
					!_board[castlingFrom - 1] &&
					!_board[castlingFrom - 2] &&
					!_board[castlingFrom - 3] &&
					!_attacked(them, _kings[us]) &&
					!_attacked(them, castlingFrom - 1) &&
					!_attacked(them, castlingTo)
				) {
					addMove(
						moves,
						us,
						_kings[us],
						castlingTo,
						KING,
						PNONE,
						BITS.at("QSIDE_CASTLE")
					);
				}
			}
		}
	}

	 // return all pseudo-legal moves (this includes moves that allow the king
	 // to be captured)
	 
	if (!legal || _kings[us] == static_cast<int>(EMPTY)) {
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
			output += std::string(1, static_cast<char>(std::toupper(pieceToChar(m.piece)))) + disambiguator;
		}
		if (m.flags & (BITS.at("CAPTURE") | BITS.at("EP_CAPTURE"))) {
			if (m.piece == PAWN) {
				output += squareToString(algebraic(m.from))[0];
			}
			output += 'x';
		}

		output += squareToString(algebraic(m.to));

		if (m.promotion != PNONE) {
			output += std::string(1, '=') + std::string(1, static_cast<char>(std::toupper(pieceToChar(m.promotion))));
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
		{{color::b, _kings[color::b]}, {color::w, _kings[color::w]}},
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
	_board[m.from] = piece();

	if (m.flags & BITS.at("EP_CAPTURE")) {
		if (_turn == BLACK) {
			_board[m.to - 16] = piece();
		}
		else {
			_board[m.to + 16] = piece();
		}
	}
	if (m.promotion != PNONE) {
		_board[m.to] = { us, m.promotion };
	}
	if (_board[m.to].type == KING) {
		_kings[(us)] = m.to;

		if (m.flags & BITS.at("KSIDE_CASTLE")) {
			const int castlingTo = m.to - 1;
			const int castlingFrom = m.to + 1;
			_board[castlingTo] = _board[castlingFrom];
			_board[castlingFrom] = piece();
		}
		else if (m.flags & BITS.at("QSIDE_CASTLE")) {
			const int castlingTo = m.to + 1;
			const int castlingFrom = m.to - 2;
			_board[castlingTo] = _board[castlingFrom];
			_board[castlingFrom] = piece();
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
	if (m.flags & BITS.at("BIG_PAWN")) {
		if (us == BLACK) {
			_epSquare = m.to - 16;
		}
		else {
			_epSquare = m.to + 16;
		}
	}
	else {
		_epSquare = static_cast<int>(EMPTY);
	}
	if (m.piece == PAWN) {
		_halfMoves = 0;
	}
	else if (m.flags & (BITS.at("CAPTURE") | BITS.at("EP_CAPTURE"))) {
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

internalMove Chess::chrImpl::_undoMove() {
	const History old = _history.back();
	_history.pop_back();

	if (!old) return internalMove();

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
	_board[m.from].type = m.piece;
	_board[m.to] = piece();

	if (m.captured != PNONE) {
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
			_board[m.to] = { them, m.captured };
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
		_board[castlingFrom] = piece();
	}
	return m;
}

internalMove Chess::chrImpl::_moveFromSan(std::string move, bool strict) {
	const std::string cleanMove = strippedSan(move);

	pieceSymbol pieceType = inferPieceType(cleanMove);
	std::vector<internalMove> moves = _moves(true, pieceType);

	for (int i = 0; i < static_cast<int>(moves.size()); i++) {
		if (cleanMove == strippedSan(_moveToSan(moves[i], moves))) {
			return moves[i];
		}
	}

	if (strict) {
		return internalMove();
	}

	std::smatch matches;
	std::string p = "";
	square from = EMPTY;
	square to = EMPTY;
	std::string promotion = "";

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
	moves = _moves(true, (!p.empty()) ? charToSymbol(static_cast<char>(std::tolower(static_cast<char>(p.at(0))))) : pieceType);
	if (to == EMPTY) {
		return internalMove();
	}
	to = stringToSquare(toSq);
	for (int i = 0, len = static_cast<int>(moves.size()); i < len; i++) {
		if (from == EMPTY) {
			std::string moveStr = strippedSan(_moveToSan(moves[i], moves));
			std::string currentMove = replaceSubstring(moveStr, "x", "");
			if (cleanMove == currentMove) {
				return moves[i];
			}
		}
		else if ((p.empty() || charToSymbol(static_cast<char>(std::tolower(p[0]))) == moves[i].piece) &&
			squareTo0x88(from) == moves[i].from && 
			squareTo0x88(to) == moves[i].to &&
			(promotion.empty() || charToSymbol(static_cast<char>(std::tolower(promotion[0]))) == moves[i].promotion)) {
			return moves[i];
		}
		else if (overlyDisambiguated) {
			square sq = algebraic(moves[i].from);
			if ((p.empty() || charToSymbol(static_cast<char>(std::tolower(p[0]))) == moves[i].piece) &&
				squareTo0x88(to) == moves[i].to &&
				(from == sq) &&
				squareTo0x88(from) == moves[i].from &&
				squareTo0x88(to) == moves[i].to &&
				(promotion.empty() || charToSymbol(static_cast<char>(std::tolower(promotion[0]))) == moves[i].promotion)) {
				return moves[i];
			}
		}
	}
	return internalMove();
}

int Chess::chrImpl::_getPositionCount(std::string fen) {
	std::string trimmedFen = trimFen(fen);
	return _positionCount.at(trimmedFen) == 0 ? _positionCount.at(trimmedFen) : 0;
}

void Chess::chrImpl::_incPositionCount(std::string fen) {
	std::string trimmedFen = trimFen(fen);
	if (_positionCount.count(trimmedFen) == 0) {
		_positionCount.insert({ trimmedFen, 0 });
	}
	_positionCount.at(trimmedFen) += 1;
}

void Chess::chrImpl::_decPositionCount(std::string fen) {
	std::string trimmedFen = trimFen(fen);
	if (_positionCount.at(trimmedFen) > 0) {
		_positionCount.at(trimmedFen) = 0;
	}
	else {
		_positionCount.at(trimmedFen) -= 1;
	}
}

move Chess::chrImpl::_makePretty(internalMove uglyMove) {
	std::string prettyFlags = "";
	color c = uglyMove.color;
	pieceSymbol p = uglyMove.piece;
	int from = uglyMove.from;
	int to = uglyMove.to;
	int flags = uglyMove.flags;
	pieceSymbol cpd = uglyMove.captured;
	pieceSymbol promotion = uglyMove.promotion;

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

	if (cpd != PNONE) {
		m.captured = cpd;
	}
	if (promotion != PNONE) {
		m.promotion = promotion;
		m.lan += pieceToChar(promotion);
	}
	return m;
}

void Chess::chrImpl::_pruneComments() {
	std::vector<internalMove> reservedHistory = {};
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
		internalMove m = reservedHistory[reservedHistory.size() - 1];
		if (!m) break;
		_makeMove(m);
		copyComment(ch.fen());
	}
	_comments = currentComments;
}

