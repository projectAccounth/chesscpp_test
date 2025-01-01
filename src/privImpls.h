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

void addMove(std::vector<internalMove>& moves, color color, int from, int to, pieceSymbol p, std::optional<pieceSymbol> captured = std::nullopt, int flags = BITS.at("NORMAL"));

std::string replaceSubstring(const std::string& str, const std::string& from, const std::string& to);

std::optional<pieceSymbol> inferPieceType(std::string san);

std::string strippedSan(std::string move);

std::string trimFen(std::string fen);

int squareTo0x88(const square& sq);

bool isValid8x8(const square& sq);

bool isValid0x88(const int& sq);

char pieceToChar(const pieceSymbol& p);

pieceSymbol charToSymbol(const char& c);