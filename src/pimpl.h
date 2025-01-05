#pragma once
#include "privImpls.h"

using namespace privs;

class Chess::chrImpl {
private:
	Chess& ch;
public:
	chrImpl(Chess& c) : ch(c) { _board = std::vector<piece>(128, piece()); }

	std::vector<piece> _board;
	color _turn = WHITE;
	std::unordered_map<std::string, std::string> _header;
	std::map<color, int> _kings = { { WHITE, -1 }, { BLACK, -1 } };
	int _epSquare = -1;
	int _halfMoves = 0;
	int _moveNumber = 0;
	std::vector<History> _history;
	std::map<std::string, std::string> _comments;
	std::map<color, int> _castling = { { color::w, 0 }, { color::b, 0 } };

	std::map<std::string, int> _positionCount;

	void _updateSetup(std::string fen);

	bool _put(pieceSymbol type, color color, square sq);

	void _updateCastlingRights();

	void _updateEnPassantSquare();

	inline bool _attacked(color c, int sq) {
		for (int i = 0; i <= 119; i++) {
			if (i & 0x88) { i += 7; continue; }
			piece currentSq = _board[i];
			if (!currentSq || currentSq.color != c) continue;
			const int diff = i - sq;

			if (diff == 0) continue;

			if (ATTACKS[diff + 119] & getPieceMasks(currentSq.type)) {
				if (currentSq.type == PAWN) {
					if (diff > 0) if (currentSq.color == WHITE) return true;
					else { if (currentSq.color == BLACK) return true; }
					continue;
				}

				if (currentSq.type == KNIGHT || currentSq.type == KING) return true;

				const int offset = RAYS[diff + 119];
				int j = i + offset;

				bool blocked = false;
				while (j != sq) {
					if (_board[j]) { blocked = true; break; }
					j += offset;
				}
				if (!blocked) return true;
			}
		}
		return false;
	}

	inline bool _isKingAttacked(color c) {
		const int sq = _kings[c];
		return sq == -1 ? false : _attacked(swapColor(c), sq);
	}

	std::vector<internalMove> _moves(bool legal = true, pieceSymbol p = PNONE, std::string sq = std::string());

	void _push(internalMove move);

	void _makeMove(internalMove move);

	internalMove _undoMove();

	std::string _moveToSan(internalMove move, std::vector<internalMove> moves);

	internalMove _moveFromSan(std::string move, bool strict = false);

	move _makePretty(internalMove uglyMove);

	int _getPositionCount(std::string fen);

	void _incPositionCount(std::string fen);

	void _decPositionCount(std::string fen);
};