#pragma once

/*
* Main include file for chesscpp
* 
* \file chess.h
*/
#ifndef CHESS_H
#define CHESS_H

#include "types.h"
#include "board.h"
class Chess {
private:

	std::array<std::optional<piece>, 128> _board;
	color _turn = WHITE;
	std::map<std::string, std::string> _header;
	std::map<color, int> _kings = { { color::w, EMPTY }, { color::b, EMPTY } };
	int _epSquare = -1;
	int _halfMoves = -1;
	int _moveNumber = 0;
	std::vector<History> _history;
	std::map<std::string, std::string> _comments;
	std::map<color, int> _castling = { { color::w, 0 }, { color::b, 0 } };

	std::map<color, int> _positionCount;

	void _updateSetup(std::string fen);

	bool _put(pieceSymbol type, color color, square sq);

	void _updateCastlingRights();

	void _updateEnPassantSquare();

	bool _attacked(color c, square sq);

	bool _isKingAttacked(color c);

	std::vector<internalMove> _moves(std::optional<bool> legal = true, std::optional<pieceSymbol> piece = std::nullopt, std::optional<square> square = std::nullopt);

	void _push(internalMove move);

	void _makeMove(internalMove move);

	move _undoMove();

	std::string _moveToSan(internalMove move, std::vector<internalMove> moves);

	std::optional<internalMove> _moveFromSan(std::string move, bool strict = false);

	move _makePretty(internalMove uglyMove);
	
	int _getPositionCount(std::string fen);

	void _incPositionCount(std::string fen);

	void _decPositionCount(std::string fen);

	void _pruneComments();
	

public:
	void clear(std::optional<bool> preserveHeaders);

	void removeHeader(std::string key);
	
	/*
	* Loads a chess game with the specified
	* FEN, manually.
	* 
	* fen: string: The FEN to be passed
	* skipValidation: optional<bool>: Skips the FEN validation. Not recommended.
	* preserveHeaders: optional<bool>: Preserve headers.
	*/
	void load(std::string fen, bool skipValidation = false, bool preserveHeaders = false);

	// Constructor.
	Chess(std::string fen) { load(fen); }

	std::string fen();

	void reset();

	int get(square sq);

	bool put(pieceSymbol type, color color, square sq);

	int remove();

	bool isAttacked(square sq, color attackedBy);

	bool isCheck();

	bool inCheck();
	
	bool isCheckmate();

	bool isStalemate();

	bool inSufficientMaterial();

	bool isThreefoldRepetition();

	bool isDraw();

	bool isGameOver();

	std::vector<std::string> moves() const;
	std::vector<std::string> moves(square sq) const;
	std::vector<std::string> moves(square sq, pieceSymbol piece) const;
	std::variant<std::vector<std::string>, std::vector<move>>
		moves(const std::optional <bool> verbose, const std::optional<square> sq);
	void cmove(std::string move, bool strict = false);
	void cmove(std::tuple<std::string, std::string, std::optional<std::string>> moveOptions, bool strict = false);
	move undo();
	std::string pgn(char newline = '\n', int maxWidth = 0);
	std::map<std::string, std::string> header(const std::vector<std::string>& args);
	void loadPgn(std::string pgn, bool strict = false, std::string newlineChr = "\r?\n");

	std::string ascii();

	int perft(int depth);

	color turn();

	std::vector<int> board();

	std::optional<std::string> squareColor(square sq);

	std::vector<std::string> history();
	std::vector<move> historym(bool verbose = false);
	std::vector<std::string> historys(bool verbose = true);

	bool getCastlingRights(color color);

	int moveNumber();
};
	
#endif