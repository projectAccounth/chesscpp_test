#pragma once
#include "../include/libtypes"
#include "../include/chesscpp"
#include <cmath>
#include <cstdint>
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

using namespace ChessCpp;

bool operator<(Square lhs, Square rhs);

class Helper {
public:
    static inline std::string trim(const std::string& str) {
        auto start = std::find_if_not(str.begin(), str.end(), [](unsigned char ch) {
            return std::isspace(ch);
            });
        auto end = std::find_if_not(str.rbegin(), str.rend(), [](unsigned char ch) {
            return std::isspace(ch);
            }).base();

        return (start < end ? std::string(start, end) : std::string());
    }

    static inline std::vector<std::string> split(const std::string& str, char delimiter) {
        std::vector<std::string> tokens;
        std::stringstream ss(str);
        std::string token;
    
        while (std::getline(ss, token, delimiter)) {
            tokens.push_back(token);
        }
    
        return tokens;
    }

    static inline std::vector<std::string> splitWithRegex(const std::string& str, const std::string& pattern) {
        std::regex re(pattern);
        std::sregex_token_iterator it(str.begin(), str.end(), re, -1); // -1 means "split by the regex"
        std::sregex_token_iterator end;
    
        return { it, end };
    }
    
    static inline std::string join(const std::vector<std::string>& elements, const std::string& delimiter) {
        std::string result;
    
        for (size_t i = 0; i < elements.size(); ++i) {
            result += elements[i];
            if (i < elements.size() - 1) result += delimiter;
        }
    
        return result;
    }

    static inline bool isDigit(std::string c) {
        std::string str = "0123456789";
        return str.find(c) != std::string::npos;
    }
    

    static inline Color swapColor(Color color) {
        return color == WHITE ? BLACK : WHITE;
    }

    static inline std::string getDisambiguator(InternalMove move, std::vector<InternalMove> moves) {
        const Square from = algebraic(move.from);
        const Square to = algebraic(move.to);
        const PieceSymbol p = move.piece;
    
        int ambiguities = 0;
        int sameRank = 0;
        int sameFile = 0;
    
        for (int i = 0; i < static_cast<int>(moves.size()); i++) {
            const Square ambigFrom = algebraic(moves[i].from);
            const Square ambigTo = algebraic(moves[i].to);
            const PieceSymbol ambigPiece = moves[i].piece;
    
            if (!(p == ambigPiece && from != ambigFrom && to == ambigTo)) continue;
    
            ambiguities++;
    
            if (rank(Ox88.at((int)(from))) == rank(Ox88.at((int)(ambigFrom)))) {
                sameRank++;
            }
            if (file(Ox88.at((int)(from))) == file(Ox88.at((int)(ambigFrom)))) {
                sameFile++;
            }
        }
        if (ambiguities <= 0) return "";
    
        if (sameRank > 0 && sameFile > 0) {
            return squareToString(from);
        }
        else if (sameFile > 0) {
            return std::string(1, squareToString(from).at(1));
        }
        else {
            return std::string(1, squareToString(from).at(0));
        }
    }

    static inline void addMove(std::vector<InternalMove>& moves, Color color, int from, int to, PieceSymbol p, PieceSymbol captured = PieceSymbol::NONE, int flags = BITS_NORMAL) {
        const int r = rank(to);
        if (p == PAWN && (r == RANK_1 || r == RANK_8)) {
            for (int i = 0; i < 4; i++) {
                const PieceSymbol promotion = PROMOTIONS[i];
                InternalMove promotionMove = {
                    color,
                    from,
                    to,
                    p,
                    captured,
                    promotion,
                    flags |= BITS_PROMOTION
                };
                moves.push_back(promotionMove);
            }
        }
        else {
            InternalMove normalMove = {
                color,
                from,
                to,
                p,
                captured,
                PieceSymbol::NONE,
                flags
            };
            moves.push_back(normalMove);
        }
    }
    
    static inline std::string replaceSubstring(const std::string& str, const std::string& from, const std::string& to) {
        size_t startPos = str.find(from);
        if (startPos == std::string::npos) {
            return str;
        }
        return str.substr(0, startPos) + to + str.substr(startPos + from.length());
    }

    static inline PieceSymbol inferPieceType(std::string san) {
        char pieceType = san.at(0);
        if (pieceType >= 'a' && pieceType <= 'h') {
            std::regex pattern("[a-h]\\d.*[a-h]\\d");
            if (std::regex_search(san, pattern)) {
                return PieceSymbol::NONE;
            }
            return PAWN;
        }
        pieceType = static_cast<char>(std::tolower(pieceType));
        if (pieceType == 'o') {
            return KING;
        }
        return charToSymbol(pieceType);
    }

    static inline std::string strippedSan(std::string move) {
        return std::regex_replace(std::regex_replace(move, std::regex("="), ""), std::regex("[+#]?[?!]*$"), "");
    }
    

    static inline std::string trimFen(std::string fen) {
        std::vector<std::string> stpld = split(fen, ' ');
        return join(std::vector<std::string>(stpld.begin(), stpld.begin() + std::min<size_t>(4, stpld.size())), " ");
    }

    static inline int squareTo0x88(const Square& sq) {
        return ((int)(sq) >> 3 << 4) | (int)(sq) & 7;
    }

    static inline bool isValid8x8(const Square& sq) {
        return (int)(sq) >= 0 && (int)(sq) < 64;
    }    

    static inline bool isValid0x88(const int& sq) {
        return (sq & 0x88) == 0;
    }    

    static inline char pieceToChar(const PieceSymbol& p) {
        switch (p) {
        case PAWN: return 'p';
        case KNIGHT: return 'n';
        case BISHOP: return 'b';
        case ROOK: return 'r';
        case QUEEN: return 'q';
        case KING: return 'k';
        case PieceSymbol::NONE: return ' ';
        default: break;
        }
        throw std::invalid_argument("Invalid piece (at pieceToChar)");
    }
    

    static inline PieceSymbol charToSymbol(const char& c) {
        switch (c) {
        case 'p': return PAWN;
        case 'n': return KNIGHT;
        case 'b': return BISHOP;
        case 'r': return ROOK;
        case 'q': return QUEEN;
        case 'k': return KING;
        default: break;
        }
        return PieceSymbol::NONE;
    }
};