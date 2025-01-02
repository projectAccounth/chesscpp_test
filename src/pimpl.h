#pragma once
#include "privImpls.h"

class Chess::chrImpl {
private:
	Chess& ch;
public:
	chrImpl(Chess& c) : ch(c) {}

	std::array<piece, 128> _board;
	color _turn = WHITE;
	std::unordered_map<std::string, std::string> _header;
	std::unordered_map<color, int> _kings = { { WHITE, (int)(EMPTY) }, { WHITE, (int)(EMPTY) } };
	int _epSquare = -1;
	int _halfMoves = -1;
	int _moveNumber = 0;
	std::vector<History> _history;
	std::map<std::string, std::string> _comments;
	std::unordered_map<color, int> _castling = { { color::w, 0 }, { color::b, 0 } };

	std::map<std::string, std::optional<int>> _positionCount;

	void _updateSetup(std::string fen);

	bool _put(pieceSymbol type, color color, square sq);

	void _updateCastlingRights();

	void _updateEnPassantSquare();

	bool _attacked(color c, int sq);

	bool _isKingAttacked(color c);

	std::vector<internalMove> _moves(std::optional<bool> legal = true, pieceSymbol piece = PNONE, std::optional<std::string> sq = std::nullopt);

	void _push(internalMove move);

	void _makeMove(internalMove move);

	internalMove _undoMove();

	std::string _moveToSan(internalMove move, std::vector<internalMove> moves);

	internalMove _moveFromSan(std::string move, bool strict = false);

	move _makePretty(internalMove uglyMove);

	int _getPositionCount(std::string fen);

	void _incPositionCount(std::string fen);

	void _decPositionCount(std::string fen);

	void _pruneComments();
};