#include <vector>
#include "Game.hpp"

GameState::GameState(std::array<int,64> board, int player) : board(board), player(player) { }

GameState::GameState(void) : GameState(STARTING_BOARD, 1) {}


//All the legal moves from this state have to be built
std::vector<GameState> getLegalMoves(void) {
  std::vector<GameState> moves;
  std::vector<std::vector<int>> takenPieces;
  
  //Cycle over all the black squares
  for(int index=0;index<32;i++) {
    //Get the starting black square
    int oldSquare = BLACK_SQUARES[index];
    
    //Check if the square is occupied by a draught of the current player
    if(this->board[oldSquare] == this->player) {
      //Then, check if there are available moves from this square
      //Along the first diagonal
      if(this->[board]
      
    }
    //Or by a king of the current player
    else if(this->board[square] == (2 * this->player)) {
    }
  }
  
}
