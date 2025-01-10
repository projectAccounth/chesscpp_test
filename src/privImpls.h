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

bool operator<(Square lhs, Square rhs);

std::vector<std::string> split(const std::string& str, char delimiter);

std::vector<std::string> splitWithRegex(const std::string& str, const std::string& pattern);

std::string join(const std::vector<std::string>& elements, const std::string& delimiter);

bool isDigit(std::string c);

Color swapColor(Color color);

std::string getDisambiguator(InternalMove move, std::vector<InternalMove> moves);

void addMove(std::vector<InternalMove>& moves, Color color, int from, int to, PieceSymbol p, std::optional<PieceSymbol> captured = std::nullopt, int flags = BITS_NORMAL);

std::string replaceSubstring(const std::string& str, const std::string& from, const std::string& to);

std::optional<PieceSymbol> inferPieceType(std::string san);

std::string strippedSan(std::string move);

std::string trimFen(std::string fen);

int squareTo0x88(const Square& sq);

bool isValid8x8(const Square& sq);

bool isValid0x88(const int& sq);

char pieceToChar(const PieceSymbol& p);

PieceSymbol charToSymbol(const char& c);