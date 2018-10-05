/*
    Chess.hpp:
        Library for the definition of the ChessState class.
        The ChessState contains the player and a vector of legalMoves (calculated via a method of the class itself).
        It contains a method to determine if the state is a final state of the game, and eventually who is the winner.
        Also, the method simulateGame performs a random game simulation starting from the current state, and the returns the reward.
        It contains a routine to graphically print the state in the console and a destructor.

        @author: Massimiliano Chiappini 
        @contact: massimilianochiappini@gmail.com
        @version: 0.2
*/


#ifndef CHESS_HPP
#define CHESS_HPP

#include <vector>
#include <string>
#include <array>
#include <unordered_map>



//ENUMS
//There are 6 piece types
enum piece_type {
    PAWN,
    ROOK,
    KNIGHT,
    BISHOP,
    QUEEN,
    KING
};


//The board of chess is a 64 board, in which each piece is represented by a number:
enum piece {
    empty,
    white_pawn,
    white_pawn2,
    white_rook,
    white_knight,
    white_bishop,
    white_queen,
    white_king,
    black_pawn,
    black_pawn2,
    black_rook,
    black_knight,
    black_bishop,
    black_queen,
    black_king
};



//And a symbol
#define N_CHESS_PIECES 15
const std::array<std::string,N_CHESS_PIECES> PIECES_SYMBOLS = {" ", "\u2659", "\u2659", "\u2656", "\u2658", "\u2657", "\u2655", "\u2654", "\u265F", "\u265F", "\u265C", "\u265E", "\u265D", "\u265B", "\u265A"};
//and a color, either white (+1) or black (-1)
const std::array<int,N_CHESS_PIECES> PIECES_COLORS = {0, 1, 1, 1, 1, 1, 1, 1, -1, -1, -1, -1, -1, -1, -1};
const std::array<int,N_CHESS_PIECES> PIECES_TYPES = {-1, PAWN, PAWN, ROOK, KNIGHT, BISHOP, QUEEN, KING, PAWN, PAWN, ROOK, KNIGHT, BISHOP, QUEEN, KING};
//Also, the pieces of the chessboard have a code
const std::array<std::string,64> CHESSBOARD_SQUARE_NAMES = {"a1","a2","a3","a4","a5","a6","a7","a8","b1","b2","b3","b4","b5","b6","b7","b8","c1","c2","c3","c4","c5","c6","c7","c8","d1","d2","d3","d4","d5","d6","d7","d8","e1","e2","e3","e4","e5","e6","e7","e8","f1","f2","f3","f4","f5","f6","f7","f8","g1","g2","g3","g4","g5","g6","g7","g8","h1","h2","h3","h4","h5","h6","h7","h8"};
//Each position of the chessboard can be described either by an integer scalar n or by an integer vector (i, j)
//In particular n = 8 * i + j
//And:
//i = n / 8
//j = n % 8
typedef std::array<int,64> ChessBoard;
const ChessBoard CHESS_STARTING_BOARD = {white_rook, white_knight, white_bishop, white_queen, white_king, white_bishop, white_knight, white_rook, white_pawn, white_pawn, white_pawn, white_pawn, white_pawn, white_pawn, white_pawn, white_pawn, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, black_pawn, black_pawn, black_pawn, black_pawn, black_pawn, black_pawn, black_pawn, black_pawn, black_rook, black_knight, black_bishop, black_queen, black_king, black_bishop, black_knight, black_rook};
//It is also useful to save for later the possible knight jumps
const std::array<std::array<int,2>,8> KNIGHT_JUMPS = {{{-2,-1},{-2,1},{-1,-2},{-1,2},{1,-2},{1,2},{2,-1},{2,1}}};


#define MAX_SIMULATION_LENGTH 1000

class ChessState;

//Class representing a move
struct ChessMove {
    //Properties
    ChessState *finalState;
    
    //Constructors
    ChessMove(ChessState*, int, int, int);

    //Move identifiers for the networks
    int piece;
    int startingSquare;
    int id;
};


//Class representing a state of the game
class ChessState {
  protected:
  	//CHESS STATE DESCRIBERS
    //Current player
    int player;
    //Board
    ChessBoard board;
    //Position of the kings
    std::array<int,2> kingPositions;
    //Is castle still legal
    std::array<std::array<int,2>,2> possibleCastling;
    
    //Legal moves from this state
    std::vector<ChessMove> legalMoves;

    //Have the legal moves been computed yet?
    bool computedLegalMoves;
    
    
    
    
   public:
    //CONSTRUCTORS
    ChessState(ChessBoard, std::array<int,2>, std::array<std::array<int,2>,2>, int);
    ChessState(ChessBoard, std::array<int,2>, int);
    ChessState(ChessBoard, int);
    ChessState(void);


    //OVERLOADED OPERATORS
    bool operator==(const ChessState& otherState) {
        if (typeid((*this)) != typeid(otherState)) {
            return false;
        }
        else {
            return isEqual(otherState);
        }
    }
       

    bool isEqual(const ChessState& otherState) {
    	if((this->board == otherState.board) && (this->player == otherState.player)) {
            return true;
        }
        else {
            return false;
        }
    }
  
    
    //SET/GET methods
    int getPlayer(void);
    std::vector<ChessMove> getLegalMoves(void);
    ChessBoard getBoard();
	std::array<int,2> getKingPositions(void);
  

    //MCTS FUNCTIONS
    //Compute the legal moves from this game state
    virtual std::vector<ChessMove> computeLegalMoves(void);

    //Print an input for the network
    virtual std::vector<double> getFirstNetworkInput(void);
    virtual std::vector<double> getSecondNetworkInput(int);
    
    //Returns true if the state is final
    virtual bool isFinalState(void);
    
    //Returns the winner of the game if it is a final state, otherwise it returns 0
    virtual int getWinner(void);
    
    //Performs a simulation from this state, returning the winner of the game
    int simulateGame(void);


    //GAME FUNCTIONS
    //Function that returns true if a given cell is under attack from the a certain player, false otherwise
    static bool isUnderAttack(ChessBoard, int, int);

    //STATIC VARIABLES
    static std::unordered_map<std::string, int> MovesDictionary;
    

    //OUTPUT FUNCTIONS
    virtual void printState(void);


    //DESTRUCTOR
    virtual ~ChessState() = default;
};


void CreateChessMovesDictionary(void);

#endif
