/*
* Main include file for chesscpp
* 
* \file chess.h
*/
#ifndef CHESSCPP_H
#define CHESSCPP_H

#ifndef __cplusplus
#error "This library is C++-based. Please use C++ (C++17 or above) for this library."
#endif

#if defined(_MSC_VER) && (_MSC_VER < 1914)
#error "This library requires C++17 or later. Update to Visual Studio 2017 version 15.7 or later."
#elif !defined(_MSC_VER) && (__cplusplus < 201703L)
#error "This library requires C++17 or later. Please use a compatible compiler or use C++17 standard with the --std=c++17 option."
#endif

#include <utility>

#include "exptypes.h"

std::pair<bool, std::string> validateFen(std::string fen);

class Chess {
private:	
	class chrImpl;
	chrImpl* chImpl;
public:
	// Clear.
	void clear(std::optional<bool> preserveHeaders);

	// Remove header with the specified key.
	void removeHeader(std::string key);
	
	/*
	* Loads a chess game with the specified
	* FEN, manually.
	* 
	* fen: string: The FEN to be passed
	* skipValidation: optional<bool>: Skips the FEN validation. Not recommended.
	* preserveHeaders: optional<bool>: Preserve headers.
	*
	* Raises a runtime error when the FEN is invalid.
	*/
	void load(std::string fen, bool skipValidation = false, bool preserveHeaders = false);

	// Constructor.
	Chess(std::string fen);
	Chess();

	// Returns the current FEN.
	std::string fen();

	// Reset the game state.
	void reset();

	/*
	* Returns a piece on the specified square. Returns std::nullopt if nothing is found.
	*/
	std::optional<piece> get(square sq);

	// Puts a piece of a type on the specified square with the specified color.
	bool put(pieceSymbol type, color c, square sq);

	// Removes a piece from a square. Returns std::nullopt if no pieces were removed.
	std::optional<piece> remove(square sq);

	// Checks whether the king is attacked by the other side.
	bool isAttacked(square sq, color attackedBy);

	// Returns a value that indiciates whether the position is in check for the current side.
	bool isCheck();

	// Alias of isCheck().
	bool inCheck();
	
	// Returns a value that indicates whether the position is in checkmate.
	bool isCheckmate();

	// Returns a value that indicates whether the position is in stalemate.
	bool isStalemate();

	// Returns a value that indicates whether the position have enough pieces for a possible checkmate.
	bool inSufficientMaterial();

	// Returns a value that indicates whether the same position was repeated for the 3rd time.
	bool isThreefoldRepetition();

	// Returns a value that indicates whether the game is drawn.
	bool isDraw();

	// Returns a value that indicates whether the game is over or not.
	bool isGameOver();

	// Unimplemented
	std::vector<std::string> moves();

	// Returns the list of moves on a square/of a piece (optional)
	std::vector<move> moves(std::optional<square> sq, std::optional<pieceSymbol> piece, bool verbose);

	/*
	* Moves the specified piece to a specific position on the board.
	* 
	* Parameters:
	* moveArg - variant<string, moveOption>
	* 
	* 
	* moveOption: { string from, string to, optional<string> promotion }
	* from - to: square to move from and to.
	* promotion: Promoting to a specific piece (in case of a pawn promotion move)
	* 
	* string: Move using SAN notation.
	* 
	* Raises an exception on a fail move.
	*/
	move cmove(const std::variant<std::string, moveOption>& moveArg, bool strict = false);
	
	// Undos a move.
	std::optional<move> undo();
	
	// Gets the current PGN of the game. Does not include results of the game.
	std::string pgn(char newline = '\n', int maxWidth = 0);
	
	std::map<std::string, std::string> header(std::vector<std::string> args ...);
	
	/*
	* Loads the specified PGN.
	* 
	* Parameters:
	* 
	* pgn: string : The PGN to load.
	* 
	* strict: bool : Enables the strict parser.
	* 
	* newline: string : the newline character. Should be kept default.
	*/ 
	void loadPgn(std::string pgn, bool strict = false, std::string newlineChr = "\r?\n");
	
	// Returns the current chessboard in ASCII, in White's perspective. Recommended for debugging or console-based chess games.
	std::string ascii();

	// PERFT - Performance Test.
	int perft(int depth);

	// Returns the current turn.
	color turn();

	std::vector<std::vector<std::tuple<square, pieceSymbol, color>>> board();

	// Returns the current square color of the specified square.
	std::optional<std::string> squareColor(square sq);

	// Unimplemented
	std::vector<std::string> history();
	
	// Unimplemented
	std::vector<move> historym(bool verbose = false);
	
	// Unimplemented
	std::vector<std::string> historys(bool verbose = true);

	// Returns a pair of booleans that indicates the rights to castle. First element is for king-side castle, second element is for queen-side.
	std::pair<bool, bool> getCastlingRights(color color);

	int moveNumber();

	~Chess();
};
	
#endif