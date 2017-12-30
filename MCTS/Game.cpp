#include <vector>
#include "Game.hpp"

GameState::GameState(Board board, int player) : board(board), player(player), computedLegalMoves(false) { }

GameState::GameState(void) : GameState(STARTING_BOARD, 1) {}


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
    //Cycle over all the black squares
    for(int index=0;index<32;i++) {
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

      //If the first element corresponds to zero taken pieces, then all the moves are equivalently legal
      if(piacesTaken[indexes[0]] == 0) {
        std::vector<GameState> legalMoves = possibleMoves;

        this->legalMoves = legalMoves;
        this->computedLegalMoves = true;
        return this->legalMoves;
      }

      //Otherwise, we have to limit ourselves to the ones with the maximum number of taken pieces
      maxends=0;
      do { maxends++; } while (piecesTaken[maxends] == piecesTaken[0]);
      indexes.erase(indexes.begin()+maxends,indexes.end());


      //Then, if possible, we have to eat with the king
      //So let's reorder the indexes based on the content of movingPiece
      std::sort(indexes.begin(), indexes.end(), [&](int a, int b) { return movingPiece[a] < movingPiece[b]; });
      std::reverse(indexes.begin(),indexes.end());

      //If the first element corresponds to a draught, then all the remaining moves are equivalently legal
      if(movingPiece[indexes[0]] == 1) {
        std::vector<GameState> legalMoves;
        for(int i=0;i<indexes.size();int++) {
          legalMoves.push_back(possibleMoves[indexes[i]]);
        }

        this->legalMoves = legalMoves;
        this->computedLegalMoves = true;
        return this->legalMoves;
      }

      //Otherwise, we have to limit ourselves to the ones with a moved king
      maxends=0;
      do { maxends++; } while (movingPiece[maxends] == 2);
      indexes.erase(indexes.begin()+maxends,indexes.end());


      //In the end, we have to eat the highest possible number of kings
      //So let's reorder the indexes based on the content of kingsTaken
      std::sort(indexes.begin(), indexes.end(), [&](int a, int b) { return kingsTaken[a] < kingsTaken[b]; });
      std::reverse(indexes.begin(),indexes.end());

      //If the first element corresponds to zero kings taken, then all the remaining moves are equivalently legal
      if(kingsTaken[indexes[0]] == 0) {
        std::vector<GameState> legalMoves;
        for(int i=0;i<indexes.size();int++) {
          legalMoves.push_back(possibleMoves[indexes[i]]);
        }

        this->legalMoves = legalMoves;
        this->computedLegalMoves = true;
        return this->legalMoves;
      }

      //Otherwise, we have to limit ourselves to the ones with the highest number of taken kings
      maxends=0;
      do { maxends++; } while (kingsTaken[maxends] == kingsTaken[0]);
      indexes.erase(indexes.begin()+maxends,indexes.end());


      //In the end, we can return the remaining legal moves
      std::vector<GameState> legalMoves;
      for(int i=0;i<indexes.size();int++) {
        legalMoves.push_back(possibleMoves[indexes[i]]);
      }

      this->legalMoves = legalMoves;
      this->computedLegalMoves = true;
      return this->legalMoves;
    }
  }
}


int GameState::isFinalState(void) {
  //If the player has no more legal moves available, the opponent won
  if(this->getLegalMoves().size() == 0)
  {
    return (-1 * player);
  }
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
        Board newBoard = board;
        newBoard[draught] = 0;
        newBoard[next] = player;
        
        possibleMoves.push_back(GameState(newBoard, (-1 * player)));
        movingPiece.push_back(1);
        piecesTaken.push_back(alreadyTakenPieces);
        kingsTaken.push_back(alreadyTakenKings);
      }
      //Otherwise, if there is an opponents' draught, we can look at the following square
      else if(board[next] == (-1 * player))
      {
        int nextnext = next + player * DIAGONALS[diag];
        if(areNeighbourSquares(next, nextnext) == true) {
          //If it is free, we can catch the draught
          if(board[nextnext] == 0) {
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


//Step of a king movement
void moveKing(std::vector<GameState> &possibleMoves, std::vector<int> &movingPiece, std::vector<int> &piecesTaken, std::vector<int> &kingsTaken, Board board, int player, int king, int alreadyTakenPieces, int alreadyTakenKings) {
  //Along each diagonal
  for(int diag=0;diag<2;diag++) {
    //And along forward and backward direction
    for(int dir=-1;dir<=+1;dir+=2) {
      //Look at the forward next square along the diagonal
      int next = king + player * dir * DIAGONALS[diag];
      if(areNeighbourSquares(draught, next) == true) {
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
  
  aRow = aSquare / 8;
  aColumn = aSquare % 8;
  bRow = bSquare / 8;
  bColumn = bSquare % 8;
  
  if((abs(aRow - bRow) == 1) && (abs(aColumn - bColumn) == 1)) {
    return true;
  }
  return false;
}

         
         
      
