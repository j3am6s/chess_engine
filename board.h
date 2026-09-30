#ifndef _BOARD_
#define _BOARD_

#include <string>
#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <typeinfo>
#include <unordered_map>
#include <sstream>
#include <cstring>
#include <cctype> // For std::isupper
#include <set>
#include <bitset>
#include <cstring>
#include <random>

#include "pieces.h"
#include "io.h"

struct pos{
    /*positions with character x and integer y (eg. a2). Each position (square) has a 
    piece in it (or doesn't and then p = nullptr) */
    char x;
    int y;
    piece* p;
    

    void display(){
        if (p == nullptr){
            std::cout << "x";
        }
        else {
            std::cout << p->getName();
        }
    }

    void printPos(){
        std::cout << x << y << std::endl;
    }

    pos(char x, int y, piece* p) : x {x}, y {y}, p {p} {}
    

};

struct board{
    pos* squares[8][8];

    int whiteAttack[8][8] = {0}; //initialize all squares to 0
    int blackAttack[8][8] = {0};

    bool whiteKingHasCastled = false;
    bool blackKingHasCastled = false;
    //This tracks the type of move in case it has to be included in output file
    std::string moveType;

    enum PieceIndex {
        WhitePawn, WhiteKnight, WhiteBishop, WhiteRook, WhiteQueen, WhiteKing,
        BlackPawn, BlackKnight, BlackBishop, BlackRook, BlackQueen, BlackKing,
        PieceCount // Total number of piece types
    };

    // Array to track piece counts
    int pieceCounts[PieceCount];

    uint64_t zobristTable[64][12]; // 64 squares x 12 piece types (6 for each color)
    uint64_t zobristWhiteToMove;

    std::vector<uint64_t> positionHistory;
    //this is to track en passant target
    std::pair<char, int> enPassantTarget = {'-', -1};

    // Bitboards for white and black pieces
    uint64_t whitePieces = 0ULL; // All white pieces
    uint64_t blackPieces = 0ULL; // All black pieces

    // Bitboards for individual piece types
    uint64_t whitePawns = 0ULL, whiteKnights = 0ULL, whiteBishops = 0ULL;
    uint64_t whiteRooks = 0ULL, whiteQueens = 0ULL, whiteKing = 0ULL;

    uint64_t blackPawns = 0ULL, blackKnights = 0ULL, blackBishops = 0ULL;
    uint64_t blackRooks = 0ULL, blackQueens = 0ULL, blackKing = 0ULL;

    void displayBoard(){
        for (int i = 7; i >= 0; --i) {
            std::cout << "---------------------------------"  << std::endl;
            std::cout << "|";
            for (int j = 0; j < 8; ++j) {
                std::cout << " ";
                squares[i][j]->display();
                std::cout << " |";
            }
            std::cout << "  " << std::to_string(i+1) <<std::endl;
        }
        std::cout << "---------------------------------"  << std::endl;
        std::cout << "  a   b   c   d   e   f   g   h  "<<std::endl;
    }

    //returns the pos variable that corresponds to the position 'xy'
    pos* getPos(char x, int y) {
        return squares[y-1][x-'a'];
    }

    board() : moveType("") {
        initializeBoard();
    }

    // Function to print a bitboard as an 8x8 chessboard
    void displayBitboard(uint64_t bitboard) {
        std::cout << "  A B C D E F G H" << std::endl;
        for (int rank = 7; rank >= 0; --rank) { // Ranks 8 to 1
            std::cout << rank + 1 << " ";      // Print rank number
            for (int file = 0; file < 8; ++file) { // Files A to H
                int square = rank * 8 + file;      // Calculate square index
                std::cout << ((bitboard & (1ULL << square)) ? "1 " : "0 ");
            }
            std::cout << rank + 1 << std::endl;    // Print rank number again
        }
        std::cout << "  A B C D E F G H" << std::endl;
    }

    int getPieceIndex(char piece) {
        switch (piece) {
            case 'p': return WhitePawn;
            case 'n': return WhiteKnight;
            case 'b': return WhiteBishop;
            case 'r': return WhiteRook;
            case 'q': return WhiteQueen;
            case 'k': return WhiteKing;
            case 'P': return BlackPawn;
            case 'N': return BlackKnight;
            case 'B': return BlackBishop;
            case 'R': return BlackRook;
            case 'Q': return BlackQueen;
            case 'K': return BlackKing;
            default: return -1; // Invalid piece
        }
    }

    void initializeZobristKeys() {
        std::mt19937_64 rng(12345); // Seed for reproducibility
        std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);

        for (int square = 0; square < 64; ++square) {
            for (int piece = 0; piece < 12; ++piece) {
                zobristTable[square][piece] = dist(rng);
            }
        }

        zobristWhiteToMove = dist(rng);
    }

    //creates starting board
    void initializeBoard() {

        // Initial positions for white pieces
        whitePawns = 0x000000000000FF00ULL;   // Rank 2
        whiteKnights = (1ULL << 1) | (1ULL << 6); // B1 and G1
        whiteBishops = (1ULL << 2) | (1ULL << 5); // C1 and F1
        whiteRooks = (1ULL << 0) | (1ULL << 7);   // A1 and H1
        whiteQueens = (1ULL << 3);                // D1
        whiteKing = (1ULL << 4);                  // E1

        // Combine all white pieces
        whitePieces = whitePawns | whiteKnights | whiteBishops | whiteRooks | whiteQueens | whiteKing;

        // Initial positions for black pieces
        blackPawns = 0x00FF000000000000ULL;   // Rank 7
        blackKnights = (1ULL << 57) | (1ULL << 62); // B8 and G8
        blackBishops = (1ULL << 58) | (1ULL << 61); // C8 and F8
        blackRooks = (1ULL << 56) | (1ULL << 63);   // A8 and H8
        blackQueens = (1ULL << 59);                 // D8
        blackKing = (1ULL << 60);                   // E8

        // Combine all black pieces
        blackPieces = blackPawns | blackKnights | blackBishops | blackRooks | blackQueens | blackKing;

        pieceCounts[WhitePawn] = 8;
        pieceCounts[WhiteKnight] = 2;
        pieceCounts[WhiteBishop] = 2;
        pieceCounts[WhiteRook] = 2;
        pieceCounts[WhiteQueen] = 1;
        pieceCounts[WhiteKing] = 1;

        pieceCounts[BlackPawn] = 8;
        pieceCounts[BlackKnight] = 2;
        pieceCounts[BlackBishop] = 2;
        pieceCounts[BlackRook] = 2;
        pieceCounts[BlackQueen] = 1;
        pieceCounts[BlackKing] = 1;

        for (int i = 2; i < 7; ++i) {
            for (int j = 0; j < 8; ++j) {
                squares[i][j] = new pos(static_cast<char>( 'a' + j), i+1, nullptr);
            }
        }
        for (int i = 0; i < 8; ++i) {
            squares[1][i] = new pos(static_cast<char>( 'a' + i), 2, new pawn(true));
            squares[6][i] = new pos(static_cast<char>( 'a' + i), 7, new pawn(false));
        }

        squares[0][0] = new pos{'a', 1, new rook(true)};
        squares[0][1] = new pos{'b', 1, new knight(true)};
        squares[0][2] = new pos{'c', 1, new bishop(true)};
        squares[0][3] = new pos{'d', 1, new queen(true)};
        squares[0][4] = new pos{'e', 1, new king(true)};
        squares[0][5] = new pos{'f', 1, new bishop(true)};
        squares[0][6] = new pos{'g', 1, new knight(true)};
        squares[0][7] = new pos{'h', 1, new rook(true)};

        squares[7][0] = new pos{'a', 8, new rook(false)};
        squares[7][1] = new pos{'b', 8, new knight(false)};
        squares[7][2] = new pos{'c', 8, new bishop(false)};
        squares[7][3] = new pos{'d', 8, new queen(false)};
        squares[7][4] = new pos{'e', 8, new king(false)};
        squares[7][5] = new pos{'f', 8, new bishop(false)};
        squares[7][6] = new pos{'g', 8, new knight(false)};
        squares[7][7] = new pos{'h', 8, new rook(false)};

        initializeZobristKeys();
    }

    uint64_t calculateZobristHash(bool Player) {
        uint64_t hash = 0;

        // Add piece positions to the hash
        for (int square = 0; square < 64; ++square) {
            if (squares[square / 8][square % 8]->p != nullptr) {
                int pieceType = squares[square / 8][square % 8]->p->typeIndex(); // Map piece type to index (0-11)
                hash ^= zobristTable[square][pieceType];
            }
        }

        if (Player) {
            hash ^= zobristWhiteToMove;
        }

        return hash;
    }

    int positionToIndex(char x, int y) {
        return (x - 'a') + (y - 1) * 8;
    }

    void move2(char x1, int y1, char x2, int y2, bool Player) {
        int from = positionToIndex(x1, y1);
        int to = positionToIndex(x2, y2);

        uint64_t& pieces = Player ? whitePieces : blackPieces;
        uint64_t& opponent = !Player ? whitePieces : blackPieces;

        char p = squares[y1 - 1][x1 - 'a']->p->name;
        uint64_t* bitboard = nullptr;

        if (Player) {
            if (p == 'p') bitboard = &whitePawns;
            else if (p == 'n') bitboard = &whiteKnights;
            else if (p == 'b') bitboard = &whiteBishops;
            else if (p == 'r') bitboard = &whiteRooks;
            else if (p == 'q') bitboard = &whiteQueens;
            else if (p == 'k') bitboard = &whiteKing;
        } else {
            if (p == 'P') bitboard = &blackPawns;
            else if (p == 'N') bitboard = &blackKnights;
            else if (p == 'B') bitboard = &blackBishops;
            else if (p == 'R') bitboard = &blackRooks;
            else if (p == 'Q') bitboard = &blackQueens;
            else if (p == 'K') bitboard = &blackKing;
        }

        // Castling logic
        if (p == 'k' || p == 'K') {
            int rank = Player ? 0 : 7;

            // Kingside castling
            if (x1 == 'e' && y1 - 1 == rank && x2 == 'g' && y2 - 1 == rank) {
                // Move the rook
                int rookFrom = positionToIndex('h', rank + 1);
                int rookTo = positionToIndex('f', rank + 1);

                uint64_t* rookBitboard = Player ? &whiteRooks : &blackRooks;
                if (rookBitboard) {
                    *rookBitboard &= ~(1ULL << rookFrom);
                    *rookBitboard |= (1ULL << rookTo);
                }

                pieces &= ~(1ULL << rookFrom);
                pieces |= (1ULL << rookTo);

                pos* rookPos = squares[rank][7];
                squares[rank][5]->p = rookPos->p;
                squares[rank][5]->p->moved = true;
                rookPos->p = nullptr;

                Player ? whiteKingHasCastled = true : blackKingHasCastled = true;
            }

            // Queenside castling
            if (x1 == 'e' && y1 - 1 == rank && x2 == 'c' && y2 - 1 == rank) {
                // Move the rook
                int rookFrom = positionToIndex('a', rank + 1);
                int rookTo = positionToIndex('d', rank + 1);

                uint64_t* rookBitboard = Player ? &whiteRooks : &blackRooks;
                if (rookBitboard) {
                    *rookBitboard &= ~(1ULL << rookFrom);
                    *rookBitboard |= (1ULL << rookTo);
                }

                pieces &= ~(1ULL << rookFrom);
                pieces |= (1ULL << rookTo);

                pos* rookPos = squares[rank][0];
                squares[rank][3]->p = rookPos->p;
                squares[rank][3]->p->moved = true;
                rookPos->p = nullptr;

                Player ? whiteKingHasCastled = true : blackKingHasCastled = true;
            }
        }

        // Handle en passant capture
        if (p == 'p' || p == 'P') {
            if (enPassantTarget == std::make_pair(x2, y2)) {
                int captureRow = Player ? y2 - 1 : y2 + 1; 
                char captureFile = x2;

                int captureIndex = positionToIndex(captureFile, captureRow + 1);
                uint64_t* capturedBitboard = Player ? &blackPawns : &whitePawns;

                if (capturedBitboard) {
                    *capturedBitboard &= ~(1ULL << captureIndex); // Remove the captured pawn
                }
                opponent &= ~(1ULL << captureIndex); // Remove the captured pawn from the opponent bitboard

                squares[captureRow-1][captureFile - 'a']->p = nullptr; // Remove captured pawn from the board
            }

            // Set en passant target if the pawn moves two squares forward
            if (abs(y2 - y1) == 2) {
                enPassantTarget = std::make_pair(x1, (y1 + y2) / 2);
            } else {
                enPassantTarget = std::make_pair(0, 0); // Clear en passant target
            }
        } else {
            enPassantTarget = std::make_pair(0, 0); // Clear en passant target
        }

        // Update the bitboards for the moving piece
        if (bitboard) {
            *bitboard &= ~(1ULL << from); // Remove the piece from its original position
            *bitboard |= (1ULL << to);    // Add the piece to its new position
        }

        pieces &= ~(1ULL << from); // Remove from the pieces bitboard
        pieces |= (1ULL << to);    // Add to the pieces bitboard

        // Handle capturing logic
        if (squares[y2 - 1][x2 - 'a']->p != nullptr) {
            char op = squares[y2 - 1][x2 - 'a']->p->name;
            pieceCounts[getPieceIndex(op)]--;
            uint64_t* bitboard2 = nullptr;

            if (!Player) {
                if (op == 'p') bitboard2 = &whitePawns;
                else if (op == 'n') bitboard2 = &whiteKnights;
                else if (op == 'b') bitboard2 = &whiteBishops;
                else if (op == 'r') bitboard2 = &whiteRooks;
                else if (op == 'q') bitboard2 = &whiteQueens;
                else if (op == 'k') bitboard2 = &whiteKing;
            } else {
                if (op == 'P') bitboard2 = &blackPawns;
                else if (op == 'N') bitboard2 = &blackKnights;
                else if (op == 'B') bitboard2 = &blackBishops;
                else if (op == 'R') bitboard2 = &blackRooks;
                else if (op == 'Q') bitboard2 = &blackQueens;
                else if (op == 'K') bitboard2 = &blackKing;
            }

            if (bitboard2) {
                *bitboard2 &= ~(1ULL << to); // Remove the captured piece
            }

            opponent &= ~(1ULL << to); // Remove from opponent bitboard
        }

        pos* p1 = squares[y1 - 1][x1 - 'a'];
        pos* p2 = squares[y2 - 1][x2 - 'a'];

        p2->p = p1->p;
        p2->p->moved = true; // Mark as moved
        p1->p = nullptr;

        if (p2->p->name == 'p' || p2->p->name == 'P') {
            p2->p->moves = p2->p->white ? std::vector<std::pair<int, int>>{{1, 0}} : std::vector<std::pair<int, int>>{{-1, 0}};
        }

        // Handle pawn promotion
        if ((Player && y2 == 8 && p == 'p') || (!Player && y2 == 1 && p == 'P')) {
            char promotionPiece = Player ? 'q' : 'Q';
            uint64_t* promotionBitboard = Player ? &whiteQueens : &blackQueens;

            pieceCounts[getPieceIndex(p)]--;
            pieceCounts[getPieceIndex(promotionPiece)]++;

            if (bitboard) {
                *bitboard &= ~(1ULL << to);
            }
            *promotionBitboard |= (1ULL << to);

            p2->p = new queen(Player);
        }
    }

    bool verifyCheck2(bool Player) {
        uint64_t kingBitboard = Player ? whiteKing : blackKing;
        int (*opponentAttack)[8] = Player ? blackAttack : whiteAttack;

        // Locate the king's position (should be exactly one bit set in the bitboard)
        if (kingBitboard == 0) {
            // No king on the board (invalid state, assume in check)
            return true;
        }

        // Find the king's position
        int kingIndex = __builtin_ctzll(kingBitboard); // Get the index of the least significant bit
        int kingX = kingIndex % 8; // File (column)
        int kingY = kingIndex / 8; // Rank (row)

        // Check if the king's position is under attack
        if (opponentAttack[kingY][kingX] != 0) {
            return true; // The king is in check
        }

        return false; // The king is not in check
    }

    std::vector<std::pair<char, int>> calculateMoves4(pos* square, int num) {
        std::vector<std::pair<char, int>> possibleMoves;
        if (square->p == nullptr) {
            return possibleMoves; // No piece on this square
        }
        char pieceType = square->p->name;
        bool isWhite = square->p->white;
        int x = square->x - 'a';
        int y = square->y - 1;
        std::vector<std::pair<int, int>> moves = square->p->moves;
        //for pieces that can move as far as possible in their directions
        if (square->p->infinity){
            for (const auto& move : moves) {
                int newY = square->y-1;
                char newX = square->x;
                while (0 <= newY+move.first && newY+move.first <= 7 && 'a' <= newX+move.second && newX+move.second <= 'h'){
                    newY = newY + move.first;
                    newX = newX + move.second;
                    auto& target = squares[newY][newX-'a']->p;
                    auto& targetAttack = isWhite ? whiteAttack[newY][newX-'a'] : blackAttack[newY][newX-'a'];
                    if (target == nullptr){
                        targetAttack++; //add the moves as an potential attack move for white or black 
                        possibleMoves.push_back({newX, newY+1});
                    } else if (target->white == isWhite){
                        targetAttack++;
                        break;
                    } else{
                        possibleMoves.push_back({newX, newY+1});
                        targetAttack++;
                        break;
                    }
                }
            }
            
        } else{ //for other pieces that only have to check for moves that are under their 'moves' vector.
            //if square (the current position) contains a pawn, we check if it can eat any opponent piece 
            
            if (pieceType == 'p' || pieceType == 'P'){
                
                int newY;
                if (pieceType == 'p'){
                    newY = square->y; //increases height by 1
                }else{
                    newY = square->y - 2; //decreases height by 1
                }
                char newX1 = square->x - 1 ;
                char newX2 = square->x + 1;

                auto& target1 = squares[newY][newX1-'a']->p;
                auto& target2 = squares[newY][newX2-'a']->p;

                auto& targetAttack1 = isWhite ? whiteAttack[newY][newX1-'a'] : blackAttack[newY][newX1-'a'];
                auto& targetAttack2 = isWhite ? whiteAttack[newY][newX2-'a'] : blackAttack[newY][newX2-'a'];
                if (0 <= newY && newY <= 7 && 'a' <= newX1 && newX1 <= 'h' && target1 != nullptr && target1->white != isWhite ){
                    possibleMoves.push_back({newX1, newY+1});
                    targetAttack1++;
                }
                if (0 <= newY && newY <= 7 && 'a' <= newX2 && newX2 <= 'h' && target2 != nullptr && target2->white != isWhite ){
                    possibleMoves.push_back({newX2, newY+1});
                    targetAttack2++;
                }
                if (enPassantTarget==std::make_pair(newX1, newY+1) && target1 == nullptr) {
                        possibleMoves.push_back({newX1, newY+1});
                }
                if (enPassantTarget==std::make_pair(newX2, newY+1) && target2 == nullptr) {
                        possibleMoves.push_back({newX2, newY+1});
                }

                for (const auto& move : moves) { 
                    int w = isWhite ? 1 : -1;
                    int newY = square->y + move.first - 1;
                    char newX = square->x + move.second;
                    if ((move.first == -1 or move.first == 1) or (squares[square->y + move.first - w - 1][square->x-'a']->p == nullptr)){

                        if(0 <= newY && newY <= 7 && 'a' <= newX && newX <= 'h' && squares[newY][newX-'a']->p == nullptr){
                            if (squares[newY][newX-'a']->p == nullptr){
                                possibleMoves.push_back({newX, newY+1});
                            } else if (squares[newY][newX-'a']->p->white != isWhite){
                                possibleMoves.push_back({newX, newY+1});
                            }
                        }
                    }
                }
                
            } else{
                

                for (const auto& move : moves) { 
                    int newY = square->y + move.first - 1;
                    char newX = square->x + move.second;
                    auto& target = squares[newY][newX-'a']->p;
                    auto& targetAttack = isWhite ? whiteAttack[newY][newX-'a'] : blackAttack[newY][newX-'a'];
                    if(0 <= newY && newY <= 7 && 'a' <= newX && newX <= 'h'){
                        if (target == nullptr){
                            possibleMoves.push_back({newX, newY+1});
                            targetAttack++;
                        } else if (target->white != isWhite){
                            possibleMoves.push_back({newX, newY+1});
                            targetAttack++;
                        } else if (target != nullptr && target->white == isWhite ){
                            targetAttack++;
                        }
                    }
                }
                
            }
            //castling
            if (pieceType=='k' || pieceType=='K') {
                
                    
                //check if white or black
                int row = (pieceType == 'k') ? 0 : 7;
                char r = (pieceType == 'k') ? 'r' : 'R';
                // Kingside castling logic
                if (!square->p->moved &&
                    squares[row][7]->p!=nullptr && 
                    squares[row][6]->p==nullptr &&
                    squares[row][5]->p==nullptr &&
                    num != 0 &&
                    squares[row][7]->p->name==r && 
                    !squares[row][7]->p->moved) {
                    
                    if (pieceType == 'k'){
                        allPossibleMoves2(false, 0);
                        if (blackAttack[row][7] == 0 &&
                            blackAttack[row][6] == 0 &&
                            blackAttack[row][5] == 0 &&
                            blackAttack[row][4] == 0) {
                                possibleMoves.push_back({'g', row+1});
                        }
                    } else {
                        allPossibleMoves2(true, 0);
                        if (whiteAttack[row][7] == 0 &&
                            whiteAttack[row][6] == 0 &&
                            whiteAttack[row][5] == 0 &&
                            whiteAttack[row][4] == 0) {
                                possibleMoves.push_back({'g', row+1});
                        }
                    }
                }

                // Queenside castling logic
                if (!square->p->moved &&
                    squares[row][0]->p!=nullptr && 
                    squares[row][1]->p==nullptr &&
                    squares[row][2]->p==nullptr &&
                    squares[row][3]->p==nullptr &&
                    num != 0 &&
                    squares[row][0]->p->name==r &&
                    !squares[row][0]->p->moved) {
                    
                    if (pieceType == 'k'){
                        allPossibleMoves2(false, 0);
                        if (blackAttack[row][0] == 0 &&
                            blackAttack[row][1] == 0 &&
                            blackAttack[row][2] == 0 &&
                            blackAttack[row][3] == 0 &&
                            blackAttack[row][4] == 0) {
                                possibleMoves.push_back({'c', row+1});
                            }
                    } else {
                        allPossibleMoves2(true, 0);
                        if (whiteAttack[row][0] == 0 &&
                            whiteAttack[row][1] == 0 &&
                            whiteAttack[row][2] == 0 &&
                            whiteAttack[row][3] == 0 &&
                            whiteAttack[row][4] == 0) {
                                possibleMoves.push_back({'c', row+1});
                        }
                    }
                }
            }
        }
        


        return possibleMoves;
    }

    std::vector<std::pair<std::pair<char, int>, std::pair<char, int>>> allPossibleMoves2(bool Player, int num) {
        std::vector<std::pair<std::pair<char, int>, std::pair<char, int>>> allMoves;

        // Initialize attack boards to 0
        if (Player) {
            std::fill(&whiteAttack[0][0], &whiteAttack[0][0] + 64, 0);
        } else {
            std::fill(&blackAttack[0][0], &blackAttack[0][0] + 64, 0);
        }

        // Select appropriate bitboards
        uint64_t pawns = Player ? whitePawns : blackPawns;
        uint64_t knights = Player ? whiteKnights : blackKnights;
        uint64_t bishops = Player ? whiteBishops : blackBishops;
        uint64_t rooks = Player ? whiteRooks : blackRooks;
        uint64_t queens = Player ? whiteQueens : blackQueens;
        uint64_t king = Player ? whiteKing : blackKing;

        // Iterate over each bitboard and generate moves
        auto processBitboard = [&](uint64_t bitboard, char pieceType) {
            while (bitboard) {
                // Find the index of the least significant bit (LSB)
                int fromIndex = __builtin_ctzll(bitboard);

                // Convert index to board coordinates
                char fromX = 'a' + (fromIndex % 8); // File (column)
                int fromY = 1 + (fromIndex / 8);    // Rank (row)

                // Calculate possible moves for this piece
                std::vector<std::pair<char, int>> possibleMoves = calculateMoves4(squares[fromY-1][fromX-'a'], num);

                // Add moves to the allMoves vector
                for (const auto& move : possibleMoves) {
                    allMoves.push_back({{fromX, fromY}, move});
                }

                // Clear the LSB to move to the next piece
                bitboard &= bitboard - 1;
            }
        };

        // Process all bitboards
        processBitboard(pawns, 'P');   // Pawns
        processBitboard(knights, 'N'); // Knights
        processBitboard(bishops, 'B'); // Bishops
        processBitboard(rooks, 'R');   // Rooks
        processBitboard(queens, 'Q');  // Queens
        processBitboard(king, 'K');    // King

        return allMoves;
    }

    void displayAllPossibleMoves(bool Player){
            std::vector< std::pair <std::pair<char, int> , std::pair<char, int> > > allMoves = allPossibleMoves2(Player, 1);
            for (int i =0; i < allMoves.size(); i++){
                std::cout << allMoves.at(i).first.first << allMoves.at(i).first.second<< allMoves.at(i).second.first << allMoves.at(i).second.second << std::endl;
            }
    }

    int teamPoints2(bool Player) {
        if (!Player) { // Black's points
            return (9 * pieceCounts[BlackQueen] +
                    5 * pieceCounts[BlackRook] +
                    3 * pieceCounts[BlackKnight] +
                    3 * pieceCounts[BlackBishop] +
                    1 * pieceCounts[BlackPawn]);
        } else { // White's points
            return (9 * pieceCounts[WhiteQueen] +
                    5 * pieceCounts[WhiteRook] +
                    3 * pieceCounts[WhiteKnight] +
                    3 * pieceCounts[WhiteBishop] +
                    1 * pieceCounts[WhitePawn]);
        }
    }

    int calculateGamePhase() {
        int totalPieces = __builtin_popcountll(whitePieces | blackPieces);
        int maxPieces = 32; // Total pieces at the start of the game
        return static_cast<int>((1.0 - (totalPieces / static_cast<double>(maxPieces))) * 100.0);
    }

    int evaluateCastling(bool Player) {
        if (Player && whiteKingHasCastled) {
            return 20; 
        }
        if (!Player && blackKingHasCastled) {
            return 20; 
        }
        return 0; 
    }

    bool kingHasMoved(bool Player) {
        int row = Player ? 0 : 7;
        char name = Player ? 'k' : 'K';
        pos* kingSquare = squares[row][4];
        if (kingSquare->p != nullptr && kingSquare->p->name == name && kingSquare->p->moved == false){
            return 1;
        }
        return 0;
    }

    int getPawnPositionBonus(bool Player, int squareIndex) {
        int gamePhase = calculateGamePhase();
        int pawnPositionTable[64];
        if(gamePhase < 60){
            int temp[] = {
                0,  5,  5,  10,  10,  5,  5,  0, 
                30, 30, 30,  20,  20, 30, 30,  30, 
                5, 10, 20,  40,  40, 20, 10,  5, 
                10, 20, 30,  50,  50, 30, 20, 10, 
                25, 30, 40,  60,  60, 40, 30, 25, 
                30, 40, 50,  70,  70, 50, 40, 30, 
                50, 60, 70,  80,  80, 70, 60, 50, 
                900, 900, 900, 900, 900, 900, 900, 900  
            };
            std::copy(std::begin(temp), std::end(temp), pawnPositionTable);
        } else{
            int temp[] = {
                0,  0,  0,  0,  0,  0,  0,  0, 
                30, 30, 30,  20,  20, 30, 30,  30, 
                40, 40, 40,  40,  40, 40, 40,  40, 
                50, 50, 50,  50,  50, 50, 50, 50, 
                60, 60, 60,  60,  60, 60, 60, 60, 
                30, 40, 50,  70,  70, 70, 70, 70, 
                80, 80, 80,  80,  80, 80, 80, 80, 
                900, 900, 900, 900, 900, 900, 900, 900  
            };
            std::copy(std::begin(temp), std::end(temp), pawnPositionTable);
        }
        // Scale bonuses based on the game phase
        double scalingFactor = 1.0 + (1.0 - (gamePhase / 100.0)); // Scale more in the endgame

        int bonus = Player ? pawnPositionTable[squareIndex] : pawnPositionTable[63 - squareIndex];
        return static_cast<int>(bonus * scalingFactor);
    }



    int evaluateKingSafety(bool Player) {
        uint64_t king = Player ? whiteKing : blackKing;
        int kingIndex = __builtin_ctzll(king);

        int rank = kingIndex / 8;
        int score = 0;
        if (rank < 1 || rank > 6) { 
            score += 5;
        }
        // if (moveCount <= 10) {
        //     if ((Player && rank == 0 && (file == 3 || file == 4)) ||  // White king on e1 or d1
        //         (!Player && rank == 7 && (file == 3 || file == 4))) { // Black king on e8 or d8
        //         score -= 100; // Penalty for staying in the center
        //     }
        // }
        return score;
    }


    int evaluateCurrent(bool Player){
        int materialScore = teamPoints2(Player) - teamPoints2(!Player);
        int positionalScore = 0;
        int kingSafety = evaluateKingSafety(Player) - evaluateKingSafety(!Player);
        int castling = evaluateCastling(Player) - evaluateCastling(!Player);
        int king = kingHasMoved(Player) - kingHasMoved(!Player);
        int check = verifyCheck2(!Player)*5 - verifyCheck2(Player)*5;

        uint64_t pieces = Player ? whitePawns : blackPawns;
        while (pieces) {
            int index = __builtin_ctzll(pieces); 
            positionalScore += getPawnPositionBonus(Player, index); 
            pieces &= pieces - 1; 
        }
        pieces = !Player ? whitePawns : blackPawns;
        while (pieces) {
            int index = __builtin_ctzll(pieces); 
            positionalScore -= getPawnPositionBonus(!Player, index); 
            pieces &= pieces - 1; 
        }
        return materialScore*20 + positionalScore*1 + kingSafety*3 + castling * 5 + 70*king + check;

    }

    struct MoveState {
        // Moved piece and its original position
        piece* movedPiece;
        pos* movedSquare;
        bool movedFirst;

        pos* castlingRookSquare;
        pos* castlingRookTargetSquare;
        piece* castlingRookPiece;
        bool castlingRookMoved;

        std::pair<char, int> enPassantTarget;
        // Captured piece (if any) and its original position
        pos* capturedSquare;
        piece* capturedPiece;
        bool movedSecond;

        int pieceCounts[PieceCount];

        bool whiteKingHasCastled;
        bool blackKingHasCastled;

        // Copies of critical variables
        uint64_t whitePawns, whiteKnights, whiteBishops, whiteRooks, whiteQueens, whiteKing;
        uint64_t blackPawns, blackKnights, blackBishops, blackRooks, blackQueens, blackKing;

        uint64_t whitePieces; // All white pieces combined
        uint64_t blackPieces; // All black pieces combined

        // Attack boards
        int whiteAttack[8][8];
        int blackAttack[8][8];
    };

    MoveState saveState(pos* a, pos* b) {
        MoveState state;

        // std::cout << "1" << std::endl;
        // Save piece information
        state.movedPiece = a->p;
        // std::cout << "1" << std::endl;
        state.movedSquare = a;
        // std::cout << a->p << std::endl;
        // displayBoard();
        state.movedFirst = a->p->moved;
        // std::cout << "1" << std::endl;
        state.capturedSquare = b;
        // std::cout << "1" << std::endl;
        state.capturedPiece = b->p;
        if (b->p != nullptr) {
            state.movedSecond = b->p->moved;
        }

        state.enPassantTarget = enPassantTarget;

        state.castlingRookSquare = nullptr;
        state.castlingRookTargetSquare = nullptr;
        state.castlingRookPiece = nullptr;
        // std::cout << "1" << std::endl;
        if ((a->p->name == 'k' || a->p->name == 'K') && a->x == 'e') {
            int rank = a->p->name == 'k' ? 0 : 7;
            // std::cout << "1" << std::endl;
            if (b->x == 'g') { // Kingside castling
                // std::cout << "2" << std::endl;
                state.castlingRookSquare = squares[rank][7];
                state.castlingRookTargetSquare = squares[rank][5];
            } else if (b->x == 'c') { // Queenside castling
                // std::cout << "3" << std::endl;
                state.castlingRookSquare = squares[rank][0];
                state.castlingRookTargetSquare = squares[rank][3];
            }
            // std::cout << "2" << std::endl;
            if (state.castlingRookSquare != nullptr && state.castlingRookSquare->p != nullptr) {
                // std::cout << "4" << std::endl;
                state.castlingRookPiece = state.castlingRookSquare->p;
                state.castlingRookMoved = state.castlingRookPiece->moved;
            }
            // std::cout << "3" << std::endl;
        }
        // std::cout << "2" << std::endl;

        std::memcpy(state.pieceCounts, pieceCounts, sizeof(pieceCounts));

        // Save all bitboards
        state.whitePawns = whitePawns;
        state.whiteKnights = whiteKnights;
        state.whiteBishops = whiteBishops;
        state.whiteRooks = whiteRooks;
        state.whiteQueens = whiteQueens;
        state.whiteKing = whiteKing;

        state.blackPawns = blackPawns;
        state.blackKnights = blackKnights;
        state.blackBishops = blackBishops;
        state.blackRooks = blackRooks;
        state.blackQueens = blackQueens;
        state.blackKing = blackKing;

        state.whitePieces = whitePieces;
        state.blackPieces = blackPieces;

        state.whiteKingHasCastled = whiteKingHasCastled;
        state.blackKingHasCastled = blackKingHasCastled;
        // Save attack boards
        std::memcpy(state.whiteAttack, whiteAttack, sizeof(whiteAttack));
        std::memcpy(state.blackAttack, blackAttack, sizeof(blackAttack));

        return state;
    }

    void restoreState(const MoveState& state) {
        // Restore piece positions
        state.movedSquare->p = state.movedPiece;
        state.capturedSquare->p = state.capturedPiece;
        if (state.capturedSquare->p != nullptr) {
            state.capturedSquare->p->moved = state.movedSecond;
        }
        state.movedSquare->p->moved = state.movedFirst;
        if (state.capturedPiece != nullptr) {
            state.capturedSquare->p->alive = true;
        }

        if (state.castlingRookSquare != nullptr && state.castlingRookTargetSquare != nullptr && state.castlingRookPiece != nullptr) {
        state.castlingRookSquare->p = state.castlingRookPiece;
        if (state.castlingRookSquare->p != nullptr) { 
            state.castlingRookSquare->p->moved = state.castlingRookMoved;
        }
        state.castlingRookTargetSquare->p = nullptr;
        }

        enPassantTarget = state.enPassantTarget;
        // Restore all bitboards
        whitePawns = state.whitePawns;
        whiteKnights = state.whiteKnights;
        whiteBishops = state.whiteBishops;
        whiteRooks = state.whiteRooks;
        whiteQueens = state.whiteQueens;
        whiteKing = state.whiteKing;

        blackPawns = state.blackPawns;
        blackKnights = state.blackKnights;
        blackBishops = state.blackBishops;
        blackRooks = state.blackRooks;
        blackQueens = state.blackQueens;
        blackKing = state.blackKing;

        whitePieces = state.whitePieces;
        blackPieces = state.blackPieces;

        whiteKingHasCastled = state.whiteKingHasCastled;
        blackKingHasCastled = state.blackKingHasCastled;

        std::memcpy(pieceCounts, state.pieceCounts, sizeof(pieceCounts));
        // Restore attack boards
        std::memcpy(whiteAttack, state.whiteAttack, sizeof(whiteAttack));
        std::memcpy(blackAttack, state.blackAttack, sizeof(blackAttack));
    }

    std::string getNextMove(bool Player){
        std::vector< std::pair <std::pair<char, int> , std::pair<char, int> > > possibleMoves = allPossibleMoves2(Player, 1);
        std::vector< std::pair <std::pair<char, int> , std::pair<char, int> > > possibleMoves2 = allPossibleMoves2(!Player, 1);
        verifyCheck2(Player) ? std::cout << "CHECK!" << std::endl : std::cout << "";
        int bestEval = -999999999;
        std::pair <std::pair<char, int> , std::pair<char, int> > bestMove;
        //board virtualboard(*this);
        for (auto& move : possibleMoves){
            // std::cout << "1" << std::endl;
            int eval;
            eval = helperMethod(move.first, move.second, 3, !Player, Player);
            // std::cout << move.first.first << move.first.second << move.second.first << move.second.second << eval << std::endl;
            // std::cout << "2" << std::endl;
            if (eval > bestEval){
                // std::cout << "3" << std::endl;
                bestMove = move;
                // std::cout << "4" << std::endl;
                bestEval = eval;
            }
            // std::cout << "5" << std::endl;
        }
        std::stringstream res;
        res << bestMove.first.first << bestMove.first.second << bestMove.second.first << bestMove.second.second;
        // std::cout << "7" << std::endl;
        return res.str();
    }

    int helperMethod(std::pair<char, int> from, std::pair<char, int> to, int num, bool w, bool Player){
        // std::cout << "1" << std::endl;
        MoveState state = saveState(getPos(from.first, from.second), getPos(to.first, to.second));
        move2(from.first, from.second, to.first, to.second, !w);
        if(num == 3) {
            int repetitionCount = std::count(positionHistory.begin(), positionHistory.end(), calculateZobristHash(!Player));
            if (repetitionCount >= 2) {
                int eval = evaluateCurrent(Player);
                if (eval >= 0){
                    // std::cout << "3" << std::endl;
                    restoreState(state);
                    return -99999999;
                }
            }
        }
        if (num == 0){
            int eval = evaluateCurrent(Player);
            restoreState(state);
            return eval;
        }
        std::vector< std::pair <std::pair<char, int> , std::pair<char, int> > > possibleMoves = allPossibleMoves2(w, 1);
        if (verifyCheck2(!w)){
            restoreState(state);
            if (w == Player) {
                return 999999+num;
            } else {
                return -999999-num;
            }
        }
        std::vector< int > allEvals = {};
        for(auto& move : possibleMoves){
            int eval = helperMethod(move.first, move.second, num-1, !w, Player);
            allEvals.push_back(eval);
        }
        // std::cout << "10" << std::endl;
        // std::cout << "11" << std::endl;
        // std::cout << allEvals.size() << std::endl;
        if(w == Player){
            int m = *std::max_element(allEvals.begin(), allEvals.end());
            restoreState(state);
            return m;
        }
        int m = *std::min_element(allEvals.begin(), allEvals.end());
        // std::cout << m << " " << verifyCheck2(w);
        if (m >= 99999){
            std::vector< std::pair <std::pair<char, int> , std::pair<char, int> > > f = allPossibleMoves2(!w,1);
            if(!verifyCheck2(w)){
                restoreState(state);
                return -999999;
            }
        }
        restoreState(state);
        return m;
    }
};

#endif
