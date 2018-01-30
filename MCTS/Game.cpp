#include <array>
#include <vector>
#include <stdlib.h>
#include <algorithm>
#include <iostream>
#include "Game.hpp"

GameState::GameState(Board board, int player) : board(board), player(player), computedLegalMoves(false) { }

GameState::GameState(void) : GameState(STARTING_BOARD, 1) {}


//Checks if this is the final state
int GameState::isFinalState(void) {
  //If the player has no more legal moves available, the opponent won
  std::vector<GameState> legalMoves = this->getLegalMoves();
 
  if(legalMoves.size() == 0)
  {
    //std::cout << "This is a final state\n";
    return (-1 * player);
  }
  
  //std::cout << "This is not a final state!\n";
  //Otherwise, the game is not over yet
  return 0;
}


/*
//Total value of the board
double GameState::getBoardValue(void) {
  double total = 0;
  
  for (Board::iterator it = this->board.begin(); it != this->board.end(); ++it) {
    total += (double)(*it);
  }
  
  return (total/24.);
}
*/


Board GameState::getBoard(void) {
  return this->board;
}


int GameState::getPlayer(void) {
  return this->player;
}


//All the legal moves from this state have to be built
std::vector<GameState> GameState::getLegalMoves(void) {
  std::vector<GameState> possibleMoves;
  std::vector<int> movingPiece;
  std::vector<int> piecesTaken;
  std::vector<int> kingsTaken;
  int maxends;
  
  if(this->computedLegalMoves == true)
  {
    return this->legalMoves;
  }
  else
  {
    //std::cout << "We are building the moves.\n";
    //Cycle over all the black squares
    for(int index=0;index<32;index++) {
      //Get the starting black square
      int square = BLACK_SQUARES[index];

      //Check if the square is occupied by a draught of the current player
      if(this->board[square] == this->player) {
        //Then, get all the possible movements of this draught
        moveDraught(possibleMoves, movingPiece, piecesTaken, kingsTaken, this->board, this->player, square, 0, 0); 
      }
      //Or by a king of the current player
      else if(this->board[square] == (2 * this->player)) {
        //Then, get all the possible movements of this king
        moveKing(possibleMoves, movingPiece, piecesTaken, kingsTaken, this->board, this->player, square, 0, 0); 
      }
    }
    //std::cout << "We found " << possibleMoves.size() << " possible moves.\n";

    //Now, possibleMoves contains all the possible moves
    //takenPieces contain the respective number of taken pieces, and taken kings of taken kings
    //movingPiece tells if the moved piece is a draught or a king
    //First of all, let's check if there are any possible moves (and more than one in particular)
    if(possibleMoves.size() < 2)
    {
      std::vector<GameState> legalMoves = possibleMoves;
 
      this->legalMoves = legalMoves;
      this->computedLegalMoves = true;
      return this->legalMoves;
    }
    else
    {
      //If there are more possible moves, not all of them are in general legal
      //In particular, only the moves leading to the highest number of taken pieces are allowed
      //Let's build an array of indexes
      std::vector<int> indexes;
      for(int i=0;i<possibleMoves.size();++i) { 
        indexes.push_back(i); 
      }


      //Let's order it depending on the value of piecesTaken
      std::sort(indexes.begin(), indexes.end(), [&](int a, int b) { return piecesTaken[a] < piecesTaken[b]; });
      std::reverse(indexes.begin(),indexes.end());

      //std::cout << "These moves correspond to the following amount of taken pieces:\n";
      //for(int i=0;i<indexes.size();i++) { std::cout << piecesTaken[indexes[i]]; }
      //std::cout << "\n";

      //If the first element corresponds to zero taken pieces, then all the moves are equivalently legal
      if(piecesTaken[indexes[0]] == 0) {
        std::vector<GameState> legalMoves = possibleMoves;

        this->legalMoves = legalMoves;
        this->computedLegalMoves = true;
        return this->legalMoves;
      }

      //std::cout << "Then, we can eliminate the non legal moves.\n";

      //Otherwise, we have to limit ourselves to the ones with the maximum number of taken pieces
      maxends=0;
      while ((maxends < indexes.size()) && (piecesTaken[indexes[maxends]] == piecesTaken[indexes[0]]))
      {
        //std::cout << "Maxends = " << maxends << "\n";
        maxends++;
      }
      if(maxends != indexes.size()) {
	indexes.erase(indexes.begin()+maxends,indexes.end());
      }

      //std::cout << "And, after eliminating the non legal ones, we were left with moves taking the following amount of pieces:\n";
      //for(int i=0;i<indexes.size();i++) { std::cout << piecesTaken[indexes[i]]; }
      //std::cout << "\n";


      //Then, if possible, we have to eat with the king
      //So let's reorder the indexes based on the content of movingPiece
      std::sort(indexes.begin(), indexes.end(), [&](int a, int b) { return movingPiece[a] < movingPiece[b]; });
      std::reverse(indexes.begin(),indexes.end());

      //If the first element corresponds to a draught, then all the remaining moves are equivalently legal
      if(movingPiece[indexes[0]] == 1) {
        std::vector<GameState> legalMoves;
        for(int i=0;i<indexes.size();i++) {
          legalMoves.push_back(possibleMoves[indexes[i]]);
        }

        this->legalMoves = legalMoves;
        this->computedLegalMoves = true;
        return this->legalMoves;
      }

      //Otherwise, we have to limit ourselves to the ones with a moved king
      maxends=0;
      while ((maxends < indexes.size()) && (movingPiece[indexes[maxends]] == 2))
      {
        maxends++;
      }
      if(maxends != indexes.size()) {      
	indexes.erase(indexes.begin()+maxends,indexes.end());
      }


      //In the end, we have to eat the highest possible number of kings
      //So let's reorder the indexes based on the content of kingsTaken
      std::sort(indexes.begin(), indexes.end(), [&](int a, int b) { return kingsTaken[a] < kingsTaken[b]; });
      std::reverse(indexes.begin(),indexes.end());

      //If the first element corresponds to zero kings taken, then all the remaining moves are equivalently legal
      if(kingsTaken[indexes[0]] == 0) {
        std::vector<GameState> legalMoves;
        for(int i=0;i<indexes.size();i++) {
          legalMoves.push_back(possibleMoves[indexes[i]]);
        }

        this->legalMoves = legalMoves;
        this->computedLegalMoves = true;
        return this->legalMoves;
      }

      //Otherwise, we have to limit ourselves to the ones with the highest number of taken kings
      maxends=0;
      while ((maxends < indexes.size()) && (kingsTaken[indexes[maxends]] == kingsTaken[indexes[0]]))
      {
        maxends++;
      }
      if(maxends != indexes.size()) {      
	indexes.erase(indexes.begin()+maxends,indexes.end());
      }


      //In the end, we can return the remaining legal moves
      std::vector<GameState> legalMoves;
      for(int i=0;i<indexes.size();i++) {
        legalMoves.push_back(possibleMoves[indexes[i]]);
      }

      this->legalMoves = legalMoves;
      this->computedLegalMoves = true;
      return this->legalMoves;
    }
  }
}


//Simulates a random draughts game starting from the game state
double GameState::simulateGame(void) {
  //Get the starting state
  GameState currentState = *this;
  //and check if it is final
  int isFinal = currentState.isFinalState();

  //If it's not, perform a random simulation starting from the current state
  int Nmoves = 0;
  while((isFinal == 0) && (Nmoves < MAX_SIMULATION_LENGTH)) {
    //Get the possible legal moves from the current state
    std::vector<GameState> legalMoves = currentState.getLegalMoves();
    
    //Pick a random one
    currentState = legalMoves[std::rand()%legalMoves.size()];

    //And check if it's final
    isFinal = currentState.isFinalState();

    Nmoves++;
  }
  
  //Then, return isFinal;
  return ((double)isFinal);  
}


//Step of a draughts' movement
void moveDraught(std::vector<GameState> &possibleMoves, std::vector<int> &movingPiece, std::vector<int> &piecesTaken, std::vector<int> &kingsTaken, Board board, int player, int draught, int alreadyTakenPieces, int alreadyTakenKings) {
  //Along each diagonal
  for(int diag=0;diag<2;diag++) {
    //Look at the successive square
    int next = draught + player * DIAGONALS[diag];
    if(areNeighbourSquares(draught, next) == true) {
      //If it is free, it is possible to move there
      if((board[next] == 0) && (alreadyTakenPieces == 0)) {
        //And if it is the last row, it gets promoted to a king
        if(((next / 8 == 7) && (player == 1)) || ((next / 8 == 0) && (player == -1))) {
          Board newBoard = board;
          newBoard[draught] = 0;
          newBoard[next] = 2 * player;
        
          possibleMoves.push_back(GameState(newBoard, (-1 * player)));
          movingPiece.push_back(1);
          piecesTaken.push_back(alreadyTakenPieces);
          kingsTaken.push_back(alreadyTakenKings);
        }
	else
	{
          Board newBoard = board;
          newBoard[draught] = 0;
          newBoard[next] = player;
        
          possibleMoves.push_back(GameState(newBoard, (-1 * player)));
          movingPiece.push_back(1);
          piecesTaken.push_back(alreadyTakenPieces);
          kingsTaken.push_back(alreadyTakenKings);
	}
      }
      //Otherwise, if there is an opponents' draught, we can look at the following square
      else if(board[next] == (-1 * player))
      {
        int nextnext = next + player * DIAGONALS[diag];
        if(areNeighbourSquares(next, nextnext) == true) {
          //If it is free, we can catch the draught
          if(board[nextnext] == 0) {
	    //And if the ending position is the last row, the draught gets promoted to a king
	    if(((nextnext / 8 == 7) && (player == 1)) || ((nextnext / 8 == 0) && (player == -1))) {
              Board newBoard = board;
              newBoard[draught] = 0;
              newBoard[next] = 0;
              newBoard[nextnext] = 2 * player;
        
              possibleMoves.push_back(GameState(newBoard, (-1 * player)));
              movingPiece.push_back(1);
              piecesTaken.push_back((alreadyTakenPieces + 1));
              kingsTaken.push_back(alreadyTakenKings);
            }
	    else {
              Board newBoard = board;
              newBoard[draught] = 0;
              newBoard[next] = 0;
              newBoard[nextnext] = player;
        
              possibleMoves.push_back(GameState(newBoard, (-1 * player)));
              movingPiece.push_back(1);
              piecesTaken.push_back((alreadyTakenPieces + 1));
              kingsTaken.push_back(alreadyTakenKings);
            
              //But, after one take, it is eventually possible to take again
              moveDraught(possibleMoves, movingPiece, piecesTaken, kingsTaken, newBoard, player, nextnext, (alreadyTakenPieces + 1), alreadyTakenKings);
	    }
          }
        }
      }
    }
  }
}


//Step of a king movement
void moveKing(std::vector<GameState> &possibleMoves, std::vector<int> &movingPiece, std::vector<int> &piecesTaken, std::vector<int> &kingsTaken, Board board, int player, int king, int alreadyTakenPieces, int alreadyTakenKings) {
  //Along each diagonal
  for(int diag=0;diag<2;diag++) {
    //And along forward and backward direction
    for(int dir=-1;dir<=+1;dir+=2) {
      //Look at the forward next square along the diagonal
      int next = king + player * dir * DIAGONALS[diag];
      if(areNeighbourSquares(king, next) == true) {
        //If it is free, it is possible to move there
        if((board[next] == 0) && (alreadyTakenPieces == 0)) {
          Board newBoard = board;
          newBoard[king] = 0;
          newBoard[next] = 2 * player;
        
          possibleMoves.push_back(GameState(newBoard, (-1 * player)));
          movingPiece.push_back(2);
          piecesTaken.push_back(alreadyTakenPieces);
          kingsTaken.push_back(alreadyTakenKings);
        }
        //Otherwise, if there is an opponents' draught or king, we can look at the successive square
        else if(board[next] * player < 0)
        {
          int nextnext = next + player * dir * DIAGONALS[diag];
          if(areNeighbourSquares(next, nextnext) == true) {
            //If it is free, we can catch the draught/king
            if(board[nextnext] == 0) {
              Board newBoard = board;
              newBoard[king] = 0;
              newBoard[next] = 0;
              newBoard[nextnext] = 2 * player;
        
              if(board[next] == (-1 * player)) {
                possibleMoves.push_back(GameState(newBoard, (-1 * player)));
                movingPiece.push_back(2);
                piecesTaken.push_back((alreadyTakenPieces + 1));
                kingsTaken.push_back(alreadyTakenKings);
            
                //But, after one take, it is eventually possible to take again
                moveKing(possibleMoves, movingPiece, piecesTaken, kingsTaken, newBoard, player, nextnext, (alreadyTakenPieces + 1), alreadyTakenKings);
              }
              else {
                possibleMoves.push_back(GameState(newBoard, (-1 * player)));
                movingPiece.push_back(2);
                piecesTaken.push_back((alreadyTakenPieces + 1));
                kingsTaken.push_back((alreadyTakenKings + 1));
            
                //But, after one take, it is eventually possible to take again
                moveKing(possibleMoves, movingPiece, piecesTaken, kingsTaken, newBoard, player, nextnext, (alreadyTakenPieces + 1), (alreadyTakenKings + 1));
              }
            }
          }
        }
      }
    }
  }
}


bool areNeighbourSquares(int aSquare, int bSquare) {
  int aRow, aColumn, bRow, bColumn;

  if((aSquare < 0) || (aSquare > 63) || (bSquare < 0) || (bSquare > 63))
  {
    return false;
  }

  aRow = aSquare / 8;
  aColumn = aSquare % 8;
  bRow = bSquare / 8;
  bColumn = bSquare % 8;

  if((abs(aRow - bRow) == 1) && (abs(aColumn - bColumn) == 1)) {
    return true;
  }
  return false;
}

         
         
      
