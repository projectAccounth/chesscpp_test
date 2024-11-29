#pragma once

#ifndef TYPES_H
#define TYPES_H

#include <optional>
#include <string>
#include <map>
#include <array>
#include <vector>

// Default FEN
const std::string DEFAULT_POSITION = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

// Notation for white piece
#define WHITE "w"
// Notation for black piece
#define BLACK "b"

#define PAWN 'p'
#define ROOK 'r'
#define KNIGHT 'n'
#define BISHOP 'b'
#define QUEEN 'q'
#define KING 'k'

// Empty square
#define EMPTY -1

enum square {
    a1, a2, a3, a4, a5, a6, a7, a8,
    b1, b2, b3, b4, b5, b6, b7, b8,
    c1, c2, c3, c4, c5, c6, c7, c8,
    d1, d2, d3, d4, d5, d6, d7, d8,
    e1, e2, e3, e4, e5, e6, e7, e8,
    f1, f2, f3, f4, f5, f6, f7, f8,
    g1, g2, g3, g4, g5, g6, g7, g8,
    h1, h2, h3, h4, h5, h6, h7, h8
};

enum class color {
    w, b
};

enum class pieceSymbol {
    p, r, n, q, b, k
};

struct piece {
    color color;
    pieceSymbol type;
};

struct internalMove {
    color color;
    int from;
    int to;
    pieceSymbol piece;
    std::optional<pieceSymbol> captured;
    std::optional<pieceSymbol> promotion;
    int flags;
};

struct move  {
    color color;
    square from;
    square to;
    pieceSymbol piece;
    std::optional<pieceSymbol> captured;
    std::optional<pieceSymbol> promotion;
    int flags;
    std::string san;
    std::string lan;
    std::string before;
    std::string after;
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

const std::array<std::string, 64> SQUARES = {
        "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
        "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
        "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
        "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
        "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
        "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
        "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
        "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1"
};

const std::map<std::string, int> BITS = {
    {"NORMAL", 1},
    {"CAPTURE", 2},
    {"BIG_PAWN", 4},
    {"EP_CAPTURE", 8},
    {"PROMOTION", 16},
    {"KSIDE_CASTLE", 32},
    {"QSIDE_CASTLE", 64}
};

std::map<square, int> Ox88 = {
    {a8, 0}, {b8, 1}, {c8, 2}, {d8, 3}, {e8, 4}, {f8, 5}, {g8, 6}, {h8, 7},
    {a7, 16}, {b7, 17}, {c7, 18}, {d7, 19}, {e7, 20}, {f7, 21}, {g7, 22}, {h7, 23},
    {a6, 32}, {b6, 33}, {c6, 34}, {d6, 35}, {e6, 36}, {f6, 37}, {g6, 38}, {h6, 39},
    {a5, 48}, {b5, 49}, {c5, 50}, {d5, 51}, {e5, 52}, {f5, 53}, {g5, 54}, {h5, 55},
    {a4, 64}, {b4, 65}, {c4, 66}, {d4, 67}, {e4, 68}, {f4, 69}, {g4, 70}, {h4, 71},
    {a3, 80}, {b3, 81}, {c3, 82}, {d3, 83}, {e3, 84}, {f3, 85}, {g3, 86}, {h3, 87},
    {a2, 96}, {b2, 97}, {c2, 98}, {d2, 99}, {e2, 100}, {f2, 101}, {g2, 102}, {h2, 103},
    {a1, 112}, {b1, 113}, {c1, 114}, {d1, 115}, {e1, 116}, {f1, 117}, {g1, 118}, {h1, 119}
};

const std::map<char, std::vector<int>> PAWN_OFFSETS = {
    {'b', {16, 32, 17, 15}}, // Black pawn offsets
    {'w', {-16, -32, -17, -15}} // White pawn offsets
};

const std::map<char, std::vector<int>> PIECE_OFFSETS = {
    {'n', {-18, -33, -31, -14, 18, 33, 31, 14}}, // Knight offsets
    {'b', {-17, -15, 17, 15}},                  // Bishop offsets
    {'r', {-16, 1, 16, -1}},                    // Rook offsets
    {'q', {-17, -16, -15, 1, 17, 16, 15, -1}},   // Queen offsets
    {'k', {-17, -16, -15, 1, 17, 16, 15, -1}}    // King offsets
};

#endif
