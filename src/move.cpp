#include "pimpl.h"

// Contains public functions that are "private" apparently

color Chess::turn() {
	return chImpl->_turn;
}

std::optional<piece> Chess::remove(square sq) {
	std::optional<piece> p = get(sq);
	chImpl->_board[Ox88.at(sq)] = std::nullopt;
	if (p && p.value().type == KING) {
		chImpl->_kings[p.value().color] = EMPTY;
	}
	else if (!p) {
		return std::nullopt;
	}

	chImpl->_updateCastlingRights();
	chImpl->_updateEnPassantSquare();
	chImpl->_updateSetup(fen());

	return p.value();
}

std::optional<move> Chess::undo() {
	std::optional<internalMove> m = chImpl->_undoMove();
	if (m) {
		move prettyMove = chImpl->_makePretty(m.value());
		chImpl->_decPositionCount(prettyMove.after);
		return prettyMove;
	}
	return std::nullopt;
}

std::optional<std::string> Chess::squareColor(square sq) {
	if (Ox88.count(sq) > 0) {
		int squ = Ox88.at(sq);
		return (rank(squ) + file(squ)) % 2 == 0 ? "light" : "dark";
	}
	return std::nullopt;
}

std::optional<piece> Chess::get(square sq) {
	return chImpl->_board[static_cast<int>(sq)] ? chImpl->_board[static_cast<int>(sq)] : std::nullopt;
}

move Chess::cmove(const std::variant<std::string, moveOption>& moveArg, bool strict) {
	std::optional<internalMove> moveObj = std::nullopt;
	if (std::holds_alternative<std::string>(moveArg)) {
		moveObj = chImpl->_moveFromSan(std::get<std::string>(moveArg), strict);
	}
	else {
		std::vector<internalMove> moves = chImpl->_moves();
		moveOption o = std::get<moveOption>(moveArg);
		move m = {
			chImpl->_turn,
			stringToSquare(o.from),
			stringToSquare(o.to),
			std::nullopt,
			std::nullopt,
			o.promotion ? std::optional<pieceSymbol>(strPchrs.at(o.promotion.value()[0])) : std::nullopt
		};
		for (int i = 0; i < static_cast<int>(moves.size()); i++) {
			if (
				m.from == algebraic(moves[i].from) &&
				m.to == algebraic(moves[i].to) &&
				(!(moves[i].promotion) || m.promotion == moves[i].promotion)
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
			throw std::runtime_error("Invalid move: from " + std::get<moveOption>(moveArg).from + "to " + std::get<moveOption>(moveArg).to);
	}

	move prettyMove = chImpl->_makePretty(moveObj.value());

	chImpl->_makeMove(moveObj.value());
	chImpl->_incPositionCount(prettyMove.after);
	return prettyMove;
}

int Chess::perft(int depth) {
	const std::vector<internalMove> moves = chImpl->_moves(true); // get all legal moves
	int nodes = 0;
	color c = chImpl->_turn;

	if (depth == 1) return moves.size();
	if (depth == 0) return 1;

	for (int i = 0, len = static_cast<int>(moves.size()); i < len; i++) {
		chImpl->_makeMove(moves[i]);
		nodes += perft(depth - 1);
		chImpl->_undoMove();
	}
	return nodes;
}

std::pair<bool, bool> Chess::getCastlingRights(color c) {
	return {
		(chImpl->_castling[c] & SIDES.at(KING)) != 0,
		(chImpl->_castling[c] & SIDES.at(QUEEN)) != 0
	};
}

void Chess::clear(std::optional<bool> preserveHeaders) {
	chImpl->_board = std::array<std::optional<piece>, 128>();
	chImpl->_kings = { { color::w, EMPTY }, { color::b, EMPTY } };
	chImpl->_turn = WHITE;
	chImpl->_castling = { {color::w, 0}, {color::b, 0} };
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

std::vector<std::vector<std::optional<std::tuple<square, pieceSymbol, color>>>> Chess::board() {
	std::vector<std::vector<std::optional<std::tuple<square, pieceSymbol, color>>>> output = {};
	std::vector<std::optional<std::tuple<square, pieceSymbol, color>>> row = {};

	for (int i = Ox88.at(square::a8); i <= Ox88.at(square::h1); i++) {
		if (!chImpl->_board[i]) {
			row.push_back(std::nullopt);
		}
		else {
			row.push_back(std::tuple<square, pieceSymbol, color>{
				algebraic(i),
					chImpl->_board[i].value().type,
					chImpl->_board[i].value().color
			});
		}
		if ((i + 1) & 0x88) {
			std::vector<std::optional<std::tuple<square, pieceSymbol, color>>> trow;
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

bool Chess::isAttacked(square sq, color attackedBy) {
	return chImpl->_attacked(attackedBy, Ox88.at(sq));
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

		if (chImpl->_board[i].has_value()) {
			pieces.at(chImpl->_board[i].value().type) = pieces.count(chImpl->_board[i].value().type) > 0 ? pieces.at(chImpl->_board[i].value().type) + 1 : 1;
			if (chImpl->_board[i].value().type == BISHOP) {
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

bool Chess::put(pieceSymbol type, color c, square sq) {
	if (chImpl->_put(type, c, sq)) {
		chImpl->_updateCastlingRights();
		chImpl->_updateEnPassantSquare();
		chImpl->_updateSetup(fen());
		return true;
	}
	return false;
}

std::vector<move> Chess::moves(bool verbose, std::optional<std::string> sq, std::optional<pieceSymbol> piece) {
	std::vector<internalMove> generatedMoves = chImpl->_moves(true, piece, sq);

	std::vector<move> result;
	for (const auto& internal: generatedMoves) {
		move mv;

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

std::vector<std::string> Chess::moves() {
	std::vector<internalMove> generatedMoves = chImpl->_moves(true);

	std::vector<std::string> result;

	for (const auto& internal: generatedMoves) {
		result.push_back(chImpl->_moveToSan(internal, generatedMoves));
	}

	return result;
}

std::vector<move> Chess::moves(std::optional<std::string> sq, std::optional<pieceSymbol> piece) {
	return moves(true, sq, piece);
}