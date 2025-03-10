/*
* Main include file for chesscpp
* 
* \file chess.h
*/
#ifndef CHESSCPP_H
#define CHESSCPP_H

#ifndef __cplusplus
#error "This library is C++-based. Please use C++ (C++14 or above) for this library."
#endif

#if defined(_MSC_VER) && (_MSC_VER < 1914)
#error "This library requires C++14 or later. Update to Visual Studio 2015 or later."
#elif !defined(_MSC_VER) && (__cplusplus < 201703L)
#error "This library requires C++14 or later. Please use a compatible compiler or use C++14 standard with the --std=c++14 option."
#endif

#include <utility>
#include <unordered_map>

#include "exptypes.h"

// Used to validate strings for FEN notation. Returns a boolean that indicates whether the FEN is valid or not, and a string for the error message.
std::pair<bool, std::string> validateFen(std::string fen);

// File of a 0x88 square.
int file(int square);

// Rank of a 0x88 square.
int rank(int square);

// Convert a string (algebraic notation) to a value of type square.
Square stringToSquare(const std::string& squareStr);

// Convert a square (the type) to a string.
std::string squareToString(Square sq);

// Convert a 0x88 square to algebraic notation.
Square algebraic(int square);

std::string moveToUci(move m);

class Chess {
private:	
	class chrImpl;
	chrImpl* chImpl;
public:
	// Clear.
	void clear(bool preserveHeaders);

	// Remove header with the specified key.
	void removeHeader(std::string key);
	
	/*
	* Loads a chess game with the specified
	* FEN, manually.
	* 
	* fen: string: The FEN to be passed
	* skipValidation: bool: Skips the FEN validation. Not recommended.
	* preserveHeaders: bool: Preserve headers.
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
	* Returns a piece on the specified square. Returns Piece() if nothing is found.
	*/
	Piece get(Square sq);

	// Puts a piece of a type on the specified square with the specified color.
	bool put(PieceSymbol type, Color c, Square sq);

	// Removes a piece from a square. Returns std::nullopt if no pieces were removed.
	Piece remove(Square sq);

	// Checks whether the king is attacked by the other side.
	bool isAttacked(Square sq, Color attackedBy);

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
	std::vector<move> moves(bool verbose, std::string sq, PieceSymbol piece = PNONE);

	std::vector<move> moves(std::string sq, PieceSymbol piece);
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
	move cmove(const moveOption& moveArg);

	move cmove(const std::string& moveArg, bool strict = false);
	
	// Undos a move.
	move undo();
	
	// Gets the current PGN of the game. Does not include results of the game.
	std::string pgn(char newline = '\n', int maxWidth = 0);
	
	std::unordered_map<std::string, std::string> header(std::vector<std::string> args ...);
	
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
	unsigned long long perft(int depth);

	// Returns the current turn, Black or White.
	Color turn();

	// Returns the current board. Useful for analysis.
	std::vector<std::vector<std::tuple<Square, PieceSymbol, Color>>> board();

	// Returns the current square color of the specified square. Either "light" or "dark" or an empty string (if square is invalid) is returned.
	std::string squareColor(Square sq);

	// Returns the history of the board, in the form of a string array.
	std::vector<std::string> historys();
	
	// Returns the history of the board. If verbose = true, returns an array of moves. Otherwise, an array of string is returned (array of moves).
	std::vector<std::tuple<std::string, move>> history(bool verbose);
	
	// Returns the history of the board, in the form of a move array.
	std::vector<move> historym();

	// Returns a pair of booleans that indicates the rights to castle. First element is for king-side castle, second element is for queen-side.
	std::pair<bool, bool> getCastlingRights(Color color);

	// Returns the current move number, in full moves.
	int moveNumber();

	bool setCastlingRights(const Color& c, std::pair<PieceSymbol, bool> rights);

	~Chess();
};
	
#endif