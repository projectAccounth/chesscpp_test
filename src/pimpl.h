#pragma once
#include "privImpls.h"

class Chess::chrImpl {
private:
	Chess& ch;
public:
	chrImpl(Chess& c) : ch(c) {}

	std::array<std::optional<Piece>, 128> _board;
	Color _turn = WHITE;
	std::map<std::string, std::string> _header;
	std::map<Color, int> _kings = { { WHITE, EMPTY }, { BLACK, EMPTY } };
	int _epSquare = -1;
	int _halfMoves = -1;
	int _moveNumber = 0;
	std::vector<History> _history;
	std::map<std::string, std::string> _comments;
	std::map<Color, int> _castling = { { WHITE, 0 }, { BLACK, 0 } };

	std::map<std::string, std::optional<int>> _positionCount;

	void _updateSetup(std::string fen);

	bool _put(PieceSymbol type, Color color, Square sq);

	void _updateCastlingRights();

	void _updateEnPassantSquare();

	bool _attacked(Color c, int sq);

	std::vector<std::optional<PieceSymbol>> _getAttackingPiece(Color c, int sq);

	bool _isKingAttacked(Color c);

	std::vector<InternalMove> _moves(std::optional<bool> legal = true, std::optional<PieceSymbol> piece = std::nullopt, std::optional<std::string> sq = std::nullopt);

	void _push(InternalMove move);

	void _makeMove(InternalMove move);

	std::optional<InternalMove> _undoMove();

	std::string _moveToSan(InternalMove move, std::vector<InternalMove> moves);

	std::optional<InternalMove> _moveFromSan(std::string move, bool strict = false);

	move _makePretty(InternalMove uglyMove);

	int _getPositionCount(std::string fen);

	void _incPositionCount(std::string fen);

	void _decPositionCount(std::string fen);

	void _pruneComments();
};