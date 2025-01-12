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
#define EMPTY Square::NO_SQUARE

typedef struct InternalMove {
    Color color = Color::NO_COLOR;
    int from = -1;
    int to = -1;
    PieceSymbol piece = PNONE;
    PieceSymbol captured = PNONE;
    PieceSymbol promotion = PNONE;
    int flags = 0;

    explicit operator bool() const;
} InternalMove;

class History {
public:
    InternalMove move = InternalMove();
    std::map<Color, int> kings = std::map<Color, int>();
    Color turn = Color::NO_COLOR;
    std::map<Color, int> castling = std::map<Color, int>();
    int epSquare = -1;
    int halfMoves = 0;
    int moveNumber = 0;
    explicit operator bool() const;
};

const char FLAGS_NORMAL = 'n';
const char FLAGS_CAPTURE = 'c';
const char FLAGS_BIG_PAWN = 'b';
const char FLAGS_EP_CAPTURE = 'e';
const char FLAGS_PROMOTION = 'p';
const char FLAGS_KSIDE_CASTLE = 'k';
const char FLAGS_QSIDE_CASTLE = 'q';

const int BITS_NORMAL = 1;
const int BITS_CAPTURE = 2;
const int BITS_BIG_PAWN = 4;
const int BITS_EP_CAPTURE = 8;
const int BITS_PROMOTION = 16;
const int BITS_KSIDE_CASTLE = 32;
const int BITS_QSIDE_CASTLE = 64;

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

const std::array<int, 64> Ox88 = {
    0, 1, 2, 3, 4, 5, 6, 7,
    16, 17, 18, 19, 20, 21, 22, 23,
    32, 33, 34, 35, 36, 37, 38, 39,
    48, 49, 50, 51, 52, 53, 54, 55,
    64, 65, 66, 67, 68, 69, 70, 71,
    80, 81, 82, 83, 84, 85, 86, 87,
    96, 97, 98, 99, 100, 101, 102, 103,
    112, 113, 114, 115, 116, 117, 118, 119
};

const std::string SYMBOLS = "pnbrqkPNBRQK";

const std::array<PieceSymbol, 4> PROMOTIONS = { KNIGHT, BISHOP, ROOK, QUEEN };

struct RookPosition {
    int square;
    int flag;
};

const std::map<Color, std::vector<RookPosition>> ROOKS = {
    { BLACK,
    {
        { 0, BITS_QSIDE_CASTLE },
        { 7, BITS_KSIDE_CASTLE }
    }
    },
    { WHITE,
    {
        { 112, BITS_QSIDE_CASTLE },
        { 119, BITS_KSIDE_CASTLE }
    }
    }
};

const int RANK_1 = 7;
const int RANK_2 = 6;

const int RANK_7 = 1;
const int RANK_8 = 0;

const std::vector<std::string> TERMINATION_MARKERS = { "1-0", "0-1", "1/2-1/2", "*" };

#endif
