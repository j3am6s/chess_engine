#ifndef _PIECES_
#define _PIECES_

#include <string>
#include <iostream>
#include <vector>
#include <utility>

struct pos;

struct piece{
    /*A vector is practically the equivalent of a python list and a pair is like a tuple
    This variable will hold the possible directions that a piece can go in.
    For example for a pawn it would be {{1,0}} because it can only move forward one (ignore exceptions for now).
    The calculating moves function is now in pos structure in board.h.*/
    std::vector<std::pair<int, int>> moves;

    bool alive;

    /* Variable which states whether this piece can move infinitely across rows or diagonals
     (true for bishop, rook and queen false for rest)*/
    bool infinity;

    //name is capitalized for black team to differentiate them 
    char name;

    //variable to say if piece is on white team or not
    bool white;

    bool moved; //useful for pawn and king: for pawn allows to know if we can make them advance from 2 cases
                // for king allows to know if we can castle or not

    char getName(){
        return name;
    }

    int typeIndex() const {
        switch (name) {
            case 'P': return 0;
            case 'N': return 1;
            case 'B': return 2;
            case 'R': return 3;
            case 'Q': return 4;
            case 'K': return 5;
            case 'p': return 6;
            case 'n': return 7;
            case 'b': return 8;
            case 'r': return 9;
            case 'q': return 10;
            case 'k': return 11;
            default: return -1; // Invalid piece
        }
    }
    
    piece(bool infinity, const char name, bool white) : alive {true}, infinity {infinity}, name {name}, white {white}, moved{false} {}
    
};

struct pawn:piece { //Problem: does not update correctly the fields moves when it is changed in the board structure
    pawn(bool white) : piece(false, white ? 'p' : 'P', white) {
        moves = white ? std::vector<std::pair<int, int>>{{2,0}, {1, 0}} : std::vector<std::pair<int, int>>{{-2,0}, {-1, 0}};
    }
    
};

struct queen:piece {
    queen(bool white) : piece(true, white ? 'q' : 'Q', white) {
        moves = {{1,0}, {0,1}, {0, -1}, {-1, 0}, {-1, -1}, {1,1}, {-1, 1}, {1, -1}};
    }
};

struct king:piece {
    king(bool white) : piece(false, white ? 'k' : 'K', white) {
        moves = {{1,0}, {0,1}, {0, -1}, {-1, 0}, {-1, -1}, {1,1}, {-1, 1}, {1, -1}};
    }
};

struct knight:piece {
    knight(bool white) : piece(false, white ? 'n' : 'N', white) {
        moves = {{2,1}, {2,-1}, {-2, 1}, {-2,-1}, {-1,-2}, {-1,2}, {1,2}, {1,-2}};
    }
};

struct bishop:piece {
    bishop(bool white) : piece(true, white ? 'b' : 'B', white) {
        moves = {{1,1}, {-1,-1}, {1,-1}, {-1,1}};
    }
};

struct rook:piece {
    rook(bool white) : piece(true, white ? 'r' : 'R', white) {
        moves = {{1,0}, {-1,0}, {0, 1}, {0,-1}};
    }
};

#endif
