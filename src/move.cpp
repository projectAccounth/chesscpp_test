#include "pimpl.h"

// Contains public functions that are "private" apparently

using namespace privs;

Color Chess::turn() {
	return chImpl->_turn;
}

Piece Chess::remove(Square sq) {
	Piece p = get(sq);
	chImpl->_board[Ox88.at((int)(sq))] = Piece();
	if (p && p.type == KING) {
		chImpl->_kings[p.color] = -1;
	}
	else if (!p) {
		return Piece();
	}

	chImpl->_updateCastlingRights();
	chImpl->_updateEnPassantSquare();
	chImpl->_updateSetup(fen());

	return p;
}

move Chess::undo() {
	InternalMove m = chImpl->_undoMove();
	if (m) {
		move prettyMove = chImpl->_makePretty(m);
		chImpl->_decPositionCount(prettyMove.after);
		return prettyMove;
	}
	return move();
}

std::string Chess::squareColor(Square sq) {
	if (isValid8x8(sq)) {
		int squ = Ox88.at((int)(sq));
		return (rank(squ) + file(squ)) % 2 == 0 ? "light" : "dark";
	}
	return "";
}

Piece Chess::get(Square sq) {
	int sqi = static_cast<int>(sq);
	return chImpl->_board[sqi] ? chImpl->_board[sqi] : Piece();
}

move Chess::cmove(const std::string& moveArg, bool strict) {
	InternalMove moveObj = InternalMove();
	moveObj = chImpl->_moveFromSan(moveArg, strict);
	if (!moveObj) {
		throw std::runtime_error("Invalid move: " + moveArg);
	}

	move prettyMove = chImpl->_makePretty(moveObj);

	chImpl->_makeMove(moveObj);
	chImpl->_incPositionCount(prettyMove.after);
	return prettyMove;
}


move Chess::cmove(const moveOption& moveArg) {
	InternalMove moveObj = InternalMove();
	std::vector<InternalMove> moves = chImpl->_moves();
	move m = {
		chImpl->_turn,
		stringToSquare(moveArg.from),
		stringToSquare(moveArg.to),
		PNONE,
		PNONE,
		!moveArg.promotion.empty() ? PieceSymbol(charToSymbol(moveArg.promotion[0])) : PNONE
	};
	for (int i = 0; i < static_cast<int>(moves.size()); i++) {
		if (
			m.from == algebraic(moves[i].from) &&
			m.to == algebraic(moves[i].to) &&
			((moves[i].promotion == PNONE) || m.promotion == moves[i].promotion)
			) {
			moveObj = moves[i];
			break;
		}
	}

	if (!moveObj) {
		throw std::runtime_error("Invalid move: from " + moveArg.from + "to " + moveArg.to);
	}

	move prettyMove = chImpl->_makePretty(moveObj);

	chImpl->_makeMove(moveObj);
	chImpl->_incPositionCount(prettyMove.after);
	return prettyMove;
}

unsigned long long Chess::perft(int depth) {
	if (depth == 1) return chImpl->_moves(true).size();

	const auto moves = chImpl->_moves(false);
	unsigned long long nodes = 0;
	Color us = chImpl->_turn;

	for (const auto& m : moves) {
		chImpl->_makeMove(m);
		if (!chImpl->_isKingAttacked(us))
			nodes += perft(depth - 1);
		chImpl->_undoMove();
	}
	return nodes;
}

std::pair<bool, bool> Chess::getCastlingRights(Color c) {
	return {
		(chImpl->_castling[c] & getCastlingSide(KING)) != 0,
		(chImpl->_castling[c] & getCastlingSide(QUEEN)) != 0
	};
}

void Chess::clear(bool preserveHeaders) {
	chImpl->_board = std::vector<Piece>(128, Piece());
	chImpl->_kings = { { Color::w, -1 }, { Color::b, -1 } };
	chImpl->_turn = WHITE;
	chImpl->_castling = { {Color::w, 0}, {Color::b, 0} };
	chImpl->_epSquare = -1;
	chImpl->_halfMoves = 0;
	chImpl->_moveNumber = 1;
	chImpl->_history = {};
	chImpl->_comments = {};
	chImpl->_header = preserveHeaders ? chImpl->_header : std::unordered_map<std::string, std::string>();
	chImpl->_positionCount = {};

	chImpl->_header.erase("SetUp");
	chImpl->_header.erase("FEN");
}

void Chess::removeHeader(std::string key) {
	if (chImpl->_header.count(key) > 0) {
		chImpl->_header.erase(key);
	}
}

std::unordered_map<std::string, std::string> Chess::header(std::vector<std::string> args ...) {
	for (int i = 0; i < static_cast<int>(args.size()); i += 2) {
		chImpl->_header[args[i]] = args[i + 1];
	}
	return chImpl->_header;
}

std::vector<std::vector<std::tuple<Square, PieceSymbol, Color>>> Chess::board() {
	std::vector<std::vector<std::tuple<Square, PieceSymbol, Color>>> output = {};
	std::vector<std::tuple<Square, PieceSymbol, Color>> row = {};

	for (int i = 0; i <= 119; i++) {
		if (!chImpl->_board[i]) {
			row.push_back({ EMPTY, PNONE, Color::NO_COLOR });
		}
		else {
			row.push_back(std::tuple<Square, PieceSymbol, Color>{
				algebraic(i),
				chImpl->_board[i].type,
				chImpl->_board[i].color
			});
		}
		if ((i + 1) & 0x88) {
			std::vector<std::tuple<Square, PieceSymbol, Color>> trow;
			for (const auto& elem : row) {
				bool hasValue = (std::get<Square>(elem) != EMPTY && std::get<PieceSymbol>(elem) != PNONE && std::get<Color>(elem) != Color::NO_COLOR);
				if (hasValue) {
					trow.push_back(elem);
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
		{PieceSymbol::b, 0},
		{PieceSymbol::n, 0},
		{PieceSymbol::r, 0},
		{PieceSymbol::q, 0},
		{PieceSymbol::k, 0},
		{PieceSymbol::p, 0},
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
			auto p = chImpl->_board[i].type;
			pieces.at(p) = pieces.count(p) > 0 ? pieces.at(p) + 1 : 1;
			if (chImpl->_board[i].type == BISHOP) {
				bishops.push_back(squareColor);
			}
			numPieces++;
		}
	}

	if (numPieces == 2) {
		return true;
	}
	else if (numPieces == 3 &&
		(pieces.at(BISHOP) == 1 ||
		 pieces.at(KNIGHT) == 1)) {
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

std::vector<move> Chess::moves(bool verbose, std::string sq, PieceSymbol piece) {
	std::vector<InternalMove> generatedMoves = chImpl->_moves(true, piece, sq);

	std::vector<move> result;
	for (const auto& internal: generatedMoves) {
		move mv;

		mv.color = internal.color;
		mv.from = algebraic(internal.from);
		mv.to = algebraic(internal.to);
		mv.piece = internal.piece;
		mv.captured = internal.captured;
		mv.promotion = internal.promotion;
		mv.flags = static_cast<char>(internal.flags);

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

std::vector<std::string> Chess::moves() {
	std::vector<InternalMove> generatedMoves = chImpl->_moves(true);

	std::vector<std::string> result;

	for (const auto& internal: generatedMoves) {
		result.push_back(chImpl->_moveToSan(internal, generatedMoves));
	}

	return result;
}

std::vector<move> Chess::moves(std::string sq, PieceSymbol piece) {
	return moves(true, sq, piece);
}