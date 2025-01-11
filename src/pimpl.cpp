#include "pimpl.h"


/* Class definitions start here */

using namespace privs;

Chess::Chess(std::string fen) : chImpl(new chrImpl(*this)) { load(fen); }
Chess::Chess() : chImpl(new chrImpl(*this)) { load(DEFAULT_POSITION); }
Chess::~Chess() { delete chImpl; }

InternalMove::operator bool() const {
	return !(
		color == Color::NO_COLOR &&
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
		kings == std::map<Color, int>() &&
		turn == Color::NO_COLOR &&
		epSquare == -1 &&
		castling == std::map<Color, int>() &&
		halfMoves == 0 &&
		moveNumber == 0
	);
}

inline bool operator!(PieceSymbol p) {
	return p == PieceSymbol::NO_PIECE;
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

bool Chess::chrImpl::_put(PieceSymbol type, Color color, Square sq) {
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

	const Piece currentPieceOnsquare = _board[squ];

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
		_board[squareTo0x88(Square::e1)].type == KING &&
		_board[squareTo0x88(Square::e1)].color == WHITE;
	const bool blackKingInPlace =
		!_board.empty() &&
		_board[squareTo0x88(Square::e8)].type == KING &&
		_board[squareTo0x88(Square::e8)].color == BLACK;

	auto isRookOnSquare = [&](const Color& c, const Square& sq) {
		return _board[squareTo0x88(sq)].type == ROOK &&
			   _board[squareTo0x88(sq)].color == c;
	};

	if (!whiteKingInPlace || !isRookOnSquare(WHITE, Square::a1)) {
		_castling.at(Color::w) &= ~BITS_QSIDE_CASTLE;
	}
	if (!whiteKingInPlace || !isRookOnSquare(WHITE, Square::h1)) {
		_castling.at(Color::w) &= ~BITS_KSIDE_CASTLE;
	}
	if (!blackKingInPlace || !isRookOnSquare(BLACK, Square::a8)) {
		_castling.at(Color::b) &= ~BITS_QSIDE_CASTLE;
	}
	if (!blackKingInPlace || !isRookOnSquare(BLACK, Square::h8)) {
		_castling.at(Color::b) &= ~BITS_KSIDE_CASTLE;
	}
}



void Chess::chrImpl::_updateEnPassantSquare() {
	if (_epSquare == -1) return;

	const Square startsquare = static_cast<Square>(_epSquare + (_turn == WHITE ? -16 : 16));
	const Square currentsquare = static_cast<Square>(_epSquare + (_turn == WHITE ? -16 : 16));

	const int strsq = static_cast<int>(startsquare);
	const int crsq = static_cast<int>(currentsquare);

	const std::vector<int> attackers = { crsq + 1, crsq - 1 };

	const Piece stsq = _board[strsq];
	const Piece epsq = _board[_epSquare];
	if (stsq || epsq ||
		_board[crsq].color != swapColor(_turn) ||
		_board[crsq].type != PAWN) {
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

bool Chess::chrImpl::_attacked(Color c, int sq) {
	for (int i = 0; i <= 0x77; i++) {
		if (i & 0x88) {
			i += 7; continue;
		}
		Piece piece = _board[i];
		if (!piece || piece.color != c) continue;
		const int diff = i - sq;

		if (diff == 0) continue;

		const int index = diff + 0x77;

		if (ATTACKS[index] & getPieceMasks(piece.type)) {
			if (piece.type == PAWN) {
				if ((diff > 0 && piece.color == WHITE) || (diff < 0 && piece.color == BLACK))
					return true;
				continue;
			}

			if (piece.type == KNIGHT || piece.type == KING) return true;

			const int offset = RAYS[index];
			int j = i + offset;

			bool blocked = false;
			while (j != sq) {
				if (_board[j]) {
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

std::vector<InternalMove> Chess::chrImpl::_moves(bool legal, PieceSymbol p, std::string sq) {
	std::vector<InternalMove> moves;
	Color us = _turn;
	Color them = swapColor(us);
	
	Square forSquare = !sq.empty() ? stringToSquare(sq) : EMPTY;
	PieceSymbol forPiece = p != PNONE ? p : PNONE;

	int firstSquare = 0;
	int lastSquare = 119;
	bool singleSquare = false;

	if (forSquare != EMPTY) {
		if (!isValid8x8(forSquare)) return {};

		firstSquare = lastSquare = squareTo0x88(forSquare);
		singleSquare = true;
	}

	for (int from = firstSquare; from <= lastSquare; from++) {
		if (from & 0x88) { from += 7; continue; }

		if (!_board[from] || _board[from].color == them) continue;

		PieceSymbol type = _board[from].type;

		int to;

		if (type == PAWN) {
			if ((forPiece != PNONE) && (forPiece != type)) continue;

			to = from + getPawnOffsets(us)[0];
			if (!_board[to]) {
				addMove(moves, us, from, to, PAWN);
				to = from + getPawnOffsets(us)[1];

				if (getSecondRank(us) == rank(from) && !_board[to])
					addMove(moves, us, from, to, PAWN, PNONE, BITS_BIG_PAWN);
			}

			for (int j = 2; j < 4; j++) {
				to = from + getPawnOffsets(us)[j];
				if (to & 0x88) continue;

				if (_board[to] && _board[to].color == them)
					addMove(moves, us, from, to, PAWN, _board[to].type, BITS_CAPTURE);
				else if (to == _epSquare)
					addMove(moves, us, from, to, PAWN, PAWN, BITS_EP_CAPTURE);
			}
			continue;
		}
		if (forPiece != PNONE && forPiece != type) continue;

		for (size_t j = 0; j < getPieceOffsets(type).size(); j++) {
			const int offset = getPieceOffsets(type)[j];
			to = from;

			while (true) {
				to += offset;
				if (to & 0x88) break;

				if (!_board[to])
					addMove(moves, us, from, to, type);
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
				const bool canCastleKSide =
					!_board[castlingFrom + 1] &&
					!_board[castlingTo] &&
					!_attacked(them, _kings[us]) &&
					!_attacked(them, castlingFrom + 1) &&
					!_attacked(them, castlingTo);
				if (canCastleKSide)
					addMove(moves, us, _kings[us], castlingTo, KING, PNONE, BITS_KSIDE_CASTLE);
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
				if (canCastleQSide)
					addMove(moves, us, _kings[us], castlingTo, KING, PNONE, BITS_QSIDE_CASTLE);
			}
		}
	}

	 // return all pseudo-legal moves (this includes moves that allow the king
	 // to be captured)
	 
	if (!legal || _kings[us] == -1) return moves;

	std::vector<InternalMove> legalMoves = {};

	for (const auto& m : moves) {
		_makeMove(m);
		if (!_isKingAttacked(us)) legalMoves.push_back(m);
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
			output += std::string(1, static_cast<char>(std::toupper(pieceToChar(m.piece)))) + disambiguator;
		}
		if (m.flags & (BITS_CAPTURE | BITS_EP_CAPTURE)) {
			output += (m.piece == PAWN ? std::string(1, squareToString(algebraic(m.from))[0]) : "") + 'x';
		}

		output += squareToString(algebraic(m.to));

		if (m.promotion != PNONE) {
			output += std::string(1, '=') + std::string(1, static_cast<char>(std::toupper(pieceToChar(m.promotion))));
		}
	}

	_makeMove(m);

	if (ch.isCheck()) output += ch.isCheckmate() ? '#' : '+';
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
	_board[m.from] = Piece();

	if (m.flags & BITS_EP_CAPTURE) {
		if (_turn == BLACK)  _board[m.to - 16] = Piece();
		else _board[m.to + 16] = Piece();
	}
	if (m.promotion != PNONE) _board[m.to] = { us, m.promotion };
	if (_board[m.to].type == KING) {
		_kings[(us)] = m.to;

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
	if (m.flags & BITS_BIG_PAWN)
		_epSquare = m.to + (us == BLACK ? -16 : 16);
	else
		_epSquare = -1;

	if (m.piece == PAWN)
		_halfMoves = 0;
	else if (m.flags & (BITS_CAPTURE | BITS_EP_CAPTURE))
		_halfMoves = 0;
	else
		_halfMoves++;

	if (us == BLACK)
		_moveNumber++;
	_turn = them;
}

InternalMove Chess::chrImpl::_undoMove() {
	const History old = _history.back();
	_history.pop_back();

	if (!old) return InternalMove();

	const InternalMove m = old.move;

	_kings = old.kings;
	_turn = old.turn;
	_castling = old.castling;
	_epSquare = old.epSquare;
	_halfMoves = old.halfMoves;
	_moveNumber = old.moveNumber;

	const Color us = _turn;
	const Color them = swapColor(us);

	_board[m.from] = _board[m.to];
	_board[m.from].type = m.piece;
	_board[m.to] = Piece();

	if (m.captured != PNONE) {
		if (m.flags & BITS_EP_CAPTURE) {
			int index = m.to + (us == BLACK ? 16 : -16);
			_board[index] = { them, PAWN };
		}
		else _board[m.to] = { them, m.captured };
	}
	if (m.flags & (BITS_KSIDE_CASTLE | BITS_QSIDE_CASTLE)) {
		int castlingTo = (m.flags & BITS_KSIDE_CASTLE) ? m.to + 1 : m.to - 2;
		int castlingFrom = (m.flags & BITS_KSIDE_CASTLE) ? m.to - 1 : m.to + 1;

		_board[castlingTo] = _board[castlingFrom];
		_board[castlingFrom] = Piece();
	}
	return m;
}

InternalMove Chess::chrImpl::_moveFromSan(std::string move, bool strict) {
	const std::string cleanMove = strippedSan(move);

	PieceSymbol pieceType = inferPieceType(cleanMove);
	std::vector<InternalMove> moves = _moves(true, pieceType);

	for (int i = 0; i < static_cast<int>(moves.size()); i++) {
		if (cleanMove == strippedSan(_moveToSan(moves[i], moves))) {
			return moves[i];
		}
	}

	if (strict) {
		return InternalMove();
	}

	std::smatch matches;
	std::string p = "";
	Square from = EMPTY;
	Square to = EMPTY;
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
		return InternalMove();
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
			Square sq = algebraic(moves[i].from);
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
	return InternalMove();
}

int Chess::chrImpl::_getPositionCount(std::string fen) {
	return _positionCount.count(trimFen(fen)) != 0 ? _positionCount.at(trimFen(fen)) : 0;
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

move Chess::chrImpl::_makePretty(InternalMove uglyMove) {
	std::string prettyFlags = "";
	Color c = uglyMove.color;
	PieceSymbol p = uglyMove.piece;
	int from = uglyMove.from;
	int to = uglyMove.to;
	int flags = uglyMove.flags;
	PieceSymbol cpd = uglyMove.captured;
	PieceSymbol promotion = uglyMove.promotion;

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

	const Square fromAlgebraic = algebraic(from);
	const Square toAlgebraic = algebraic(to);

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

