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
//
//const std::unordered_map<square, int> Ox88 = {
//    {Square::a8, 0}, {Square::b8, 1}, {Square::c8, 2}, {Square::d8, 3}, {Square::e8, 4}, {Square::f8, 5}, {Square::g8, 6}, {Square::h8, 7},
//    {Square::a7, 16}, {Square::b7, 17}, {Square::c7, 18}, {Square::d7, 19}, {Square::e7, 20}, {Square::f7, 21}, {Square::g7, 22}, {Square::h7, 23},
//    {Square::a6, 32}, {Square::b6, 33}, {Square::c6, 34}, {Square::d6, 35}, {Square::e6, 36}, {Square::f6, 37}, {Square::g6, 38}, {Square::h6, 39},
//    {Square::a5, 48}, {Square::b5, 49}, {Square::c5, 50}, {Square::d5, 51}, {Square::e5, 52}, {Square::f5, 53}, {Square::g5, 54}, {Square::h5, 55},
//    {Square::a4, 64}, {Square::b4, 65}, {Square::c4, 66}, {Square::d4, 67}, {Square::e4, 68}, {Square::f4, 69}, {Square::g4, 70}, {Square::h4, 71},
//    {Square::a3, 80}, {Square::b3, 81}, {Square::c3, 82}, {Square::d3, 83}, {Square::e3, 84}, {Square::f3, 85}, {Square::g3, 86}, {Square::h3, 87},
//    {Square::a2, 96}, {Square::b2, 97}, {Square::c2, 98}, {Square::d2, 99}, {Square::e2, 100}, {Square::f2, 101}, {Square::g2, 102}, {Square::h2, 103},
//    {Square::a1, 112}, {Square::b1, 113}, {Square::c1, 114}, {Square::d1, 115}, {Square::e1, 116}, {Square::f1, 117}, {Square::g1, 118}, {Square::h1, 119}
//};

const std::string SYMBOLS = "pnbrqkPNBRQK";

const std::array<PieceSymbol, 4> PROMOTIONS = { KNIGHT, BISHOP, ROOK, QUEEN };

const std::unordered_map<PieceSymbol, int> SIDES = {
    {KING, BITS_KSIDE_CASTLE},
    {QUEEN, BITS_QSIDE_CASTLE}
};

struct RookPosition {
    int square;
    int flag;
};

const int RANK_1 = 7;
const int RANK_2 = 6;

const int RANK_7 = 1;
const int RANK_8 = 0;

const std::vector<std::string> TERMINATION_MARKERS = { "1-0", "0-1", "1/2-1/2", "*" };

#endif
