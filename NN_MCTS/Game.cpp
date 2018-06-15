#include <array>
#include <vector>
#include <stdlib.h>
#include <algorithm>
#include <iostream>
#include "Game.hpp"


Move::Move(GameState *finalState, int id) : finalState(finalState), id(id) { }
Move::Move(GameState *finalState) : finalState(finalState) {
  this->id = 0;
}


//Returns the player
int GameState::getPlayer(void) {
    return this->player;
}

//Return the legal moves from this state
std::vector<Move> GameState::getLegalMoves(void) {
    //If the legal moves have not yet been calculated, calculate them
    if(this->computedLegalMoves == false)
    {
        this->legalMoves = computeLegalMoves();
        this->computedLegalMoves = true;
    }
    
    //Then, return them
    return this->legalMoves;
}


//Checks if this is the final state
bool GameState::isFinalState(void) {
    //If the player has no more legal moves available, the opponent won
    std::vector<Move> legalMoves = this->getLegalMoves();
    
    if(legalMoves.size() == 0)
    {
        //std::cout << "This is a final state\n";
        return true;
    }
    
    //std::cout << "This is not a final state!\n";
    //Otherwise, the game is not over yet
    return false;
}


//Checks if this is the final state
int GameState::getWinner(void) {
    //If this is a final state
    if(this->isFinalState()) {
        //The winner is the opposite of the current player
        return (-1 * this->player);
    }
    
    //Otherwise, the game is not over yet, and there are no winners
    return 0;
}


//Simulates a random draughts game starting from the game state
int GameState::simulateGame(void) {
    //Get the starting state
    GameState *currentState = this;
    //and check if it is final
    bool isFinal = currentState->isFinalState();
    
    //If it's not, perform a random simulation starting from this state
    std::vector<GameState*> simulatedGame;
    simulatedGame.push_back(currentState);
    int Nmoves = 0;
    while((!isFinal) && (Nmoves < MAX_SIMULATION_LENGTH)) {
        //Get the possible legal moves from the current state
        std::vector<Move> legalMoves = simulatedGame[simulatedGame.size()-1]->getLegalMoves();
        
        //Pick a random one
        simulatedGame.push_back(legalMoves[std::rand()%legalMoves.size()].finalState);
        
        //And check if it's final
        isFinal = simulatedGame[simulatedGame.size()-1]->isFinalState();
        //std::cout << "simulatedGame.size() = " << simulatedGame.size() << "\n";
        Nmoves++;
    }
    
    //Then, get the winner
    int winner = simulatedGame[simulatedGame.size()-1]->getWinner();
    
    //And clean the memory
    if(simulatedGame.size()>=1){
        //std::cout << "simulatedGame.size() = " << simulatedGame.size() << "\n";
        for(int i=simulatedGame.size()-1;i>=0;i--) {
            //std::cout << "i = " << i << "\n";
            //std::cout << "simulatedGame.[" << i << "]->legalMoves.size() = " << simulatedGame[i]->legalMoves.size() << "\n";
            for(int j=0;j<simulatedGame[i]->legalMoves.size();j++) {
                //std::cout << "j = " << j << "\n";
                delete simulatedGame[i]->legalMoves[j].finalState;
            }
            simulatedGame[i]->computedLegalMoves = false;
        }
    }
    
    return winner;
}



DraughtsMove::DraughtsMove(DraughtsState *finalState, int movingPiece, int piecesTaken, int kingsTaken, int id) : Move(finalState, id), movingPiece(movingPiece), piecesTaken(piecesTaken), kingsTaken(kingsTaken) { }
DraughtsMove::DraughtsMove(DraughtsState *finalState, int movingPiece, int piecesTaken, int kingsTaken) : Move(finalState), movingPiece(movingPiece), piecesTaken(piecesTaken), kingsTaken(kingsTaken) {
  this->id = 0;
}
DraughtsMove::DraughtsMove(DraughtsState *finalState, int id) : DraughtsMove(finalState, 0, 0, 0, id) {}
DraughtsMove::DraughtsMove(DraughtsState *finalState) : DraughtsMove(finalState, 0, 0, 0) {}
DraughtsMove::DraughtsMove() : DraughtsMove(new DraughtsState()) {}



DraughtsState::DraughtsState(DraughtsBoard board, int player) : board(board) {
    this->player = player;
    this->computedLegalMoves = false;
}
DraughtsState::DraughtsState(void) : DraughtsState(DRAUGHTS_STARTING_BOARD, 1) {}


DraughtsBoard DraughtsState::getBoard(void) {
  return this->board;
}


//All the legal moves from this state have to be built
std::vector<Move> DraughtsState::computeLegalMoves(void) {
    std::vector<DraughtsMove> possibleMoves;
    int maxends;
    
    //std::cout << "We are building the moves.\n";
    //Cycle over all the black squares
    for(int index=0;index<32;index++) {
        //Get the starting black square
        int square = DRAUGHTS_BOARD_BLACK_SQUARES[index];
            
        //Check if the square is occupied by a draught of the current player
        if(this->board[square] == this->player) {
            //Then, get all the possible movements of this draught
            moveDraught(possibleMoves, this->board, this->player, square, 0, 0);
        }
        //Or by a king of the current player
        else if(this->board[square] == (2 * this->player)) {
            //Then, get all the possible movements of this king
            moveKing(possibleMoves, this->board, this->player, square, 0, 0);
        }
    }
    //std::cout << "We found " << possibleMoves.size() << " possible moves.\n";
        
    //Now, possibleMoves contains all the possible moves
    //First of all, let's check if there are any possible moves (and more than one in particular)
    if(possibleMoves.size() < 2)
    {
        std::vector<Move> legalMoves;
        for(int i=0;i<possibleMoves.size();i++) {
            legalMoves.push_back(Move(possibleMoves[i].finalState));
        }
        
        return legalMoves;
    }
    else
    {
        //If there are more possible moves, not all of them are in general legal
        //In particular, only the moves leading to the highest number of taken pieces are allowed
         //Let's order the moves depending on the value of piecesTaken
        std::sort(possibleMoves.begin(), possibleMoves.end(), [&](const DraughtsMove& a, const DraughtsMove& b) { return a.piecesTaken < b.piecesTaken; });
        std::reverse(possibleMoves.begin(),possibleMoves.end());
        /*
        std::cout << "The possible moves capture ";
        for(int i=0;i<possibleMoves.size();i++) {
            std::cout << possibleMoves[i].piecesTaken;
        }
        std::cout << " pieces.\n";
        */
        
        //If the first element corresponds to zero taken pieces, then all the moves are equivalently legal
        if(possibleMoves[0].piecesTaken == 0) {
            std::vector<Move> legalMoves;
            for(int i=0;i<possibleMoves.size();i++) {
                legalMoves.push_back(Move(possibleMoves[i].finalState));
            }
            
            return legalMoves;
        }
            
        //std::cout << "Then, we can eliminate the non legal moves.\n";
            
        //Otherwise, we have to limit ourselves to the ones with the maximum number of taken pieces
        maxends=0;
        while ((maxends < possibleMoves.size()) && (possibleMoves[maxends].piecesTaken == possibleMoves[0].piecesTaken))
        {
            maxends++;
        }
        for(int i=maxends;i<possibleMoves.size();i++) {
            delete  possibleMoves[i].finalState;
        }
        if(maxends != possibleMoves.size()) {
            possibleMoves.erase(possibleMoves.begin()+maxends,possibleMoves.end());
        }
        /*
        std::cout << "While after the erasing they capture ";
        for(int i=0;i<possibleMoves.size();i++) {
            std::cout << possibleMoves[i].piecesTaken;
        }
        std::cout << " pieces.\n";
         */
            
            
        //Then, if possible, we have to eat with the king
        //So let's reorder the moves based on the content of movingPiece
        std::sort(possibleMoves.begin(), possibleMoves.end(), [&](const DraughtsMove& a, const DraughtsMove& b) { return a.movingPiece < b.movingPiece; });
        std::reverse(possibleMoves.begin(),possibleMoves.end());
            
        //If the first element corresponds to a draught, then all the remaining moves are equivalently legal
        if(possibleMoves[0].movingPiece == 1) {
            std::vector<Move> legalMoves;
            for(int i=0;i<possibleMoves.size();i++) {
                legalMoves.push_back(Move(possibleMoves[i].finalState));
            }
            
            return legalMoves;
        }
            
        //Otherwise, we have to limit ourselves to the ones with a moved king
        maxends=0;
        while ((maxends < possibleMoves.size()) && (possibleMoves[maxends].movingPiece == 2))
        {
            maxends++;
        }
        for(int i=maxends;i<possibleMoves.size();i++) {
            delete  possibleMoves[i].finalState;
        }
        if(maxends != possibleMoves.size()) {
            possibleMoves.erase(possibleMoves.begin()+maxends,possibleMoves.end());
        }
            
            
        //In the end, we have to eat the highest possible number of kings
        //So let's reorder the moves based on the content of kingsTaken
        std::sort(possibleMoves.begin(), possibleMoves.end(), [&](const DraughtsMove& a, const DraughtsMove& b) { return a.kingsTaken < b.kingsTaken; });
        std::reverse(possibleMoves.begin(),possibleMoves.end());
            
        //If the first element corresponds to zero kings taken, then all the remaining moves are equivalently legal
        if(possibleMoves[0].kingsTaken == 0) {
            std::vector<Move> legalMoves;
            for(int i=0;i<possibleMoves.size();i++) {
                legalMoves.push_back(Move(possibleMoves[i].finalState));
            }
            
            return legalMoves;
        }
            
        //Otherwise, we have to limit ourselves to the ones with the highest number of taken kings
        maxends=0;
        while ((maxends < possibleMoves.size()) && (possibleMoves[maxends].kingsTaken == possibleMoves[0].kingsTaken))
        {
            maxends++;
        }
        for(int i=maxends;i<possibleMoves.size();i++) {
            delete  possibleMoves[i].finalState;
        }
        if(maxends != possibleMoves.size()) {
            possibleMoves.erase(possibleMoves.begin()+maxends,possibleMoves.end());
        }
            
            
        //In the end, we can return the remaining legal moves
        std::vector<Move> legalMoves;
        for(int i=0;i<possibleMoves.size();i++) {
            legalMoves.push_back(Move(possibleMoves[i].finalState));
        }
        
        return legalMoves;
    }
}


//Step of a draughts' movement
void DraughtsState::moveDraught(std::vector<DraughtsMove> &possibleMoves, DraughtsBoard board, int player, int draught, int alreadyTakenPieces, int alreadyTakenKings) {
  //Along each diagonal
  for(int diag=0;diag<2;diag++) {
    //Look at the successive square
    int next = draught + player * DRAUGHTS_BOARD_DIAGONALS[diag];
    if(areNeighbourSquares(draught, next) == true) {
      //If it is free, it is possible to move there
      if((board[next] == 0) && (alreadyTakenPieces == 0)) {
        //And if it is the last row, it gets promoted to a king
        if(((next / 8 == 7) && (player == 1)) || ((next / 8 == 0) && (player == -1))) {
          DraughtsBoard newBoard = board;
          newBoard[draught] = 0;
          newBoard[next] = 2 * player;
        
          possibleMoves.push_back(DraughtsMove(new DraughtsState(newBoard, (-1 * player)), 1, alreadyTakenPieces, alreadyTakenKings));
        }
	else
	{
          DraughtsBoard newBoard = board;
          newBoard[draught] = 0;
          newBoard[next] = player;
        
          possibleMoves.push_back(DraughtsMove(new DraughtsState(newBoard, (-1 * player)), 1, alreadyTakenPieces, alreadyTakenKings));
	}
      }
      //Otherwise, if there is an opponents' draught, we can look at the following square
      else if(board[next] == (-1 * player))
      {
        int nextnext = next + player * DRAUGHTS_BOARD_DIAGONALS[diag];
        if(areNeighbourSquares(next, nextnext) == true) {
          //If it is free, we can catch the draught
          if(board[nextnext] == 0) {
            //And if the ending position is the last row, the draught gets promoted to a king
            if(((nextnext / 8 == 7) && (player == 1)) || ((nextnext / 8 == 0) && (player == -1))) {
              DraughtsBoard newBoard = board;
              newBoard[draught] = 0;
              newBoard[next] = 0;
              newBoard[nextnext] = 2 * player;
        
            
              possibleMoves.push_back(DraughtsMove(new DraughtsState(newBoard, (-1 * player)), 1, (alreadyTakenPieces + 1), alreadyTakenKings));
            }
            else {
              DraughtsBoard newBoard = board;
              newBoard[draught] = 0;
              newBoard[next] = 0;
              newBoard[nextnext] = player;
            
              possibleMoves.push_back(DraughtsMove(new DraughtsState(newBoard, (-1 * player)), 1, (alreadyTakenPieces + 1), alreadyTakenKings));
            
              //But, after one take, it is eventually possible to take again
              moveDraught(possibleMoves, newBoard, player, nextnext, (alreadyTakenPieces + 1), alreadyTakenKings);
            }
          }
        }
      }
    }
  }
}


//Step of a king movement
void DraughtsState::moveKing(std::vector<DraughtsMove> &possibleMoves, DraughtsBoard board, int player, int king, int alreadyTakenPieces, int alreadyTakenKings) {
  //Along each diagonal
  for(int diag=0;diag<2;diag++) {
    //And along forward and backward direction
    for(int dir=-1;dir<=+1;dir+=2) {
      //Look at the forward next square along the diagonal
      int next = king + player * dir * DRAUGHTS_BOARD_DIAGONALS[diag];
      if(areNeighbourSquares(king, next) == true) {
        //If it is free, it is possible to move there
        if((board[next] == 0) && (alreadyTakenPieces == 0)) {
          DraughtsBoard newBoard = board;
          newBoard[king] = 0;
          newBoard[next] = 2 * player;
        
          possibleMoves.push_back(DraughtsMove(new DraughtsState(newBoard, (-1 * player)), 2, alreadyTakenPieces, alreadyTakenKings));
        }
        //Otherwise, if there is an opponents' draught or king, we can look at the successive square
        else if(board[next] * player < 0)
        {
          int nextnext = next + player * dir * DRAUGHTS_BOARD_DIAGONALS[diag];
          if(areNeighbourSquares(next, nextnext) == true) {
            //If it is free, we can catch the draught/king
            if(board[nextnext] == 0) {
              DraughtsBoard newBoard = board;
              newBoard[king] = 0;
              newBoard[next] = 0;
              newBoard[nextnext] = 2 * player;
        
              if(board[next] == (-1 * player)) {
                possibleMoves.push_back(DraughtsMove(new DraughtsState(newBoard, (-1 * player)), 2, (alreadyTakenPieces + 1), alreadyTakenKings));
            
                //But, after one take, it is eventually possible to take again
                moveKing(possibleMoves, newBoard, player, nextnext, (alreadyTakenPieces + 1), alreadyTakenKings);
              }
              else {
                possibleMoves.push_back(DraughtsMove(new DraughtsState(newBoard, (-1 * player)), 2, (alreadyTakenPieces + 1), (alreadyTakenKings + 1)));
            
                //But, after one take, it is eventually possible to take again
                moveKing(possibleMoves, newBoard, player, nextnext, (alreadyTakenPieces + 1), (alreadyTakenKings + 1));
              }
            }
          }
        }
      }
    }
  }
}

                                    

bool DraughtsState::areNeighbourSquares(int aSquare, int bSquare) {
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



void DraughtsState::printState(void) {
    for(int j=0;j<17;j++) {
        std::cout << "_";
    }
    std::cout << "\n";
    for(int i=7;i>=0;i--) {
        std::cout << "|";
        for(int j=0;j<8;j++) {
            if(this->board[(8*i)+j] == 1)
                std::cout << "o";
            if(this->board[(8*i)+j] == 2)
                std::cout << "O";
            if(this->board[(8*i)+j] == -1)
                std::cout << "x";
            if(this->board[(8*i)+j] == -2)
                std::cout << "X";
            if(this->board[(8*i)+j] == 0)
                std::cout << " ";
            std::cout << "|";
        }
        std::cout << "\n";
    }
    std::cout << "\n\n";
}






ChessMove::ChessMove(ChessState *finalState, int id) : Move(finalState, id) {}
ChessMove::ChessMove(ChessState *finalState) : Move(finalState) {}
ChessMove::ChessMove() : ChessMove(new ChessState()) {}


ChessState::ChessState(ChessBoard board, std::array<int,2> kingPositions, int player) : kingPositions(kingPositions), board(board) {
    this->player = player;
    this->computedLegalMoves = false;
}
ChessState::ChessState(ChessBoard board, int player) : board(board) {
    this->player = player;
    this->computedLegalMoves = false;

    //Run over the chess to locate the white and black kings
    for(int square;square<64;square++) {
      if(this->board[square] == white_king)
      {
        kingPositions[0] = square;
      }
      if(this->board[square] == black_king)
      {
        kingPositions[1] = square;
      }
    }
}
ChessState::ChessState(void) : ChessState(CHESS_STARTING_BOARD, 1) {}


ChessBoard ChessState::getBoard(void) {
    return this->board;
}


std::array<int,2> ChessState::getKingPositions(void) {
    return this->kingPositions;
}


//All the legal moves from this state have to be built
std::vector<Move> ChessState::computeLegalMoves(void) {
    int i, j, n;
    int check;
    std::vector<Move> possibleMoves;

    //Run over all the cells
    for(int square=0;square<64;square++) {
      if(this->player == 1) {
        switch(this->board[square]) {
          case white_pawn:
            //A pawn can either move ahead of one step not eating
            if((square < 56) && (this->board[square + 8] == empty)) {
              ChessBoard newBoard = this->board;
              std::array<int,2> newKingPositions = this->kingPositions;
              newBoard[square] = empty;
              newBoard[square + 8] = white_pawn;

              //And if the move does not lead to check it a possible move
              if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
              {
                possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
              }

              //Eventually getting promoted if it is moving to the last row
              if((square / 8) == 7) {
                newBoard[square + 8] = white_knight;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }

                newBoard[square + 8] = white_bishop;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }

                newBoard[square + 8] = white_rook;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }

                newBoard[square + 8] = white_queen;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }
              }
            }

            //Or eat on the right and left
            if((square < 56) && ((square % 8) < 7) && (PIECES_COLORS[this->board[square + 8 + 1]] == -1)) {
              ChessBoard newBoard = this->board;
              std::array <int,2> newKingPositions = this->kingPositions;
              newBoard[square] = empty;
              newBoard[square + 8 + 1] = white_pawn;

              //And if the move does not lead to check it a possible move
              if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
              {
                possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
              }

              //Eventually getting promoted if it is moving to the last row
              if((square / 8) == 7) {
                newBoard[square + 8 + 1] = white_knight;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }

                newBoard[square + 8 + 1] = white_bishop;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }

                newBoard[square + 8 + 1] = white_rook;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }

                newBoard[square + 8 + 1] = white_queen;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }
              }
            }
            if((square < 56) && ((square % 8) > 0) && (PIECES_COLORS[this->board[square + 8 - 1]] == -1)) {
              ChessBoard newBoard = this->board;
              std::array<int,2> newKingPositions = this->kingPositions;
              newBoard[square] = empty;
              newBoard[square + 8 - 1] = white_pawn;

              //And if the move does not lead to check it a possible move
              if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
              {
                possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
              }

              //Eventually getting promoted if it is moving to the last row
              if((square / 8) == 7) {
                newBoard[square + 8 - 1] = white_knight;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }

                newBoard[square + 8 - 1] = white_bishop;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }

                newBoard[square + 8 - 1] = white_rook;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }

                newBoard[square + 8 - 1] = white_queen;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }
              }
            }

            //If it is in his starting cell it can move ahead of two cells too
            if(((square / 8) == 1) && (this->board[square + 8] == empty) && (this->board[square + 16] == empty)) {
              ChessBoard newBoard = this->board;
              std::array<int,2> newKingPositions = this->kingPositions;
              newBoard[square] = empty;
              newBoard[square + 16] = white_pawn;

              //And if the move does not lead to check it a possible move
              if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
              {
                possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
              }
            }
            break;

          case white_bishop:
            //A bishop can either move on the positive diagonal
            n = 1;
            check = 0;
            while(((square + (9 * n)) < 64) && (((square + (9 * n)) % 8) != 0) && (check == 0))
            {
              if(this->board[square + (9 * n)] == empty) {
                ChessBoard newBoard = this->board;
                std::array<int,2> newKingPositions = this->kingPositions;
                newBoard[square] = empty;
                newBoard[square + (9 * n)] = white_bishop;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + (9 * n)]] == -1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  newBoard[square] = empty;
                  newBoard[square + (9 * n)] = white_bishop;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                  }
                }
              }
              n++;
            }
            n = 1;
            check = 0;
            while(((square - (9 * n)) >= 0) && (((square - (9 * n)) % 8) != 7) && (check == 0))
            {
              if(this->board[square - (9 * n)] == empty) {
                ChessBoard newBoard = this->board;
                std::array<int,2> newKingPositions = this->kingPositions;
                newBoard[square] = empty;
                newBoard[square - (9 * n)] = white_bishop;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - (9 * n)]] == -1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  newBoard[square] = empty;
                  newBoard[square - (9 * n)] = white_bishop;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                  }
                }
              }
              n++;
            }

            //Or on the negative one
            n = 1;
            check = 0;
            while(((square + (7 * n)) < 64) && (((square + (7 * n)) % 8) != 7) && (check == 0))
            {
              if(this->board[square + (7 * n)] == empty) {
                ChessBoard newBoard = this->board;
                std::array<int,2> newKingPositions = this->kingPositions;
                newBoard[square] = empty;
                newBoard[square + (7 * n)] = white_bishop;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + (7 * n)]] == -1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  newBoard[square] = empty;
                  newBoard[square + (7 * n)] = white_bishop;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                  }
                }
              }
              n++;
            }
            n = 1;
            check = 0;
            while(((square - (7 * n)) >= 0) && (((square - (7 * n)) % 8) != 0) && (check == 0))
            {
              if(this->board[square - (7 * n)] == empty) {
                ChessBoard newBoard = this->board;
                std::array<int,2> newKingPositions = this->kingPositions;
                newBoard[square] = empty;
                newBoard[square - (7 * n)] = white_bishop;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - (7 * n)]] == -1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  newBoard[square] = empty;
                  newBoard[square - (7 * n)] = white_bishop;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                  }
                }
              }

              n++;
            }
            break;

          case white_rook:
            //The rook can move and eat along the vertical line
            i = 1;
            check = 0;
            while(((square + (8 * i)) < 64) && (check == 0))
            { 
              if(this->board[square + (8 * i)] == empty) {
                ChessBoard newBoard = this->board;
                std::array<int,2> newKingPositions = this->kingPositions;
                newBoard[square] = empty;
                newBoard[square + (8 * i)] = white_rook;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + (8 * i)]] == -1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  newBoard[square] = empty;
                  newBoard[square + (8 * i)] = white_rook;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                  }
                }
              }
              i++;
            }

            i = 1;
            check = 0;
            while(((square - (8 * i)) >= 0) && (check == 0))
            { 
              if(this->board[square - (8 * i)] == empty) {
                ChessBoard newBoard = this->board;
                std::array<int,2> newKingPositions = this->kingPositions;
                newBoard[square] = empty;
                newBoard[square - (8 * i)] = white_rook;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - (8 * i)]] == -1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  newBoard[square] = empty;
                  newBoard[square - (8 * i)] = white_rook;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                  }
                }
              }
              i++;
            }

            //And along the horizontal line
            j = 1;
            check = 0;
            while((((square + j) % 8) != 0) && (check == 0))
            {
              if(this->board[square + j] == empty) {
                ChessBoard newBoard = this->board;
                std::array<int,2> newKingPositions = this->kingPositions;
                newBoard[square] = empty;
                newBoard[square + j] = white_rook;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + j]] == -1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  newBoard[square] = empty;
                  newBoard[square + j] = white_rook;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                  }
                }
              }
              j++;
            }

            j = 1;
            check = 0;
            while((square - j >= 0) && (((square - j) % 8) != 7) && (check == 0))
            {
              if(this->board[square - j] == empty) {
                ChessBoard newBoard = this->board;
                std::array<int,2> newKingPositions = this->kingPositions;
                newBoard[square] = empty;
                newBoard[square - j] = white_rook;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - j]] == -1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  newBoard[square] = empty;
                  newBoard[square - j] = white_rook;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                  }
                }
              }
              j++;
            }
            break;

          case white_queen:
            //The queen can move in all the directions
            //The vertical one
            i = 1;
            check = 0;
            while(((square + (8 * i)) < 64) && (check == 0))
            {
              if(this->board[square + (8 * i)] == empty) {
                ChessBoard newBoard = this->board;
                std::array<int,2> newKingPositions = this->kingPositions;
                newBoard[square] = empty;
                newBoard[square + (8 * i)] = white_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + (8 * i)]] == -1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  newBoard[square] = empty;
                  newBoard[square + (8 * i)] = white_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                  }
                }
              }
              i++;
            }
            i = 1;
            check = 0;
            while(((square - (8 * i)) >= 0) && (check == 0))
            {
              if(this->board[square - (8 * i)] == empty) {
                ChessBoard newBoard = this->board;
                std::array<int,2> newKingPositions = this->kingPositions;
                newBoard[square] = empty;
                newBoard[square - (8 * i)] = white_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - (8 * i)]] == -1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  newBoard[square] = empty;
                  newBoard[square - (8 * i)] = white_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                  }
                }
              }
              i++;
            }

            //The horizontal one
            j = 1;
            check = 0;
            while((((square + j) % 8) != 0) && (check == 0))
            {
              if(this->board[square + j] == empty) {
                ChessBoard newBoard = this->board;
                std::array<int,2> newKingPositions = this->kingPositions;
                newBoard[square] = empty;
                newBoard[square + j] = white_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + j]] == -1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  newBoard[square] = empty;
                  newBoard[square + j] = white_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                  }
                }
              }
              j++;
            }
            j = 1;
            check = 0;
            while((square - j >= 0) && (((square - j) % 8) != 7) && (check == 0))
            {
              if(this->board[square - j] == empty) {
                ChessBoard newBoard = this->board;
                std::array<int,2> newKingPositions = this->kingPositions;
                newBoard[square] = empty;
                newBoard[square - j] = white_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - j]] == -1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  newBoard[square] = empty;
                  newBoard[square - j] = white_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                  }
                }
              }
              j++;
            }

            //The positive diagonal
            n = 1;
            check = 0;
            while(((square + (9 * n)) < 64) && (((square + (9 * n)) % 8) != 0) && (check == 0))
            {
              if(this->board[square + (9 * n)] == empty) {
                ChessBoard newBoard = this->board;
                std::array<int,2> newKingPositions = this->kingPositions;
                newBoard[square] = empty;
                newBoard[square + (9 * n)] = white_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + (9 * n)]] == -1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  newBoard[square] = empty;
                  newBoard[square + (9 * n)] = white_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                  }
                }
              }
              n++;
            }
            n = 1;
            check = 0;
            while(((square - (9 * n)) >= 0) && (((square - (9 * n)) % 8) != 7) && (check == 0))
            {
              if(this->board[square - (9 * n)] == empty) {
                ChessBoard newBoard = this->board;
                std::array<int,2> newKingPositions = this->kingPositions;
                newBoard[square] = empty;
                newBoard[square - (9 * n)] = white_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - (9 * n)]] == -1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  newBoard[square] = empty;
                  newBoard[square - (9 * n)] = white_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                  }
                }
              }
              n++;
            }

            //And the negative diagonal
            n = 1;
            check = 0;
            while(((square + (7 * n)) < 64) && (((square + (7 * n)) % 8) != 7) && (check == 0))
            {
              if(this->board[square + (7 * n)] == empty) {
                ChessBoard newBoard = this->board;
                std::array<int,2> newKingPositions = this->kingPositions;
                newBoard[square] = empty;
                newBoard[square + (7 * n)] = white_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + (7 * n)]] == -1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  newBoard[square] = empty;
                  newBoard[square + (7 * n)] = white_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                  }
                }
              }
              n++;
            }
            n = 1;
            check = 0;
            while(((square - (7 * n)) >= 0) && (((square - (7 * n)) % 8) != 0) && (check == 0))
            {
              if(this->board[square - (7 * n)] == empty) {
                ChessBoard newBoard = this->board;
                std::array<int,2> newKingPositions = this->kingPositions;
                newBoard[square] = empty;
                newBoard[square - (7 * n)] = white_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - (7 * n)]] == -1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  newBoard[square] = empty;
                  newBoard[square - (7 * n)] = white_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                  }
                }
              }

              n++;
            }
            break;

          case white_knight:
            //The knight can only jump on a specific set of squares       
            for(n=0;n<8;n++)
            {
              //If the jumping cell is a real cell
              i = (square / 8) + KNIGHT_JUMPS[n][0];
              j = (square % 8) + KNIGHT_JUMPS[n][1];
              if((i>=0) && (i<8) && (j>=0) && (j<8))
              {
                //And if it is empty or contains an enemy piece
                if(PIECES_COLORS[this->board[(8 * i) + j]] != 1) {
                  //It is possible to move there
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  newBoard[square] = empty;
                  newBoard[(8 * i) + j] = white_knight;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                  }
                }
              }
            }
            break;

          case white_king:
            //The king can move in all the directions, but of only one square
            for(i=-1;i<=1;i++) {
              for(j=-1;j<=1;j++) {
                //And if it exists
                if((i != 0) || (j != 0))
                {
                  //And if the cells exists
                  if((((square / 8) + i)>=0) && (((square / 8) + i)<8) && (((square % 8) + j)>=0) && (((square % 8) + j)<8))
                  {
                    //And if it is empty or contains an enemy piece
                    if(PIECES_COLORS[this->board[square + (8 * i) + j]] != 1) {
                      //The move is possible
                      ChessBoard newBoard = this->board;
                      std::array<int,2> newKingPositions = this->kingPositions;
                      newKingPositions[0] = square + (8 * i) + j;
                      newBoard[square] = empty;
                      newBoard[square + (8 * i) + j] = white_king;

                      //And if the move does not lead to check it a possible move
                      if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                      {
                        possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                      }
                    }
                  }
                }
              }
            }
            break;

          default:
            break;
        }
      }
      else
      {
        switch(this->board[square]) {
          case black_pawn:
            //A pawn can either move ahead of one step not eating
            if((square >= 8) && (this->board[square - 8] == empty)) {
              ChessBoard newBoard = this->board;
              std::array<int,2> newKingPositions = this->kingPositions;
              newBoard[square] = empty;
              newBoard[square - 8] = black_pawn;

              //And if the move does not lead to check it a possible move
              if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
              {
                possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
              }

              //Eventually getting promoted if it is moving to the last row
              if((square / 8) == 1) {
                newBoard[square - 8] = black_knight;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }

                newBoard[square - 8] = black_bishop;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }

                newBoard[square - 8] = black_rook;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }

                newBoard[square - 8] = black_queen;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }
              }
            }

            //Or eat on the right and left
            if((square >= 8) && ((square % 8) < 7) && (PIECES_COLORS[this->board[square - 8 + 1]] == 1)) {
              ChessBoard newBoard = this->board;
              std::array<int,2> newKingPositions = this->kingPositions;
              newBoard[square] = empty;
              newBoard[square - 8 + 1] = black_pawn;

              //And if the move does not lead to check it a possible move
              if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
              {
                possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
              }

              //Eventually getting promoted if it is moving to the last row
              if((square / 8) == 1) {
                newBoard[square - 8 + 1] = black_knight;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }

                newBoard[square - 8 + 1] = black_bishop;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }

                newBoard[square - 8 + 1] = black_rook;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }

                newBoard[square - 8 + 1] = black_queen;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }
              }
            }
            if((square >= 8) && ((square % 8) > 0) && (PIECES_COLORS[this->board[square - 8 - 1]] == 1)) {
              ChessBoard newBoard = this->board;
              std::array<int,2> newKingPositions = this->kingPositions;
              newBoard[square] = empty;
              newBoard[square - 8 - 1] = black_pawn;

              //And if the move does not lead to check it a possible move
              if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
              {
                possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
              }

              //Eventually getting promoted if it is moving to the last row
              if((square / 8) == 1) {
                newBoard[square - 8 - 1] = black_knight;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }

                newBoard[square - 8 - 1] = black_bishop;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }

                newBoard[square - 8 - 1] = black_rook;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }

                newBoard[square - 8 - 1] = black_queen;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }
              }
            }

            //If it is in his starting cell it can move ahead of two cells too
            if(((square / 8) == 6) && (this->board[square - 8] == empty) && (this->board[square - 16] == empty)) {
              ChessBoard newBoard = this->board;
              std::array<int,2> newKingPositions = this->kingPositions;
              newBoard[square] = empty;
              newBoard[square - 16] = black_pawn;

              //And if the move does not lead to check it a possible move
              if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
              {
                possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
              }
            }
            break;

          case black_bishop:
            //A bishop can either move on the positive diagonal
            n = 1;
            check = 0;
            while(((square + (9 * n)) < 64) && (((square + (9 * n)) % 8) != 0) && (check == 0))
            {
              if(this->board[square + (9 * n)] == empty) {
                ChessBoard newBoard = this->board;
                std::array<int,2> newKingPositions = this->kingPositions;
                newBoard[square] = empty;
                newBoard[square + (9 * n)] = black_bishop;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + (9 * n)]] == 1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  newBoard[square] = empty;
                  newBoard[square + (9 * n)] = black_bishop;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                  }
                }
              }
              n++;
            }
            n = 1;
            check = 0;
            while(((square - (9 * n)) >= 0) && (((square - (9 * n)) % 8) != 7) && (check == 0))
            {
              if(this->board[square - (9 * n)] == empty) {
                ChessBoard newBoard = this->board;
                std::array<int,2> newKingPositions = this->kingPositions;
                newBoard[square] = empty;
                newBoard[square - (9 * n)] = black_bishop;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - (9 * n)]] == 1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  newBoard[square] = empty;
                  newBoard[square - (9 * n)] = black_bishop;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                  }
                }
              }
              n++;
            }

            //Or on the negative one
            n = 1;
            check = 0;
            while(((square + (7 * n)) < 64) && (((square + (7 * n)) % 8) != 7) && (check == 0))
            {
              if(this->board[square + (7 * n)] == empty) {
                ChessBoard newBoard = this->board;
                std::array<int,2> newKingPositions = this->kingPositions;
                newBoard[square] = empty;
                newBoard[square + (7 * n)] = black_bishop;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + (7 * n)]] == 1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  newBoard[square] = empty;
                  newBoard[square + (7 * n)] = black_bishop;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                  }
                }
              }
              n++;
            }
            n = 1;
            check = 0;
            while(((square - (7 * n)) >= 0) && (((square - (7 * n)) % 8) != 0) && (check == 0))
            {
              if(this->board[square - (7 * n)] == empty) {
                ChessBoard newBoard = this->board;
                std::array<int,2> newKingPositions = this->kingPositions;
                newBoard[square] = empty;
                newBoard[square - (7 * n)] = black_bishop;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - (7 * n)]] == 1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  newBoard[square] = empty;
                  newBoard[square - (7 * n)] = black_bishop;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                  }
                }
              }

              n++;
            }
            break;

          case black_rook:
            //The rook can move and eat along the vertical line
            i = 1;
            check = 0;
            while(((square + (8 * i)) < 64) && (check == 0))
            {
              if(this->board[square + (8 * i)] == empty) {
                ChessBoard newBoard = this->board;
                std::array<int,2> newKingPositions = this->kingPositions;
                newBoard[square] = empty;
                newBoard[square + (8 * i)] = black_rook;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + (8 * i)]] == 1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  newBoard[square] = empty;
                  newBoard[square + (8 * i)] = black_rook;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                  }
                }
              }
              i++;
            }
            i = 1;
            check = 0;
            while(((square - (8 * i)) >= 0) && (check == 0))
            {
              if(this->board[square - (8 * i)] == empty) {
                ChessBoard newBoard = this->board;
                std::array<int,2> newKingPositions = this->kingPositions;
                newBoard[square] = empty;
                newBoard[square - (8 * i)] = black_rook;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - (8 * i)]] == 1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  newBoard[square] = empty;
                  newBoard[square - (8 * i)] = black_rook;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                  }
                }
              }
              i++;
            }

            //And along the horizontal line
            j = 1;
            check = 0;
            while((((square + j) % 8) != 0) && (check == 0))
            {
              if(this->board[square + j] == empty) {
                ChessBoard newBoard = this->board;
                std::array<int,2> newKingPositions = this->kingPositions;
                newBoard[square] = empty;
                newBoard[square + j] = black_rook;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + j]] == 1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  newBoard[square] = empty;
                  newBoard[square + j] = black_rook;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                  }
                }
              }
              j++;
            }
            j = 1;
            check = 0;
            while((square - j >= 0) && (((square - j) % 8) != 7) && (check == 0))
            {
              if(this->board[square - j] == empty) {
                ChessBoard newBoard = this->board;
                std::array<int,2> newKingPositions = this->kingPositions;
                newBoard[square] = empty;
                newBoard[square - j] = black_rook;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - j]] == 1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  newBoard[square] = empty;
                  newBoard[square - j] = black_rook;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                  }
                }
              }
              j++;
            }
            break;

          case black_queen:
            //The queen can move in all the directions
            //The vertical one
            i = 1;
            check = 0;
            while(((square + (8 * i)) < 64) && (check == 0))
            {
              if(this->board[square + (8 * i)] == empty) {
                ChessBoard newBoard = this->board;
                std::array<int,2> newKingPositions = this->kingPositions;
                newBoard[square] = empty;
                newBoard[square + (8 * i)] = black_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + (8 * i)]] == 1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  newBoard[square] = empty;
                  newBoard[square + (8 * i)] = black_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                  }
                }
              }
              i++;
            }
            i = 1;
            check = 0;
            while(((square - (8 * i)) >= 0) && (check == 0))
            {
              if(this->board[square - (8 * i)] == empty) {
                ChessBoard newBoard = this->board;
                std::array<int,2> newKingPositions = this->kingPositions;
                newBoard[square] = empty;
                newBoard[square - (8 * i)] = black_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - (8 * i)]] == 1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  newBoard[square] = empty;
                  newBoard[square - (8 * i)] = black_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                  }
                }
              }
              i++;
            }

            //The horizontal one
            j = 1;
            check = 0;
            while((((square + j) % 8) != 0) && (check == 0))
            {
              if(this->board[square + j] == empty) {
                ChessBoard newBoard = this->board;
                std::array<int,2> newKingPositions = this->kingPositions;
                newBoard[square] = empty;
                newBoard[square + j] = black_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + j]] == 1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  newBoard[square] = empty;
                  newBoard[square + j] = black_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                  }
                }
              }
              j++;
            }
            j = 1;
            check = 0;
            while((square - j >= 0) && (((square - j) % 8) != 7) && (check == 0))
            {
              if(this->board[square - j] == empty) {
                ChessBoard newBoard = this->board;
                std::array<int,2> newKingPositions = this->kingPositions;
                newBoard[square] = empty;
                newBoard[square - j] = black_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - j]] == 1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  newBoard[square] = empty;
                  newBoard[square - j] = black_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                  }
                }
              }
              j++;
            }

            //The positive diagonal
            n = 1;
            check = 0;
            while(((square + (9 * n)) < 64) && (((square + (9 * n)) % 8) != 0) && (check == 0))
            {
              if(this->board[square + (9 * n)] == empty) {
                ChessBoard newBoard = this->board;
                std::array<int,2> newKingPositions = this->kingPositions;
                newBoard[square] = empty;
                newBoard[square + (9 * n)] = black_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + (9 * n)]] == 1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  newBoard[square] = empty;
                  newBoard[square + (9 * n)] = black_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                  }
                }
              }
              n++;
            }
            n = 1;
            check = 0;
            while(((square - (9 * n)) >= 0) && (((square - (9 * n)) % 8) != 7) && (check == 0))
            {
              if(this->board[square - (9 * n)] == empty) {
                ChessBoard newBoard = this->board;
                std::array<int,2> newKingPositions = this->kingPositions;
                newBoard[square] = empty;
                newBoard[square - (9 * n)] = black_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - (9 * n)]] == 1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  newBoard[square] = empty;
                  newBoard[square - (9 * n)] = black_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                  }
                }
              }
              n++;
            }

            //And the negative one
            n = 1;
            check = 0;
            while(((square + (7 * n)) < 64) && (((square + (7 * n)) % 8) != 7) && (check == 0))
            {
              if(this->board[square + (7 * n)] == empty) {
                ChessBoard newBoard = this->board;
                std::array<int,2> newKingPositions = this->kingPositions;
                newBoard[square] = empty;
                newBoard[square + (7 * n)] = black_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + (7 * n)]] == 1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  newBoard[square] = empty;
                  newBoard[square + (7 * n)] = black_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                  }
                }
              }
              n++;
            }
            n = 1;
            check = 0;
            while(((square - (7 * n)) >= 0) && (((square - (7 * n)) % 8) != 0) && (check == 0))
            {
              if(this->board[square - (7 * n)] == empty) {
                ChessBoard newBoard = this->board;
                std::array<int,2> newKingPositions = this->kingPositions;
                newBoard[square] = empty;
                newBoard[square - (7 * n)] = black_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - (7 * n)]] == 1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  newBoard[square] = empty;
                  newBoard[square - (7 * n)] = black_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                  }
                }
              }

              n++;
            }
            break;

          case black_knight:
            //The knight can only jump on a specific set of squares       
            for(n=0;n<8;n++)
            {
              //If the jumping cell is a real cell
              i = (square / 8) + KNIGHT_JUMPS[n][0];
              j = (square % 8) + KNIGHT_JUMPS[n][1];
              if((i>=0) && (i<8) && (j>=0) && (j<8))
              {
                //And if it is empty or contains an enemy piece
                if(PIECES_COLORS[this->board[(8 * i) + j]] != -1) {
                  //It is possible to move there
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  newBoard[square] = empty;
                  newBoard[(8 * i) + j] = black_knight;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                  }
                }
              }
            }
            break;
            
          case black_king:
            //The king can move in all the directions, but of only one square
            for(i=-1;i<=1;i++) {
              for(j=-1;j<=1;j++) {
                //And if it exists
                if((i != 0) || (j != 0))
                {
                  //And if the cells exists
                  if((((square / 8) + i)>=0) && (((square / 8) + i)<8) && (((square % 8) + j)>=0) && (((square % 8) + j)<8))
                  {
                    //And if it is empty or contains an enemy piece
                    if(PIECES_COLORS[this->board[square + (8 * i) + j]] != -1) {
                      //The move is possible
                      ChessBoard newBoard = this->board;
                      std::array<int,2> newKingPositions = this->kingPositions;
                      newKingPositions[1] = square + (8 * i) + j;
                      newBoard[square] = empty;
                      newBoard[square + (8 * i) + j] = black_king;

                      //And if the move does not lead to check it a possible move
                      if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                      {
                        possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, (-1 * this->player))));
                      }
                    }
                  }
                }
              }
            }
            break;

          default:
            break;
        }
      }
    }

    return possibleMoves;
}


//Returns the winner given chess rules
int ChessState::getWinner(void) {
    //If this is a final state
    if(this->isFinalState()) {
        //If the king of the player is under check, he lost
        if(ChessState::isUnderAttack(this->board, this->kingPositions[((-1 * this->player) / 2)], -1 * this->player)) {
          return (-1 * this->player);
        }
        //Otherwise, it is a draw
        else {
          return 0;
        }
    }
    
    //Otherwise, the game is not over yet, and there are no winners
    return 0;
}


//Checks if this is the final state
bool ChessState::isFinalState(void) {
    //If the player has no more legal moves available, the game is over
    std::vector<Move> legalMoves = this->getLegalMoves();
    
    if(legalMoves.size() == 0)
    {
        return true;
    }
    
    //Otherwise, the game is not over yet
    return false;
}



void ChessState::printState(void) {
    for(int i=7;i>=0;i--) {
        for(int j=0;j<8;j++) {
            std::cout << "|" << PIECES_SYMBOLS[this->board[(8*i)+j]] << "|";
        }
        std::cout << "\n";
    }
    std::cout << "\n\n";
}



//Checks if a position on a given board is under attack from a certain player
bool ChessState::isUnderAttack(ChessBoard board, int square, int enemy) {
  //We can first of all move along the vertical direction until we find a piece or the board ends
  if((square / 8) < 7) {
    int i = 1;
    while((board[square + (8 * i)] == empty) && ((square + (8 * (i + 1))) < 64)) {
      i += 1;
    }

    //If an enemy piece is met
    if(PIECES_COLORS[board[square + (8 * i)]] == enemy) {
      //If it is a rook or a queen the square is attacked
      if((board[square + (8 * i)] == white_rook) || (board[square + (8 * i)] == white_queen) || (board[square + (8 * i)] == black_rook) || (board[square + (8 * i)] == black_queen)) {
        return true;
      }

      //Otherwise, if the piece is just in the next place, also being a king it would be attacking
      if(i == 1)
      {
        if((board[square + (8 * i)] == white_king) || (board[square + (8 * i)] == black_king)) {
          return true;
        }
      }
    }
  }
  if((square / 8) > 0) {
    int i = 1;
    while((board[square - (8 * i)] == empty) && ((square - (8 * (i + 1))) >= 0)) {
      i += 1;
    }
    //If an enemy piece is met
    if(PIECES_COLORS[board[square - (8 * i)]] == enemy) {
      //If it is a rook or a queen the square is attacked
      if((board[square - (8 * i)] == white_rook) || (board[square - (8 * i)] == white_queen) || (board[square - (8 * i)] == black_rook) || (board[square - (8 * i)] == black_queen)) {
        return true;
      }

      //Otherwise, if the piece is just in the next place, also being a king it would be attacking
      if(i == 1)
      {
        if((board[square - (8 * i)] == white_king) || (board[square - (8 * i)] == black_king)) {
          return true;
        }
      }
    }
  }


  //We can then move in the horizontal direction
  if((square % 8) < 7) {
    int j = 1;
    while((board[square + j] == empty) && (((square + (j + 1)) % 8) != 0)) {
      j += 1;
    }

    //If an enemy piece is met
    if(PIECES_COLORS[board[square + j]] == enemy) {
      //If it is a rook or a queen the square is attacked
      if((board[square + j] == white_rook) || (board[square + j] == white_queen) || (board[square + j] == black_rook) || (board[square + j] == black_queen)) {
        return true;
      }

      //Otherwise, if the piece is just in the next place, also being a king it would be attacking
      if(j == 1)
      {
        if((board[square + j] == white_king) || (board[square + j] == black_king)) {
          return true;
        }
      }
    }
  }
  if((square % 8) > 0) {
    int j = 1;
    while((board[square - j] == empty) && ((square - (j + 1)) >= 0) && (((square - (j + 1)) % 8) != 7)) {
      j += 1;
    }

    //If an enemy piece is met
    if(PIECES_COLORS[board[square - j]] == enemy) {
      //If it is a rook or a queen the square is attacked
      if((board[square - j] == white_rook) || (board[square - j] == white_queen) || (board[square - j] == black_rook) || (board[square - j] == black_queen)) {
        return true;
      }

      //Otherwise, if the piece is just in the next place, also being a king it would be attacking
      if(j == 1)
      {
        if((board[square - j] == white_king) || (board[square - j] == black_king)) {
          return true;
        }
      }
    }
  }


  //Then, we can move in the positive diagonal direction
  if(((square / 8) < 7) && ((square % 8) < 7)) {
    int n = 1;
    while((board[square + (9 * n)] == empty) && ((square + 9 * (n + 1)) < 64) && (((square + 9 * (n + 1)) % 8) != 0)) {
      n += 1;
    }

    //If an enemy piece is met
    if(PIECES_COLORS[board[square + (9 * n)]] == enemy) {
      //If it is a bishop or a queen the square is attacked
      if((board[square + (9 * n)] == white_bishop) || (board[square + (9 * n)] == white_queen) || (board[square + (9 * n)] == black_bishop) || (board[square + (9 * n)] == black_queen)) {
        return true;
      }

      //Otherwise, if the piece is just in the next place, also being a king or a black pawn it would be attacking
      if(n == 1)
      {
        if((board[square + (9 * n)] == white_king) || (board[square + (9 * n)] == black_king) || (board[square + (9 * n)] == black_pawn)) {
          return true;
        }
      }
    }
  }
  if(((square / 8) > 0) && ((square % 8) > 0)) {
    int n = 1;
    while((board[square - (9 * n)] == empty) && ((square - 9 * (n + 1)) >= 0) && (((square - 9 * (n + 1)) % 8) != 7)) {
      n += 1;
    }

    //If an enemy piece is met
    if(PIECES_COLORS[board[square - (9 * n)]] == enemy) {
      //If it is a bishop or a queen the square is attacked
      if((board[square - (9 * n)] == white_bishop) || (board[square - (9 * n)] == white_queen) || (board[square - (9 * n)] == black_bishop) || (board[square - (9 * n)] == black_queen)) {
        return true;
      }

      //Otherwise, if the piece is just in the next place, also being a king or a white pawn it would be attacking
      if(n == 1)
      {
        if((board[square - (9 * n)] == white_king) || (board[square - (9 * n)] == black_king) || (board[square - (9 * n)] == white_pawn)) {
          return true;
        }
      }
    }
  }


  //Then, we can move in the negative diagonal direction
  if(((square / 8) < 7) && ((square % 8) > 0)) {
    int n = 1;
    while((board[square + (7 * n)] == empty) && ((square + 7 * (n + 1)) < 64) && (((square + 7 * (n + 1)) % 8) != 7)) {
      n += 1;
    }

    //If an enemy piece is met
    if(PIECES_COLORS[board[square + (7 * n)]] == enemy) {
      //If it is a bishop or a queen the square is attacked
      if((board[square + (7 * n)] == white_bishop) || (board[square + (7 * n)] == white_queen) || (board[square + (7 * n)] == black_bishop) || (board[square + (7 * n)] == black_queen)) {
        return true;
      }

      //Otherwise, if the piece is just in the next place, also being a king or a black pawn it would be attacking
      if(n == 1)
      {
        if((board[square + (7 * n)] == white_king) || (board[square + (7 * n)] == black_king) || (board[square + (7 * n)] == black_pawn)) {
          return true;
        }
      }
    }
  }
  if(((square / 8) > 0) && ((square % 8) < 7)) {
    int n = 1;
    while((board[square - (7 * n)] == empty) && ((square - 7 * (n + 1)) >= 0) && (((square - 7 * (n + 1)) % 8) != 0)) {
      n += 1;
    }

    //If an enemy piece is met
    if(PIECES_COLORS[board[square - (7 * n)]] == enemy) {
      //If it is a bishop or a queen the square is attacked
      if((board[square - (7 * n)] == white_bishop) || (board[square - (7 * n)] == white_queen) || (board[square - (7 * n)] == black_bishop) || (board[square - (7 * n)] == black_queen)) {
        return true;
      }

      //Otherwise, if the piece is just in the next place, also being a king or a white pawn it would be attacking
      if(n == 1)
      {
        if((board[square - (7 * n)] == white_king) || (board[square - (7 * n)] == black_king) || (board[square - (7 * n)] == white_pawn)) {
          return true;
        }
      }
    }
  }


  //In the end, we just have to consider the possible attacks from horses
  //Given a square, there are maximum 8 square from which a knight can attack it
  //We can check all of them
  for(int n=0;n<8;n++)
  {
    //If the jumping cell is a real cell
    int i = (square / 8) + KNIGHT_JUMPS[n][0];
    int j = (square % 8) + KNIGHT_JUMPS[n][1];
    if((i>=0) && (i<8) && (j>=0) && (j<8))
    {
      //And if it contains an enemy knight
      if((PIECES_COLORS[board[(8 * i) + j]] == enemy) && ((board[(8 * i) + j] == white_knight) || (board[(8 * i) + j] == black_knight))) {
        //The square is attacked
        return true;
      }
    }
  }

  return false;
}






TicTacToeMove::TicTacToeMove(TicTacToeState *finalState, int id) : Move(finalState, id) {}
TicTacToeMove::TicTacToeMove(TicTacToeState *finalState) : Move(finalState) {}
TicTacToeMove::TicTacToeMove() : TicTacToeMove(new TicTacToeState()) {}


TicTacToeState::TicTacToeState(TicTacToeBoard board, int player) : board(board) {
    this->player = player;
    this->computedLegalMoves = false;
}
TicTacToeState::TicTacToeState(void) : TicTacToeState(TICTACTOE_STARTING_BOARD, 1) {}

TicTacToeBoard TicTacToeState::getBoard(void) {
    return this->board;
}


//All the legal moves from this state have to be built
std::vector<Move> TicTacToeState::computeLegalMoves(void) {
    std::vector<Move> possibleMoves;
    
    //Cycle over all the board squares
    for(int index=0;index<9;index++) {
        //If the square is empty, it corresponds to a possible move
        if(this->board[index] == 0)
        {
            TicTacToeBoard newBoard = this->board;
            newBoard[index] = this->player;
            possibleMoves.push_back(TicTacToeMove(new TicTacToeState(newBoard, (-1 * this->player)), index));
        }
    }
    
    std::vector<Move> legalMoves = possibleMoves;
    return legalMoves;
}


std::vector<double> TicTacToeState::getNetworkInput(void) {
  std::vector<double> netInput;

  for(int sign=1;sign>=-1;sign-=2) {
    for(int i=0;i<9;i++) {
      if(this->board[i] == (sign * this->player)) {
        netInput.push_back(1);
      }
      else {
        netInput.push_back(0);
      }
    }
  }

  return netInput;
}


//Returns the winner given tictactoe rules
int TicTacToeState::getWinner(void) {
    //We have to check every line and diagonal
    for(int i=0;i<8;i++) {
        std::array<int,3> line = TICTACTOE_WINNING_LINES[i];
        
        //If this is a winning line, return the winner
        if((this->board[line[0]] == this->board[line[1]]) && (this->board[line[1]] == this->board[line[2]])) {
            return this->board[line[0]];
        }
    }
    
    //If no line is a winning line the game is not over yet, and there are no winners
    return 0;
}


//Checks if this is the final state
bool TicTacToeState::isFinalState(void) {
    //Test the tic tac toe winning condition
    int winner = this->getWinner();
    
    //If there is a winner, the game is over
    if(winner != 0)
    {
        return true;
    }
    
    //Otherwise, the game is over if there are no other legal moves
    std::vector<Move> legalMoves = this->getLegalMoves();
    if(legalMoves.size() == 0)
    {
        return true;
    }
    
    //Otherwise, the game is not over yet
    return false;
}



void TicTacToeState::printState(void) {
    for(int i=0;i<3;i++) {
        for(int j=0;j<3;j++) {
            std::cout << "|";
            if(this->board[(3*i)+j] == 1)
                std::cout << "o";
            if(this->board[(3*i)+j] == -1)
                std::cout << "x";
            if(this->board[(3*i)+j] == 0)
                std::cout << " ";
            std::cout << "|";
        }
        std::cout << "\n";
    }
    std::cout << "\n\n";
}

    
                                        
                                        
                                

         
         
      
