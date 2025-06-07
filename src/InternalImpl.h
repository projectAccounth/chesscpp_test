#pragma once
#include "Helper.h"
using namespace ChessCpp;
class Chess::chrImpl {
private:
	Chess& ch;
public:
	chrImpl(Chess& c) : ch(c) {}

	KingPositions _kings;
	std::array<Piece, 128> _board;
	Color _turn = WHITE;
	std::map<std::string, std::string> _header;
	int _epSquare = -1;
	int _halfMoves = -1;
	int _moveNumber = 0;
	std::vector<History> _history;
	std::map<std::string, std::string> _comments;

	uint16_t _castlings = 0;

	std::map<std::string, std::optional<int>> _positionCount;

	void _updateSetup(std::string fen);

	bool _put(PieceSymbol type, Color color, Square sq);

	void _updateCastlingRights();

	void _updateEnPassantSquare();

	bool _attacked(Color c, int sq);

	std::vector<PieceSymbol> _getAttackingPiece(Color c, int sq);

	bool _isKingAttacked(Color c);

	std::vector<InternalMove> _moves(const bool& legal = true, const PieceSymbol& piece = PieceSymbol::NONE, const std::string& sq = std::string());

	void _push(const InternalMove& move);

	void _makeMove(const InternalMove& move);

	InternalMove _undoMove();

	std::string _moveToSan(InternalMove move, std::vector<InternalMove> moves);

	std::optional<InternalMove> _moveFromSan(std::string move, bool strict = false);

	Move _makePretty(InternalMove uglyMove);

	int _getPositionCount(std::string fen);

	void _incPositionCount(std::string fen);

	void _decPositionCount(std::string fen);

	void _pruneComments();
};