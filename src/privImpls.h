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

std::string trim(const std::string& str);

bool operator<(square lhs, square rhs);

std::vector<std::string> split(const std::string& str, char delimiter);

std::vector<std::string> splitWithRegex(const std::string& str, const std::string& pattern);

std::string join(const std::vector<std::string>& elements, const std::string& delimiter);

bool isDigit(std::string c);

color swapColor(color color);

std::string getDisambiguator(internalMove move, std::vector<internalMove> moves);

void addMove(std::vector<internalMove>& moves, color color, int from, int to, pieceSymbol p, pieceSymbol captured = PNONE, int flags = BITS_NORMAL);

std::string replaceSubstring(const std::string& str, const std::string& from, const std::string& to);

pieceSymbol inferPieceType(std::string san);

std::string strippedSan(std::string move);

std::string trimFen(std::string fen);

namespace privs {

	static inline std::vector<RookPosition> getRookInf(const color& c) {
		switch (c) {
		case BLACK: return { { 0, BITS_QSIDE_CASTLE }, { 7, BITS_KSIDE_CASTLE } };
		case WHITE: return { { 112, BITS_QSIDE_CASTLE }, { 119, BITS_KSIDE_CASTLE } };
		case color::NO_COLOR: break;
		}
		throw std::invalid_argument("Invalid piece (at getRookInf)");
	}

	static inline int getSecondRank(const color& c) {
		switch (c) {
		case BLACK: return RANK_7;
		case WHITE: return RANK_2;
		case color::NO_COLOR: break;
		}
		throw std::invalid_argument("Invalid piece (at getSecondRank)");
	}

	static inline int getCastlingSide(const pieceSymbol& p) {
		switch (p) {
		case KING: return BITS_KSIDE_CASTLE;
		case QUEEN: return BITS_QSIDE_CASTLE;
		default: break;
		}
		throw std::invalid_argument("Invalid piece (at getCastlingSide)");
	}

	static inline pieceSymbol charToSymbol(const char& c) {
		switch (c) {
		case 'p': return PAWN;
		case 'n': return KNIGHT;
		case 'b': return BISHOP;
		case 'r': return ROOK;
		case 'q': return QUEEN;
		case 'k': return KING;
		}
		throw std::invalid_argument("Invalid piece (at charToSymbol)");
	}

	static inline char pieceToChar(const pieceSymbol& p) {
		switch (p) {
		case PAWN: return 'p';
		case KNIGHT: return 'n';
		case BISHOP: return 'p';
		case ROOK: return 'r';
		case QUEEN: return 'q';
		case KING: return 'k';
		case PNONE: break;
		}
		throw std::invalid_argument("Invalid piece (at pieceToChar)");
	}

	static inline std::vector<int> getPieceOffsets(const pieceSymbol& p) {
		switch (p) {
		case KNIGHT: return { -18, -33, -31, -14, 18, 33, 31, 14 };
		case BISHOP: return { -17, -15, 17, 15 };
		case ROOK: return { -16, 1, 16, -1 };

		case QUEEN:
		case KING: return { -17, -16, -15, 1, 17, 16, 15, -1 };

		case PAWN: break;
		case PNONE: break;
		}
		throw std::invalid_argument("Invalid piece (or not supported) (at getPieceOffsets)");
	}

	static inline int getPieceMasks(const pieceSymbol& p) {
		switch (p) {
		case PAWN: return 0x1;
		case KNIGHT: return 0x2;
		case BISHOP: return 0x4;
		case ROOK: return 0x8;
		case QUEEN: return 0x10;
		case KING: return 0x20;
		case PNONE: break;
		}
		throw std::invalid_argument("Invalid piece (at getPieceMasks)");
	}

	static inline std::vector<int> getPawnOffsets(const color& c) {
		switch (c) {
		case BLACK: return { 16, 32, 17, 15 };
		case WHITE: return { -16, -32, -17, -15 };
		case color::NO_COLOR: break;
		}
		throw std::invalid_argument("Invalid piece (at getPawnOffsets)");
	}

	static inline bool isValid8x8(const square& sq) {
		return static_cast<int>(sq) >= 0 && static_cast<int>(sq) < 64;
	}

	static inline bool isValid0x88(const int& sq) {
		return (sq & 0x88) == 0;
	}

	static inline int squareTo0x88(const square& sq) {
		return (static_cast<int>(sq) >> 3 << 4) | (static_cast<int>(sq) & 7);
	}

}
