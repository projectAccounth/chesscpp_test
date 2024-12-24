#pragma once
#include "../include/cltypedefs.h"
#include "../include/chesscpp.h"
#include <cmath>
#include <tuple>
#include <sstream>
#include <cctype>
#include <regex>
#include <functional>
#include <algorithm>
#include <iomanip>
#include <bitset>
#include <stdexcept>
#include <iterator>
#include <iostream>

class Chess::chrImpl {
private:
	Chess& ch;
public:
	chrImpl(Chess& c) : ch(c) {}

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

	std::map<std::string, std::optional<int>> _positionCount;

	void _updateSetup(std::string fen);

	bool _put(pieceSymbol type, color color, square sq);

	void _updateCastlingRights();

	void _updateEnPassantSquare();

	bool _attacked(color c, int sq);

	bool _isKingAttacked(color c);

	std::vector<internalMove> _moves(std::optional<bool> legal = true, std::optional<pieceSymbol> piece = std::nullopt, std::optional<std::string> sq = std::nullopt);

	void _push(internalMove move);

	void _makeMove(internalMove move);

	std::optional<internalMove> _undoMove();

	std::string _moveToSan(internalMove move, std::vector<internalMove> moves);

	std::optional<internalMove> _moveFromSan(std::string move, bool strict = false);

	move _makePretty(internalMove uglyMove);

	int _getPositionCount(std::string fen);

	void _incPositionCount(std::string fen);

	void _decPositionCount(std::string fen);

	void _pruneComments();
};

std::string trim(const std::string& str);

bool operator<(square lhs, square rhs);

std::vector<std::string> split(const std::string& str, char delimiter);

std::vector<std::string> splitWithRegex(const std::string& str, const std::string& pattern);

std::string join(const std::vector<std::string>& elements, const std::string& delimiter);

bool isDigit(std::string c);

int file(int square);

int rank(int square);

square stringToSquare(const std::string& squareStr);

std::string squareToString(square sq);

square algebraic(int square);

color swapColor(color color);

std::string getDisambiguator(internalMove move, std::vector<internalMove> moves);

void addMove(std::vector<internalMove>& moves, color color, int from, int to, pieceSymbol p, std::optional<pieceSymbol> captured = std::nullopt, int flags = BITS.at("NORMAL"));

std::string replaceSubstring(const std::string& str, const std::string& from, const std::string& to);

std::optional<pieceSymbol> inferPieceType(std::string san);

std::string strippedSan(std::string move);

std::string trimFen(std::string fen);
