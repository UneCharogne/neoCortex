#ifndef GAME_HPP
#define GAME_HPP

#include <vector>


const std::array<int,64> STARTING_BOARD = {0,1,0,1,0,1,0,1,1,0,1,0,1,0,1,0,0,1,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,-1,0,-1,0,-1,0,-1,0,0,-1,0,-1,0,-1,0,-1,-1,0,-1,0,-1,0,-1,0};

const std::array<int,32> BLACK_SQUARES = {1,3,5,7,8,10,12,14,17,19,21,23,24,26,28,30,33,35,37,39,40,42,44,46,49,51,53,55,56,58,60,62};
const std::array<int,2> DIAGONALS = {7,9};

//Class representing a state of the game
class GameState {
  private:
    //Array representing the content of the 32 black squares (+/-1 for a white/black draught, +/-2 for a white/black king)
    std::array<int,64> board; 
    
    //Player that has to move (+1 white, -1 black)
    int player;
    
    
    
    
   public:
    //CONSTRUCTORS
    GameState(std::array<int,64>, int);
    
    //Standard constructor, gives the starting state
    GameState(void);
    
    
    //Overriden operators
    bool operator==(const GameState& otherState)const {
      if((this->board == otherState.board) && (this->player == otherState.player))
      {
        return true;
      }
      else
      {
        return false;
      }
    }
  
  
    //Gameplay
    //Returns 0 if the state is not final, otherwise it returns the winner of the game (+1 white, -1 black)
    int isFinalState(void);
    
    //Returns all the legal moves from this state
    std::vector<GameState> getLegalMoves(void);
    
    //Performs a simulation from this state, returning the reward of the simulation
    double simulateGame(void);
};


#endif
