#include "pimpl.h"


/* Class definitions start here */

using namespace privs;

Chess::Chess(std::string fen) : chImpl(new chrImpl(*this)) { load(fen); }
Chess::Chess() : chImpl(new chrImpl(*this)) { load(DEFAULT_POSITION); }
Chess::~Chess() { delete chImpl; }

internalMove::operator bool() const {
	return !(
		color == color::NO_COLOR &&
		from == -1 &&
		to == -1 &&
		piece == PNONE &&
		captured == PNONE &&
		promotion == PNONE &&
		flags == 0
	);
}

History::operator bool() const {
	return !(
		!move &&
		kings == std::map<color, int>() &&
		turn == color::NO_COLOR &&
		epSquare == -1 &&
		castling == std::map<color, int>() &&
		halfMoves == 0 &&
		moveNumber == 0
	);
}

inline bool operator!(pieceSymbol p) {
	return p == pieceSymbol::NO_PIECE;
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
	if (SYMBOLS.find(static_cast<char>(std::tolower(pieceToChar(type)))) == std::string::npos) {
		return false;
	}
	if (!isValid8x8(sq)) {
		return false;
	}
	const int squ = squareTo0x88(sq);

	if (type == KING && !(_kings[color] == -1 || _kings[color] == squ)) {
		return false;
	}

	const piece currentPieceOnsquare = _board[squ];

	if (currentPieceOnsquare && currentPieceOnsquare.type == KING) {
		_kings[currentPieceOnsquare.color] = -1;
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
		_castling.at(color::w) &= ~BITS_QSIDE_CASTLE;
	}
	if (!whiteKingInPlace || _board[squareTo0x88(square::h1)].type != ROOK || _board[squareTo0x88(square::h1)].color != WHITE) {
		_castling.at(color::w) &= ~BITS_KSIDE_CASTLE;
	}
	if (!blackKingInPlace || _board[squareTo0x88(square::a8)].type != ROOK || _board[squareTo0x88(square::a8)].color != BLACK) {
		_castling.at(color::b) &= ~BITS_QSIDE_CASTLE;
	}
	if (!blackKingInPlace || _board[squareTo0x88(square::h8)].type != ROOK || _board[squareTo0x88(square::h8)].color != BLACK) {
		_castling.at(color::b) &= ~BITS_KSIDE_CASTLE;
	}
}

void Chess::chrImpl::_updateEnPassantSquare() {
	if (_epSquare == -1) return;

	const square startsquare = static_cast<square>(_epSquare + (_turn == WHITE ? -16 : 16));
	const square currentsquare = static_cast<square>(_epSquare + (_turn == WHITE ? -16 : 16));

	const std::array<int, 2> attackers = { static_cast<int>(currentsquare) + 1, static_cast<int>(currentsquare) - 1 };

	const piece stsq = _board[static_cast<int>(startsquare)];
	const piece epsq = _board[_epSquare];
	if (stsq || epsq ||
		_board[static_cast<int>(currentsquare)].color != swapColor(_turn) ||
		_board[static_cast<int>(currentsquare)].type != PAWN) {
		_epSquare = -1;
		return;
	}

	auto canCapture = [&](int square) -> bool {
		return !(square & 0x88) &&
			_board[square].color == _turn &&
			_board[square].type == PAWN;
		};
	if (std::any_of(attackers.begin(), attackers.end(), canCapture)) {
		_epSquare = -1;
	}
}

std::vector<internalMove> Chess::chrImpl::_moves(bool legal, pieceSymbol p, std::string sq) {
	std::vector<internalMove> moves;
	color us = _turn;
	color them = swapColor(us);
	
	square forSquare = !sq.empty() ? stringToSquare(sq) : EMPTY;
	pieceSymbol forPiece = p != PNONE ? p : PNONE;

	int firstSquare = 0;
	int lastSquare = 119;
	bool singleSquare = false;

	if (forSquare != EMPTY) {
		if (!isValid8x8(forSquare)) return {};

		firstSquare = squareTo0x88(forSquare);
		lastSquare = firstSquare;
		singleSquare = true;
	}

	for (int from = firstSquare; from <= lastSquare; from++) {
		if (from & 0x88) { from += 7; continue; }

		if (!_board[from] || _board[from].color == them) continue;

		pieceSymbol type = _board[from].type;

		int to;

		if (type == PAWN) {
			if ((forPiece != PNONE) && (forPiece != type)) continue;

			to = from + getPawnOffsets(us)[0];
			if (!_board[to]) {
				addMove(moves, us, from, to, PAWN);

				to = from + getPawnOffsets(us)[1];
				if (getSecondRank(us) == rank(from) && !_board[to]) addMove(moves, us, from, to, PAWN, PNONE, BITS_BIG_PAWN);
			}

			for (int j = 2; j < 4; j++) {
				to = from + getPawnOffsets(us)[j];
				if (to & 0x88) continue;

				if (_board[to] && _board[to].color == them) addMove(moves, us, from, to, PAWN, _board[to].type, BITS_CAPTURE);
				else if (to == _epSquare) addMove(moves, us, from, to, PAWN, PAWN, BITS_EP_CAPTURE);
			}
			continue;
		}
		if (forPiece != PNONE && forPiece != type) continue;

		for (int j = 0; j < static_cast<int>(getPieceOffsets(type).size()); j++) {
			const int offset = getPieceOffsets(type)[j];
			to = from;

			while (true) {
				to += offset;
				if (to & 0x88) break;

				if (!_board[to]) addMove(moves, us, from, to, type);
				else {
					if (_board[to].color == us) break;
					addMove(moves, us, from, to, type, _board[to].type, BITS_CAPTURE);
					break;
				}

				if (type == KNIGHT || type == KING) break;
			}
		}
	}

	if (!forPiece || forPiece == KING) {
		if (!singleSquare || lastSquare == _kings[us]) {
			if (_castling[us] & BITS_KSIDE_CASTLE) {
				const int castlingFrom = _kings[us];
				const int castlingTo = castlingFrom + 2;
				const bool kSideCastle =
					!_board[castlingFrom + 1] &&
					!_board[castlingTo] &&
					!_attacked(them, _kings[(us)]) &&
					!_attacked(them, castlingFrom + 1) &&
					!_attacked(them, castlingTo);
				if (kSideCastle)
					addMove(moves, us, _kings[us], castlingTo, KING, PNONE, BITS_KSIDE_CASTLE);
			}

			if (_castling[us] & BITS_QSIDE_CASTLE) {
				const int castlingFrom = _kings[us];
				const int castlingTo = castlingFrom - 2;
				const bool qSideCastle =
					!_board[castlingFrom - 1] &&
					!_board[castlingFrom - 2] &&
					!_board[castlingFrom - 3] &&
					!_attacked(them, _kings[us]) &&
					!_attacked(them, castlingFrom - 1) &&
					!_attacked(them, castlingTo);
				if (qSideCastle)
					addMove(moves, us, _kings[us], castlingTo, KING, PNONE, BITS_QSIDE_CASTLE);
			}
		}
	}

	 // return all pseudo-legal moves (this includes moves that allow the king
	 // to be captured)
	 
	if (!legal || _kings[us] == -1) return moves;

	std::vector<internalMove> legalMoves = {};

	for (const auto& m : moves) {
		_makeMove(m);
		if (!_isKingAttacked(us)) legalMoves.push_back(m);
		_undoMove();
	}

	return legalMoves;
}

std::string Chess::chrImpl::_moveToSan(internalMove m, std::vector<internalMove> moves) {
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
			output += std::string(1, static_cast<char>(std::toupper(pieceToChar(m.piece)))) + disambiguator;
		}
		if (m.flags & (BITS_CAPTURE | BITS_EP_CAPTURE)) {
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

	if (m.flags & BITS_EP_CAPTURE) {
		if (_turn == BLACK)  _board[m.to - 16] = piece();
		else _board[m.to + 16] = piece();
	}
	if (m.promotion != PNONE) _board[m.to] = { us, m.promotion };
	if (_board[m.to].type == KING) {
		_kings[(us)] = m.to;

		if (m.flags & BITS_KSIDE_CASTLE) {
			const int castlingTo = m.to - 1;
			const int castlingFrom = m.to + 1;
			_board[castlingTo] = _board[castlingFrom];
			_board[castlingFrom] = piece();
		}
		else if (m.flags & BITS_QSIDE_CASTLE) {
			const int castlingTo = m.to + 1;
			const int castlingFrom = m.to - 2;
			_board[castlingTo] = _board[castlingFrom];
			_board[castlingFrom] = piece();
		}

		_castling.at(us) = 0;
	}
	if (_castling.at(us)) {
		for (int i = 0; i < static_cast<int>(getRookInf(us).size()); i++) {
			bool uCastling = m.from == getRookInf(us)[i].square &&
				_castling.at(us) & getRookInf(us)[i].flag;
			if (uCastling) {
				_castling.at(us) ^= getRookInf(us)[i].flag;
				break;
			}
		}
	}
	if (_castling.at(them)) {
		for (int i = 0; i < static_cast<int>(getRookInf(us).size()); i++) {
			bool uCastling = m.from == getRookInf(them)[i].square &&
				_castling.at(them) & getRookInf(them)[i].flag;
			if (uCastling) {
				_castling.at(them) ^= getRookInf(them)[i].flag;
				break;
			}
		}
	}
	if (m.flags & BITS_BIG_PAWN) _epSquare = m.to + (us == BLACK ? -16 : 16);
	else _epSquare = -1;

	if (m.piece == PAWN) _halfMoves = 0;
	else if (m.flags & (BITS_CAPTURE | BITS_EP_CAPTURE)) _halfMoves = 0;
	else _halfMoves++;

	if (us == BLACK) _moveNumber++;
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
		if (m.flags & BITS_EP_CAPTURE) {
			int index = m.to + (us == BLACK ? 16 : -16);
			_board[index] = { them, PAWN };
		}
		else _board[m.to] = { them, m.captured };
	}
	if (m.flags & (BITS_KSIDE_CASTLE | BITS_QSIDE_CASTLE)) {
		int castlingTo = m.flags & BITS_KSIDE_CASTLE ? m.to + 1 : m.to - 2;
		int castlingFrom = m.flags & BITS_KSIDE_CASTLE ? m.to - 1 : m.to + 1;

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
	moves = _moves(true, (!p.empty() ? charToSymbol((char)std::tolower(p.at(0))) : pieceType));
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
	return _positionCount.at(trimFen(fen)) == 0 ? _positionCount.at(trimFen(fen)) : 0;
}

void Chess::chrImpl::_incPositionCount(std::string fen) {
	if (_positionCount.count(trimFen(fen)) == 0) {
		_positionCount.insert({ trimFen(fen), 0 });
	}
	_positionCount.at(trimFen(fen)) += 1;
}

void Chess::chrImpl::_decPositionCount(std::string fen) {
	int posCnt = _positionCount.at(trimFen(fen));
	posCnt = posCnt > 0 ? posCnt = 0 : posCnt--;
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

	const std::vector<int> BITS = { BITS_BIG_PAWN, BITS_CAPTURE, BITS_EP_CAPTURE, BITS_KSIDE_CASTLE, BITS_NORMAL, BITS_PROMOTION, BITS_QSIDE_CASTLE };
	for (auto& val : BITS) {
		if (val & flags) {
			switch (val) {
			case BITS_BIG_PAWN: prettyFlags += FLAGS_BIG_PAWN; break;
			case BITS_EP_CAPTURE: prettyFlags += FLAGS_EP_CAPTURE; break;
			case BITS_CAPTURE: prettyFlags += FLAGS_CAPTURE; break;
			case BITS_KSIDE_CASTLE: prettyFlags += FLAGS_KSIDE_CASTLE; break;
			case BITS_QSIDE_CASTLE: prettyFlags += FLAGS_QSIDE_CASTLE; break;
			case BITS_PROMOTION: prettyFlags += FLAGS_PROMOTION; break;
			case BITS_NORMAL: prettyFlags += FLAGS_NORMAL; break;
			}
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
		internalMove m = reservedHistory.back();
		if (!m) break;
		_makeMove(m);
		copyComment(ch.fen());
	}
	_comments = currentComments;
}

