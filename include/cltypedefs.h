#pragma once
#ifndef CLTYPEDEFS_H
#define CLTYPEDEFS_H

#ifndef __cplusplus
#error "This library is C++-based. Please use C++ (C++14 or above) for this library."
#endif

#if defined(_MSC_VER) && (_MSC_VER < 1914)
#error "This library requires C++14 or later. Update to Visual Studio 2015 or later."
#elif !defined(_MSC_VER) && (__cplusplus < 201703L)
#error "This library requires C++14 or later. Please use a compatible compiler or use C++14 standard with the --std=c++14 option."
#endif

#include <string>
#include <map>
#include <unordered_map>
#include <array>
#include <vector>

#include "exptypes.h"

// Empty square
#define EMPTY square::NO_SQUARE

typedef struct internalMove {
    color color = color::NO_COLOR;
    int from = -1;
    int to = -1;
    pieceSymbol piece = PNONE;
    pieceSymbol captured = PNONE;
    pieceSymbol promotion = PNONE;
    int flags = 0;

    explicit operator bool() const;
} internalMove;

class History {
public:
    internalMove move = internalMove();
    std::map<color, int> kings = std::map<color, int>();
    color turn = color::NO_COLOR;
    std::map<color, int> castling = std::map<color, int>();
    int epSquare = -1;
    int halfMoves = 0;
    int moveNumber = 0;
    explicit operator bool() const;
};

const char FLAGS_NORMAL = 'n';
const char FLAGS_CAPTURE = 'c';
const char FLAGS_BIG_PAWN = 'b';
const char FLAGS_EP_CAPTURE = 'e';
const char FLAGS_PROMOTION = 'e';
const char FLAGS_KSIDE_CASTLE = 'k';
const char FLAGS_QSIDE_CASTLE = 'q';

const int BITS_NORMAL = 1;
const int BITS_CAPTURE = 2;
const int BITS_BIG_PAWN = 4;
const int BITS_EP_CAPTURE = 8;
const int BITS_PROMOTION = 16;
const int BITS_KSIDE_CASTLE = 32;
const int BITS_QSIDE_CASTLE = 64;

const std::unordered_map<color, std::vector<int>> PAWN_OFFSETS = {
    {color::b, {16, 32, 17, 15}}, // Black pawn offsets
    {color::w, {-16, -32, -17, -15}} // White pawn offsets
};

const std::unordered_map<pieceSymbol, std::vector<int>> PIECE_OFFSETS = {
    {KNIGHT, {-18, -33, -31, -14, 18, 33, 31, 14}}, // Knight offsets
    {BISHOP, {-17, -15, 17, 15}},                  // Bishop offsets
    {ROOK, {-16, 1, 16, -1}},                    // Rook offsets
    {QUEEN, {-17, -16, -15, 1, 17, 16, 15, -1}},   // Queen offsets
    {KING, {-17, -16, -15, 1, 17, 16, 15, -1}}    // King offsets
};
const std::vector<int> ATTACKS = {
  20, 0, 0, 0, 0, 0, 0, 24,  0, 0, 0, 0, 0, 0,20, 0,
   0,20, 0, 0, 0, 0, 0, 24,  0, 0, 0, 0, 0,20, 0, 0,
   0, 0,20, 0, 0, 0, 0, 24,  0, 0, 0, 0,20, 0, 0, 0,
   0, 0, 0,20, 0, 0, 0, 24,  0, 0, 0,20, 0, 0, 0, 0,
   0, 0, 0, 0,20, 0, 0, 24,  0, 0,20, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0,20, 2, 24,  2,20, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 2,53, 56, 53, 2, 0, 0, 0, 0, 0, 0,
  24,24,24,24,24,24,56,  0, 56,24,24,24,24,24,24, 0,
   0, 0, 0, 0, 0, 2,53, 56, 53, 2, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0,20, 2, 24,  2,20, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0,20, 0, 0, 24,  0, 0,20, 0, 0, 0, 0, 0,
   0, 0, 0,20, 0, 0, 0, 24,  0, 0, 0,20, 0, 0, 0, 0,
   0, 0,20, 0, 0, 0, 0, 24,  0, 0, 0, 0,20, 0, 0, 0,
   0,20, 0, 0, 0, 0, 0, 24,  0, 0, 0, 0, 0,20, 0, 0,
  20, 0, 0, 0, 0, 0, 0, 24,  0, 0, 0, 0, 0, 0,20
};

const std::vector<int> RAYS = {
   17,  0,  0,  0,  0,  0,  0, 16,  0,  0,  0,  0,  0,  0, 15, 0,
    0, 17,  0,  0,  0,  0,  0, 16,  0,  0,  0,  0,  0, 15,  0, 0,
    0,  0, 17,  0,  0,  0,  0, 16,  0,  0,  0,  0, 15,  0,  0, 0,
    0,  0,  0, 17,  0,  0,  0, 16,  0,  0,  0, 15,  0,  0,  0, 0,
    0,  0,  0,  0, 17,  0,  0, 16,  0,  0, 15,  0,  0,  0,  0, 0,
    0,  0,  0,  0,  0, 17,  0, 16,  0, 15,  0,  0,  0,  0,  0, 0,
    0,  0,  0,  0,  0,  0, 17, 16, 15,  0,  0,  0,  0,  0,  0, 0,
    1,  1,  1,  1,  1,  1,  1,  0, -1, -1,  -1,-1, -1, -1, -1, 0,
    0,  0,  0,  0,  0,  0,-15,-16,-17,  0,  0,  0,  0,  0,  0, 0,
    0,  0,  0,  0,  0,-15,  0,-16,  0,-17,  0,  0,  0,  0,  0, 0,
    0,  0,  0,  0,-15,  0,  0,-16,  0,  0,-17,  0,  0,  0,  0, 0,
    0,  0,  0,-15,  0,  0,  0,-16,  0,  0,  0,-17,  0,  0,  0, 0,
    0,  0,-15,  0,  0,  0,  0,-16,  0,  0,  0,  0,-17,  0,  0, 0,
    0,-15,  0,  0,  0,  0,  0,-16,  0,  0,  0,  0,  0,-17,  0, 0,
  -15,  0,  0,  0,  0,  0,  0,-16,  0,  0,  0,  0,  0,  0,-17
};

const std::unordered_map<pieceSymbol, int> PIECE_MASKS = {
    {pieceSymbol::p, 0x1},
    {pieceSymbol::n, 0x2},
    {pieceSymbol::b, 0x4},
    {pieceSymbol::r, 0x8},
    {pieceSymbol::q, 0x10},
    {pieceSymbol::k, 0x20}
};

const std::string SYMBOLS = "pnbrqkPNBRQK";

const std::array<pieceSymbol, 4> PROMOTIONS = { KNIGHT, BISHOP, ROOK, QUEEN };

const std::unordered_map<pieceSymbol, int> SIDES = {
    {KING, BITS_KSIDE_CASTLE},
    {QUEEN, BITS_QSIDE_CASTLE}
};

struct RookPosition {
    int square;
    int flag;
};

const std::unordered_map<color, std::vector<RookPosition>> ROOKS = {
    {color::w, {
        {112, BITS_QSIDE_CASTLE},
        {119, BITS_KSIDE_CASTLE}
    }},
    {color::b, {
        {0, BITS_QSIDE_CASTLE},
        {7, BITS_KSIDE_CASTLE}
    }}
};

const int RANK_1 = 7;
const int RANK_2 = 6;

const int RANK_7 = 1;
const int RANK_8 = 0;

const std::vector<std::string> TERMINATION_MARKERS = { "1-0", "0-1", "1/2-1/2", "*" };

const std::unordered_map<color, int> SECOND_RANK = {
    {color::w, RANK_2}, {color::b, RANK_7}
};

#endif
