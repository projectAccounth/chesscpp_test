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

int file(int square);

int rank(int square);

square stringToSquare(const std::string& squareStr);

std::string squareToString(square sq);

square algebraic(int square);

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

	// Constructor, with optional FEN.
	Chess(std::string fen);
	// Default constructor, default FEN is loaded.
	Chess();

	// Returns the current FEN of the board.
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

	// Returns all available moves.
	std::vector<std::string> moves();

	// Returns the list of moves on a square/of a piece (optional)
	std::vector<move> moves(bool verbose, std::optional<std::string> sq, std::optional<pieceSymbol> piece);

	std::vector<move> moves(std::optional<std::string> sq, std::optional<pieceSymbol> piece);
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
	void loadPgn(std::string pgn, bool strict = false, std::string newlineChr = "\\r?\\n");
	
	// Returns the current chessboard in ASCII, in White's perspective by default. Recommended for debugging or console-based chess games.
	std::string ascii(bool isWhitePersp = true);

	// PERFT - Performance Test.
	int perft(int depth);

	// Returns the current turn, Black or White.
	color turn();

	// Returns the current board. Useful for analysis.
	std::vector<std::vector<std::optional<std::tuple<square, pieceSymbol, color>>>> board();

	// Returns the current square color of the specified square. Either "light" or "dark" is returned.
	std::optional<std::string> squareColor(square sq);

	// Returns the history of the board, in the form of a string array.
	std::vector<std::string> historys();
	
	// Returns the history of the board. If verbose = true, returns an array of moves. Otherwise, an array of string is returned (array of moves).
	std::vector<std::variant<std::string, move>> history(bool verbose);
	
	// Returns the history of the board, in the form of a move array.
	std::vector<move> historym();

	// Returns a pair of booleans that indicates the rights to castle. First element is for king-side castle, second element is for queen-side.
	std::pair<bool, bool> getCastlingRights(color color);

	// Returns the current move number, in full moves.
	int moveNumber();

	std::string getComment();

	std::string deleteComment();

	std::string setComment(const std::string& comment);

	std::vector<std::pair<std::string, std::string>> getComments();

	std::vector<std::pair<std::string, std::string>> deleteComments();

	bool setCastlingRights(const color& c, std::optional<std::pair<pieceSymbol, bool>> rights);

	~Chess();
};
	
#endif