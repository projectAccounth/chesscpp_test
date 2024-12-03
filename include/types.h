#pragma once

#ifndef TYPES_H
#define TYPES_H

#include <optional>
#include <string>
#include <map>
#include <array>
#include <vector>
#include <variant>

// Default FEN
const std::string DEFAULT_POSITION = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

// Empty square
#define EMPTY -1

enum class square : int {
    a8 = 0, b8, c8, d8, e8, f8, g8, h8,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a1, b1, c1, d1, e1, f1, g1, h1
};

enum class color {
    w, b
};

enum class pieceSymbol : int {
    p, n, b, r, q ,k
};

struct moveOption {
    square from;
    square to;
};

#define PAWN pieceSymbol::p
#define ROOK pieceSymbol::r
#define KNIGHT pieceSymbol::n
#define BISHOP pieceSymbol::b
#define QUEEN pieceSymbol::q
#define KING pieceSymbol::k

// Notation for white piece
#define WHITE color::w
// Notation for black piece
#define BLACK color::b

struct piece {
    color color;
    pieceSymbol type;
};

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

typedef struct move  {
    color color;
    square from;
    square to;
    pieceSymbol piece;
    std::optional<pieceSymbol> captured;
    std::optional<pieceSymbol> promotion;
    std::string flags;
    std::string san;
    std::string lan;
    std::string before;
    std::string after;
} move;

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
    {square::a8, 0}, {square::b8, 1}, {square::c8, 2}, {square::d8, 3}, {square::e8, 4}, {square::f8, 5}, {square::g8, 6}, {square::h8, 7},
    {square::a7, 16}, {square::b7, 17}, {square::c7, 18}, {square::d7, 19}, {square::e7, 20}, {square::f7, 21}, {square::g7, 22}, {square::h7, 23},
    {square::a6, 32}, {square::b6, 33}, {square::c6, 34}, {square::d6, 35}, {square::e6, 36}, {square::f6, 37}, {square::g6, 38}, {square::h6, 39},
    {square::a5, 48}, {square::b5, 49}, {square::c5, 50}, {square::d5, 51}, {square::e5, 52}, {square::f5, 53}, {square::g5, 54}, {square::h5, 55},
    {square::a4, 64}, {square::b4, 65}, {square::c4, 66}, {square::d4, 67}, {square::e4, 68}, {square::f4, 69}, {square::g4, 70}, {square::h4, 71},
    {square::a3, 80}, {square::b3, 81}, {square::c3, 82}, {square::d3, 83}, {square::e3, 84}, {square::f3, 85}, {square::g3, 86}, {square::h3, 87},
    {square::a2, 96}, {square::b2, 97}, {square::c2, 98}, {square::d2, 99}, {square::e2, 100}, {square::f2, 101}, {square::g2, 102}, {square::h2, 103},
    {square::a1, 112}, {square::b1, 113}, {square::c1, 114}, {square::d1, 115}, {square::e1, 116}, {square::f1, 117}, {square::g1, 118}, {square::h1, 119}
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
