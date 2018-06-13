/*
    Game.hpp:
        Library for the definition of the GameState class.
        The GameState contains the player and a vector of legalMoves (calculated via a method of the class itself).
        It contains a method to determine if the state is a final state of the game, and eventually who is the winner.
        Also, the method simulateGame performs a random game simulation starting from the current state, and the returns the reward.
        It contains a routine to graphically print the state in the console and a destructor.

        It also contains specific GameState class for the draughts, the chess and the tictactoe, derived from the same GameState parent class.

        @author: Massimiliano Chiappini 
        @contact: massimilianochiappini@gmail.com
        @version: 0.2
*/


#ifndef GAME_HPP
#define GAME_HPP

#include <vector>
#include <string>
#include <array>


typedef std::array<int,64> DraughtsBoard;
const DraughtsBoard DRAUGHTS_STARTING_BOARD = {0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-1,0,-1,0,-1,0,-1,0,0,-1,0,-1,0,-1,0,-1,-1,0,-1,0,-1,0,-1,0};
const std::array<int,32> DRAUGHTS_BOARD_BLACK_SQUARES = {1,3,5,7,8,10,12,14,17,19,21,23,24,26,28,30,33,35,37,39,40,42,44,46,49,51,53,55,56,58,60,62};
const std::array<int,2> DRAUGHTS_BOARD_DIAGONALS = {7,9};


//The board of chess is a 64 board, in which each piece is represented by a number:
enum piece {
    empty,
    white_pawn,
    white_rook,
    white_knight,
    white_bishop,
    white_queen,
    white_king,
    black_pawn,
    black_rook,
    black_knight,
    black_bishop,
    black_queen,
    black_king
};
//And a symbol
const std::array<std::string,13> PIECES_SYMBOLS = {" ", "\u2659", "\u2656", "\u2658", "\u2657", "\u2655", "\u2654", "\u265F", "\u265C", "\u265E", "\u265D", "\u265B", "\u265A"};
//and a color, either white (+1) or black (-1)
const std::array<int,13> PIECES_COLORS = {0, 1, 1, 1, 1, 1, 1, -1, -1, -1, -1, -1, -1};
//Also, the pieces of the chessboard have a code
const std::array<std::string,64> CHESSBOARD_SQUARE_NAMES = {"a1","a2","a3","a4","a5","a6","a7","a8","b1","b2","b3","b4","b5","b6","b7","b8","c1","c2","c3","c4","c5","c6","c7","c8","d1","d2","d3","d4","d5","d6","d7","d8","e1","e2","e3","e4","e5","e6","e7","e8","f1","f2","f3","f4","f5","f6","f7","f8","g1","g2","g3","g4","g5","g6","g7","g8","h1","h2","h3","h4","h5","h6","h7","h8"};
//Each position of the chessboard can be described either by an integer scalar n or by an integer vector (i, j)
//In particular n = 8 * i + j
//And:
//i = n / 8
//j = n % 8
typedef std::array<piece,64> ChessBoard;
const ChessBoard CHESS_STARTING_BOARD = {white_rook, white_knight, white_bishop, white_queen, white_king, white_bishop, white_knight, white_rook, white_pawn, white_pawn, white_pawn, white_pawn, white_pawn, white_pawn, white_pawn, white_pawn, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, empty, black_pawn, black_pawn, black_pawn, black_pawn, black_pawn, black_pawn, black_pawn, black_pawn, black_rook, black_knight, black_bishop, black_queen, black_king, black_bishop, black_knight, black_rook};
//It is also useful to save for later the possible knight jumps
const std::array<std::array<int,2>,8> KNIGHT_JUMPS = {{{-2,-1},{-2,1},{-1,-2},{-1,2},{1,-2},{1,2},{2,-1},{2,1}}};


typedef std::array<int,9> TicTacToeBoard;
const TicTacToeBoard TICTACTOE_STARTING_BOARD = {0,0,0,0,0,0,0,0,0};
const std::array<std::array<int,3>,8> TICTACTOE_WINNING_LINES = {{{0,1,2},{3,4,5},{6,7,8},{0,3,6},{1,4,7},{2,5,8},{0,4,8},{2,4,6}}};


#define MAX_SIMULATION_LENGTH 1000

class GameState;

//Class representing a move
struct Move {
    //Properties
    GameState *finalState;
    
    //Constructors
    Move(GameState*);
};


//Class representing a state of the game
class GameState {
  protected:
    //Current player
    int player;
    
    //Legal moves from this state
    std::vector<Move> legalMoves;

    //Have the legal moves been computed yet?
    bool computedLegalMoves;
    
    
    
    
   public:
    //OVERLOADED OPERATORS
    bool operator==(const GameState& otherState) {
        if (typeid((*this)) != typeid(otherState)) {
            return false;
        }
        else {
            return isEqual(otherState);
        }
    }
    
    virtual bool isEqual(const GameState&) = 0;
  
    
    //SET/GET methods
    int getPlayer(void);
    std::vector<Move> getLegalMoves(void);
  
    //GAMEPLAY
    //Compute the legal moves from this game state
    virtual std::vector<Move> computeLegalMoves(void) = 0;
    
    //Returns true if the state is final
    virtual bool isFinalState(void);
    
    //Returns the winner of the game if it is a final state, otherwise it returns 0
    virtual int getWinner(void);
    
    //Performs a simulation from this state, returning the winner of the game
    int simulateGame(void);
    
    //Output
    virtual void printState(void) = 0;

    //DESTRUCTOR
    virtual ~GameState() = default;
};




class DraughtsState;

struct DraughtsMove : public Move {
    int movingPiece;
    int piecesTaken;
    int kingsTaken;
    
    //CONSTRUCTORS
    DraughtsMove();
    DraughtsMove(DraughtsState*);
    DraughtsMove(DraughtsState*, int, int, int);
};


class DraughtsState : public GameState {
    private:
        DraughtsBoard board;
    
    
    
    
    public:
        //CONSTRUCTORS
        DraughtsState(DraughtsBoard, int);
        DraughtsState(void);
    
    
        //OVERRIDEN OPERATORS
        bool isEqual(const GameState& otherState) {
            const DraughtsState& draughtsOtherState = static_cast<const DraughtsState&>(otherState);
            if((this->board == draughtsOtherState.board) && (this->player == draughtsOtherState.player)) {
                return true;
            }
            else {
                return false;
            }
        }
    
    
        //SET/GET methods
        DraughtsBoard getBoard(void);
    
    
        //OVERRIDEN METHODS
        //Compute the legal moves from this game state
        std::vector<Move> computeLegalMoves(void);
        void printState(void);
    
        //STATIC FUNCTIONS
        //Function that performs a single step of a move of a draught
        static void moveDraught(std::vector<DraughtsMove>&, DraughtsBoard, int, int, int, int);
    
        //Function that performs a single step of a move of a king
        static void moveKing(std::vector<DraughtsMove>&, DraughtsBoard, int, int, int, int);
    
        //Function that checks if two squares are neighbours
        static bool areNeighbourSquares(int, int);
};




class ChessState;

struct ChessMove : public Move {
    //CONSTRUCTORS
    ChessMove();
    ChessMove(ChessState*);
};


class ChessState : public GameState {
    private:
        ChessBoard board;
        std::array<int,2> kingPositions;


    public:
        //CONSTRUCTORS
        ChessState(ChessBoard, std::array<int,2>, int);
        ChessState(ChessBoard, int);
        ChessState(void);
        
        
        //OVERRIDEN OPERATORS
        bool isEqual(const GameState& otherState) {
            const ChessState& chessOtherState = static_cast<const ChessState&>(otherState);
            if((this->board == chessOtherState.board) && (this->player == chessOtherState.player)) {
                return true;
            }
            else {
                return false;
            }
        }
        
        
        //SET/GET methods
        ChessBoard getBoard(void);
        std::array<int,2> getKingPositions(void);
        
        
        //OVERRIDEN METHODS
        //Compute the legal moves from this game state
        std::vector<Move> computeLegalMoves(void);
        
        //Winning condition for tic tac toe
        int getWinner(void);
        
        //Tells if the state is a final state
        bool isFinalState(void);

        //Function that prints the state of the system
        void printState(void);
    
        //GAME FUNCTIONS
        //Function that returns true if a given cell is under attack from the a certain player, false otherwise
        static bool isUnderAttack(ChessBoard, int, int);
};




class TicTacToeState;

struct TicTacToeMove : public Move {
    //CONSTRUCTORS
    TicTacToeMove();
    TicTacToeMove(TicTacToeState*);
};


class TicTacToeState : public GameState {
    private:
        TicTacToeBoard board;
        
        
        
        
    public:
        //CONSTRUCTORS
        TicTacToeState(TicTacToeBoard, int);
        TicTacToeState(void);
        
        
        //OVERRIDEN OPERATORS
        bool isEqual(const GameState& otherState) {
            const TicTacToeState& ticTacToeOtherState = static_cast<const TicTacToeState&>(otherState);
            if((this->board == ticTacToeOtherState.board) && (this->player == ticTacToeOtherState.player)) {
                return true;
            }
            else {
                return false;
            }
        }
        
        
        //SET/GET methods
        TicTacToeBoard getBoard(void);
        
        
        //OVERRIDEN METHODS
        //Compute the legal moves from this game state
        std::vector<Move> computeLegalMoves(void);
        
        //Winning condition for tic tac toe
        int getWinner(void);
        
        //Tells if the state is a final state
        bool isFinalState(void);
        
        //Print the current board
        void printState(void);
};

#endif
