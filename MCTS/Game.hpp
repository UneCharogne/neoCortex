#ifndef GAME_HPP
#define GAME_HPP

#include <vector>
#include <array>


typedef std::array<int,64> DraughtsBoard;
typedef std::array<int,9> TicTacToeBoard;


const DraughtsBoard DRAUGHTS_STARTING_BOARD = {0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-1,0,-1,0,-1,0,-1,0,0,-1,0,-1,0,-1,0,-1,-1,0,-1,0,-1,0,-1,0};

const std::array<int,32> DRAUGHTS_BOARD_BLACK_SQUARES = {1,3,5,7,8,10,12,14,17,19,21,23,24,26,28,30,33,35,37,39,40,42,44,46,49,51,53,55,56,58,60,62};
const std::array<int,2> DRAUGHTS_BOARD_DIAGONALS = {7,9};


const TicTacToeBoard TICTACTOE_STARTING_BOARD = {0,0,0,0,0,0,0,0,0};

const std::array<std::array<int,3>,8> TICTACTOE_WINNING_LINES = {{{0,1,2},{3,4,5},{6,7,8},{0,3,6},{1,4,7},{2,5,8},{0,4,8},{2,4,6}}};


#define MAX_SIMULATION_LENGTH 120

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
