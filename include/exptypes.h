#pragma once
#ifndef EXPTYPES_H
#define EXPTYPES_H

#include <string>
#include <map>
#include <array>
#include <vector>

enum class Square : int {
    a8 = 0, b8, c8, d8, e8, f8, g8, h8,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a1, b1, c1, d1, e1, f1, g1, h1,
    NO_SQUARE = -1
};
enum class pieceSymbol : int {
    p, n, b, r, q, k, NO_PIECE = -10
};
enum class Color {
    w, b, NO_COLOR = -10
};

// Default FEN
const std::string DEFAULT_POSITION = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

// Squares as strings, what is this even used for
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

#define PAWN pieceSymbol::p
#define ROOK pieceSymbol::r
#define KNIGHT pieceSymbol::n
#define BISHOP pieceSymbol::b
#define QUEEN pieceSymbol::q
#define KING pieceSymbol::k
#define PNONE pieceSymbol::NO_PIECE

// Notation for white piece
#define WHITE Color::w
// Notation for black piece
#define BLACK Color::b

struct piece {
    Color color = Color::NO_COLOR;
    pieceSymbol type = PNONE;

    explicit operator bool() const;
};

typedef struct internalMove internalMove;

typedef struct move {
    Color color = Color::NO_COLOR;
    Square from = Square::NO_SQUARE;
    Square to = Square::NO_SQUARE;
    pieceSymbol piece = PNONE;
    pieceSymbol captured = PNONE;
    pieceSymbol promotion = PNONE;
    std::string flags = "";
    std::string san = "";
    std::string lan = "";
    std::string before = "";
    std::string after = "";
} move;

struct moveOption {
    std::string from = "";
    std::string to = "";
    std::string promotion = "";
};

#endif