#pragma once
#include "privImpls.h"

using namespace privs;

class Chess::chrImpl {
private:
	Chess& ch;
public:
	chrImpl(Chess& c) : ch(c) { _board = std::vector<Piece>(128, Piece()); }

	std::vector<Piece> _board;
	Color _turn = WHITE;
	std::unordered_map<std::string, std::string> _header;
	std::map<Color, int> _kings = { { WHITE, -1 }, { BLACK, -1 } };
	int _epSquare = -1;
	int _halfMoves = 0;
	int _moveNumber = 0;
	std::vector<History> _history;
	std::map<std::string, std::string> _comments;
	std::map<Color, int> _castling = { { Color::w, 0 }, { Color::b, 0 } };

	std::map<std::string, int> _positionCount;

	void _updateSetup(std::string fen);

	bool _put(PieceSymbol type, Color color, Square sq);

	void _updateCastlingRights();

	void _updateEnPassantSquare();

	bool _attacked(Color c, int sq);

	inline bool _isKingAttacked(Color c) {
		const int sq = _kings[c];
		return sq == -1 ? false : _attacked(swapColor(c), sq);
	}

	std::vector<InternalMove> _moves(bool legal = true, PieceSymbol p = PNONE, std::string sq = std::string());

	void _push(InternalMove move);

	void _makeMove(InternalMove move);

	InternalMove _undoMove();

	std::string _moveToSan(InternalMove move, std::vector<InternalMove> moves);

	InternalMove _moveFromSan(std::string move, bool strict = false);

	move _makePretty(InternalMove uglyMove);

	int _getPositionCount(std::string fen);

	void _incPositionCount(std::string fen);

	void _decPositionCount(std::string fen);
};