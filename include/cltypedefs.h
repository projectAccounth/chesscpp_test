#pragma once
#ifndef CLTYPEDEFS_H
#define CLTYPEDEFS_H

#ifndef __cplusplus
#error "This library is C++-based. Please use C++ (C++17 or above) for this library."
#endif

#if defined(_MSC_VER) && (_MSC_VER < 1914)
#error "This library requires C++17 or later. Update to Visual Studio 2017 version 15.7 or later."
#elif !defined(_MSC_VER) && (__cplusplus < 201703L)
#error "This library requires C++17 or later. Please use a compatible compiler or use C++17 standard with the --std=c++17 option."
#endif

#include <optional>
#include <string>
#include <map>
#include <unordered_map>
#include <array>
#include <vector>
#include <variant>

#include "exptypes.h"

// Empty square
#define EMPTY -1

typedef struct InternalMove {
    Color color;
    int from;
    int to;
    PieceSymbol piece;
    std::optional<PieceSymbol> captured;
    std::optional<PieceSymbol> promotion;
    int flags;
} InternalMove;

class History {
public:
    InternalMove move;
    std::map<Color, int> kings;
    Color turn;
    std::map<Color, int> castling;
    int epSquare;
    int halfMoves;
    int moveNumber;
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

const std::map<Color, std::vector<int>> PAWN_OFFSETS = {
    {Color::b, {16, 32, 17, 15}}, // Black pawn offsets
    {Color::w, {-16, -32, -17, -15}} // White pawn offsets
};

const std::map<PieceSymbol, std::vector<int>> PIECE_OFFSETS = {
    {KNIGHT, {-18, -33, -31, -14, 18, 33, 31, 14}}, // Knight offsets
    {BISHOP, {-17, -15, 17, 15}},                  // Bishop offsets
    {ROOK, {-16, 1, 16, -1}},                    // Rook offsets
    {QUEEN, {-17, -16, -15, 1, 17, 16, 15, -1}},   // Queen offsets
    {KING, {-17, -16, -15, 1, 17, 16, 15, -1}}    // King offsets
};
const std::vector<int> ATTACKS = {
    20, 0, 0, 0, 0, 0, 0, 24, 0, 0, 0, 0, 0, 0, 20, 0,
        0, 20, 0, 0, 0, 0, 0, 24, 0, 0, 0, 0, 0, 20, 0, 0,
        0, 0, 20, 0, 0, 0, 0, 24, 0, 0, 0, 0, 20, 0, 0, 0,
        0, 0, 0, 20, 0, 0, 0, 24, 0, 0, 0, 20, 0, 0, 0, 0,
        0, 0, 0, 0, 20, 0, 0, 24, 0, 0, 20, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 20, 2, 24, 2, 20, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 2, 53, 56, 53, 2, 0, 0, 0, 0, 0, 0,
        24, 24, 24, 24, 24, 24, 56, 0, 56, 24, 24, 24, 24, 24, 24, 0,
        0, 0, 0, 0, 0, 2, 53, 56, 53, 2, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 20, 2, 24, 2, 20, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 20, 0, 0, 24, 0, 0, 20, 0, 0, 0, 0, 0,
        0, 0, 0, 20, 0, 0, 0, 24, 0, 0, 0, 20, 0, 0, 0, 0,
        0, 0, 20, 0, 0, 0, 0, 24, 0, 0, 0, 0, 20, 0, 0, 0,
        0, 20, 0, 0, 0, 0, 0, 24, 0, 0, 0, 0, 0, 20, 0, 0,
        20, 0, 0, 0, 0, 0, 0, 24, 0, 0, 0, 0, 0, 0, 20
};

const std::vector<int> RAYS = {
    17, 0, 0, 0, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 15, 0,
        0, 17, 0, 0, 0, 0, 0, 16, 0, 0, 0, 0, 0, 15, 0, 0,
        0, 0, 17, 0, 0, 0, 0, 16, 0, 0, 0, 0, 15, 0, 0, 0,
        0, 0, 0, 17, 0, 0, 0, 16, 0, 0, 0, 15, 0, 0, 0, 0,
        0, 0, 0, 0, 17, 0, 0, 16, 0, 0, 15, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 17, 0, 16, 0, 15, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 17, 16, 15, 0, 0, 0, 0, 0, 0, 0,
        1, 1, 1, 1, 1, 1, 1, 0, -1, -1, -1, -1, -1, -1, -1, 0,
        0, 0, 0, 0, 0, 0, -15, -16, -17, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, -15, 0, -16, 0, -17, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, -15, 0, 0, -16, 0, 0, -17, 0, 0, 0, 0, 0,
        0, 0, 0, -15, 0, 0, 0, -16, 0, 0, 0, -17, 0, 0, 0, 0,
        0, 0, -15, 0, 0, 0, 0, -16, 0, 0, 0, 0, -17, 0, 0, 0,
        0, -15, 0, 0, 0, 0, 0, -16, 0, 0, 0, 0, 0, -17, 0, 0,
        -15, 0, 0, 0, 0, 0, 0, -16, 0, 0, 0, 0, 0, 0, -17
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
const std::unordered_map<PieceSymbol, int> PIECE_MASKS = {{PAWN, 0x1}, {KNIGHT, 0x2}, {BISHOP, 0x4}, {ROOK, 0x8}, {QUEEN, 0x10}, {KING, 0x20}};

const std::string SYMBOLS = "pnbrqkPNBRQK";

const std::array<PieceSymbol, 4> PROMOTIONS = { KNIGHT, BISHOP, ROOK, QUEEN };

struct RookPosition {
    int square;
    int flag;
};

const std::map<Color, std::vector<RookPosition>> ROOKS = {
    {Color::w, {
        {0, BITS_QSIDE_CASTLE},
        {7, BITS_KSIDE_CASTLE}
    }},
    {Color::b, {
        {56, BITS_QSIDE_CASTLE},
        {63, BITS_KSIDE_CASTLE}
    }}
};

const int RANK_1 = 7;
const int RANK_2 = 6;

const int RANK_7 = 1;
const int RANK_8 = 0;

const std::vector<std::string> TERMINATION_MARKERS = { "1-0", "0-1", "1/2-1/2", "*" };

const std::map<Color, int> SECOND_RANK = {
    {Color::w, RANK_2}, {Color::b, RANK_7}
};

#endif
