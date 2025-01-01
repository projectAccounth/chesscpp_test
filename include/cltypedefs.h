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

// Default FEN
const std::string DEFAULT_POSITION = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

#include "exptypes.h"

// Empty square
#define EMPTY -1

const std::map<pieceSymbol, char> ptoc = {
    {PAWN, 'p'}, {ROOK, 'r'}, {BISHOP, 'b'}, {KNIGHT, 'n'}, {QUEEN, 'q'}, {KING, 'k'}
};

const std::map<char, color> charToColor = {
    {'w', WHITE},
    {'b', BLACK}
};

const std::map<char, pieceSymbol> strPchrs = {
    {'p', pieceSymbol::p}, {'r', pieceSymbol::r}, {'b', pieceSymbol::b}, {'n', pieceSymbol::n}, {'q', pieceSymbol::q}, {'k', pieceSymbol::k}
};

const std::map<color, char> ctoc = { {WHITE, 'w'}, {BLACK, 'b'} };

typedef struct internalMove {
    color color;
    int from;
    int to;
    pieceSymbol piece;
    std::optional<pieceSymbol> captured;
    std::optional<pieceSymbol> promotion;
    int flags;
} internalMove;

class History {
public:
    internalMove move;
    std::map<color, int> kings;
    color turn;
    std::map<color, int> castling;
    int epSquare;
    int halfMoves;
    int moveNumber;
};

const std::map<std::string, char> FLAGS = {
    {"NORMAL", 'n'},
    {"CAPTURE", 'c'},
    {"BIG_PAWN", 'b'},
    {"EP_CAPTURE", 'e'},
    {"PROMOTION", 'p'},
    {"KSIDE_CASTLE", 'k'},
    {"QSIDE_CASTLE", 'q'}
};

const std::unordered_map<std::string, int> BITS = {
    {"NORMAL", 1},
    {"CAPTURE", 2},
    {"BIG_PAWN", 4},
    {"EP_CAPTURE", 8},
    {"PROMOTION", 16},
    {"KSIDE_CASTLE", 32},
    {"QSIDE_CASTLE", 64}
};

const std::map<color, std::vector<int>> PAWN_OFFSETS = {
    {color::b, {16, 32, 17, 15}}, // Black pawn offsets
    {color::w, {-16, -32, -17, -15}} // White pawn offsets
};

const std::map<pieceSymbol, std::vector<int>> PIECE_OFFSETS = {
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

const std::map<pieceSymbol, int> PIECE_MASKS = {{pieceSymbol::p, 0x1}, {pieceSymbol::n, 0x2}, {pieceSymbol::b, 0x4}, {pieceSymbol::r, 0x8}, {pieceSymbol::q, 0x10}, {pieceSymbol::k, 0x20}};

const std::string SYMBOLS = "pnbrqkPNBRQK";

const std::array<pieceSymbol, 4> PROMOTIONS = { KNIGHT, BISHOP, ROOK, QUEEN };

const std::map<pieceSymbol, int> SIDES = {
    {KING, BITS.at("KSIDE_CASTLE")},
    {QUEEN, BITS.at("QSIDE_CASTLE")}
};

struct RookPosition {
    int square;
    int flag;
};

const std::map<color, std::vector<RookPosition>> ROOKS = {
    {color::w, {
        {0, BITS.at("QSIDE_CASTLE")},
        {7, BITS.at("KSIDE_CASTLE")}
    }},
    {color::b, {
        {56, BITS.at("QSIDE_CASTLE")},
        {63, BITS.at("KSIDE_CASTLE")}
    }}
};

const int RANK_1 = 7;
const int RANK_2 = 6;

const int RANK_7 = 1;
const int RANK_8 = 0;

const std::vector<std::string> TERMINATION_MARKERS = { "1-0", "0-1", "1/2-1/2", "*" };

const std::map<color, int> SECOND_RANK = {
    {color::w, RANK_2}, {color::b, RANK_7}
};

#endif
