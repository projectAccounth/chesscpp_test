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

	std::array<piece, 128> _board;
	color _turn = WHITE;
	std::map<std::string, std::string> _header;
	std::map<color, int> _kings = { { color::w, EMPTY }, { color::b, EMPTY } };
	int _epSquare = -1;
	int _halfMoves = -1;
	int _moveNumber = 0;
	History _history;
	std::map<std::string, std::string> _comments;
	std::map<color, int> _castling = { { color::w, 0 }, { color::b, 0 } };

	std::map<color, int> _positionCount;

	void _updateSetup(std::string fen);

	bool _put(pieceSymbol type, color color, square sq);

	void _updateCastlingRights();

	void _updateEnPassantSquare();

	bool _attacked();

	bool _isKingAttacked();

	void _moves(std::optional<bool> legal = true, std::optional<pieceSymbol> piece = std::nullopt, std::optional<square> square = std::nullopt);
public:
	Chess() {}

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
	std::vector<move> moves(std::true_type, std::optional<square> sq) const;
	std::vector<std::string> moves(std::false_type, std::optional<square> sq) const;
	std::variant<std::vector<std::string>, std::vector<move>>
		moves(const std::optional<bool> verbose, const std::optional<square> sq);

};
	
#endif