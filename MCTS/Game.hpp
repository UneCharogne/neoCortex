#ifndef GAME_HPP
#define GAME_HPP

#include <vector>


const std::array<int,32> STARTING_BOARD = {1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};


//Class representing a state of the game
class GameState {
  private:
    //Array representing the content of the 32 black squares (+/-1 for a white/black draught, +/-2 for a white/black king)
    std::array<int,32> board; 
    
    //Player that has to move (+1 white, -1 black)
    int player;
    
    
    
    
   public:
    //CONSTRUCTORS
    GameState(std::array<int,32>, int);
    
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
