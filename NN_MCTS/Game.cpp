#include <array>
#include <vector>
#include <stdlib.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <unordered_map>
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


ChessState::ChessState(ChessBoard board, std::array<int,2> kingPositions, std::array<std::array<int,2>,2> possibleCastling, int player) : kingPositions(kingPositions), possibleCastling(possibleCastling), board(board) {
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

    //If nothing is stated we assume that castling is possible if the king and the rooks are in their starting place
    if(kingPositions[0] == 4) {
      if(this->board[0] == white_rook) {
        possibleCastling[0][0] = 1;
      }
      else {
        possibleCastling[0][0] = 0;
      }
      if(this->board[7] == white_rook) {
        possibleCastling[0][1] = 1;
      }
      else {
        possibleCastling[0][1] = 0;
      }
    } 
    else {
      possibleCastling[0][0] = 0;
      possibleCastling[0][1] = 0;
    }

    if(kingPositions[1] == 60) {
      if(this->board[56] == black_rook) {
        possibleCastling[1][0] = 1;
      }
      else {
        possibleCastling[1][0] = 0;
      }
      if(this->board[63] == white_rook) {
        possibleCastling[1][1] = 1;
      }
      else {
        possibleCastling[1][1] = 0;
      }
    } 
    else {
      possibleCastling[1][0] = 0;
      possibleCastling[1][1] = 0;
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
    std::string moveName;
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
              std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
              newBoard[square] = empty;
              newBoard[square + 8] = white_pawn;

              //And if the move does not lead to check it a possible move
              if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
              {
              	moveName = "P" + std::to_string(square) + "P" + std::to_string(square + 8);
                possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
              }

              //Eventually getting promoted if it is moving to the last row
              if((square / 8) == 6) {
                newBoard[square + 8] = white_knight;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(square) + "N" + std::to_string(square + 8);
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }

                newBoard[square + 8] = white_bishop;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(square) + "B" + std::to_string(square + 8);
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }

                newBoard[square + 8] = white_rook;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(square) + "R" + std::to_string(square + 8);
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }

                newBoard[square + 8] = white_queen;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(square) + "Q" + std::to_string(square + 8);
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
            }

            //Or eat on the right and left
            if((square < 56) && ((square % 8) < 7) && (PIECES_COLORS[this->board[square + 8 + 1]] == -1)) {
              ChessBoard newBoard = this->board;
              std::array <int,2> newKingPositions = this->kingPositions;
              std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
              newBoard[square] = empty;
              newBoard[square + 8 + 1] = white_pawn;

              //And if the move does not lead to check it a possible move
              if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
              {
                moveName  = "P" + std::to_string(square) + "P" + std::to_string(square + 8 + 1);
                possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
              }

              //Eventually getting promoted if it is moving to the last row
              if((square / 8) == 6) {
                newBoard[square + 8 + 1] = white_knight;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(square) + "N" + std::to_string(square + 8 + 1);
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }

                newBoard[square + 8 + 1] = white_bishop;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(square) + "B" + std::to_string(square + 8 + 1);
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }

                newBoard[square + 8 + 1] = white_rook;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(square) + "R" + std::to_string(square + 8 + 1);
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }

                newBoard[square + 8 + 1] = white_queen;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(square) + "Q" + std::to_string(square + 8 + 1);
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
            }
            if((square < 56) && ((square % 8) > 0) && (PIECES_COLORS[this->board[square + 8 - 1]] == -1)) {
              ChessBoard newBoard = this->board;
              std::array<int,2> newKingPositions = this->kingPositions;
              std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
              newBoard[square] = empty;
              newBoard[square + 8 - 1] = white_pawn;

              //And if the move does not lead to check it a possible move
              if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
              {
                moveName  = "P" + std::to_string(square) + "P" + std::to_string(square + 8 - 1);
                possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
              }

              //Eventually getting promoted if it is moving to the last row
              if((square / 8) == 6) {
                newBoard[square + 8 - 1] = white_knight;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(square) + "N" + std::to_string(square + 8 - 1);
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }

                newBoard[square + 8 - 1] = white_bishop;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(square) + "B" + std::to_string(square + 8 - 1);
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }

                newBoard[square + 8 - 1] = white_rook;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(square) + "R" + std::to_string(square + 8 - 1);
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }

                newBoard[square + 8 - 1] = white_queen;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(square) + "Q" + std::to_string(square + 8 - 1);
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
            }

            //If it is in his starting cell it can move ahead of two cells too
            if(((square / 8) == 1) && (this->board[square + 8] == empty) && (this->board[square + 16] == empty)) {
              ChessBoard newBoard = this->board;
              std::array<int,2> newKingPositions = this->kingPositions;
              std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
              newBoard[square] = empty;
              newBoard[square + 16] = white_pawn2;

              //And if the move does not lead to check it a possible move
              if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
              {
                moveName  = "P" + std::to_string(square) + "P" + std::to_string(square + 16);
                possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
              }
            }

            //If it is in the 5th row, a capture en passant is possible
            if((square >= 32) && (square < 39)) {
              if((this->board[(square + 1)] == black_pawn2) && (this->board[square + 9] == empty)) {
                ChessBoard newBoard = this->board;
                std::array<int,2> newKingPositions = this->kingPositions;
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                newBoard[square] = empty;
                newBoard[square + 1] = empty;
                newBoard[square + 9] = white_pawn;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(square) + "P" + std::to_string(square + 9);
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
            }

            //If it is in the 5th row, a capture en passant is possible
            if((square > 32) && (square <= 39)) {
              if((this->board[(square - 1)] == black_pawn2) && (this->board[square + 7] == empty)) {
                ChessBoard newBoard = this->board;
                std::array<int,2> newKingPositions = this->kingPositions;
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                newBoard[square] = empty;
                newBoard[square - 1] = empty;
                newBoard[square + 7] = white_pawn;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(square) + "P" + std::to_string(square + 7);
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
            }
            break;

          case white_pawn2:
            //A pawn can either move ahead of one step not eating
            if((square < 56) && (this->board[square + 8] == empty)) {
              ChessBoard newBoard = this->board;
              std::array<int,2> newKingPositions = this->kingPositions;
              std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
              newBoard[square] = empty;
              newBoard[square + 8] = white_pawn;

              //And if the move does not lead to check it a possible move
              if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
              {
                moveName  = "P" + std::to_string(square) + "P" + std::to_string(square + 8);
                possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
              }

              //Eventually getting promoted if it is moving to the last row
              if((square / 8) == 6) {
                newBoard[square + 8] = white_knight;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(square) + "N" + std::to_string(square + 8);
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }

                newBoard[square + 8] = white_bishop;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(square) + "B" + std::to_string(square + 8);
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }

                newBoard[square + 8] = white_rook;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(square) + "R" + std::to_string(square + 8);
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }

                newBoard[square + 8] = white_queen;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(square) + "Q" + std::to_string(square + 8);
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
            }

            //Or eat on the right and left
            if((square < 56) && ((square % 8) < 7) && (PIECES_COLORS[this->board[square + 8 + 1]] == -1)) {
              ChessBoard newBoard = this->board;
              std::array <int,2> newKingPositions = this->kingPositions;
              std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
              newBoard[square] = empty;
              newBoard[square + 8 + 1] = white_pawn;

              //And if the move does not lead to check it a possible move
              if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
              {
                moveName  = "P" + std::to_string(square) + "P" + std::to_string(square + 8 + 1);
                possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
              }

              //Eventually getting promoted if it is moving to the last row
              if((square / 8) == 6) {
                newBoard[square + 8 + 1] = white_knight;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(square) + "N" + std::to_string(square + 8 + 1);
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }

                newBoard[square + 8 + 1] = white_bishop;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(square) + "B" + std::to_string(square + 8 + 1);
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }

                newBoard[square + 8 + 1] = white_rook;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(square) + "R" + std::to_string(square + 8 + 1);
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }

                newBoard[square + 8 + 1] = white_queen;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(square) + "Q" + std::to_string(square + 8 + 1);
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
            }
            if((square < 56) && ((square % 8) > 0) && (PIECES_COLORS[this->board[square + 8 - 1]] == -1)) {
              ChessBoard newBoard = this->board;
              std::array<int,2> newKingPositions = this->kingPositions;
              std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
              newBoard[square] = empty;
              newBoard[square + 8 - 1] = white_pawn;

              //And if the move does not lead to check it a possible move
              if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
              {
                moveName  = "P" + std::to_string(square) + "P" + std::to_string(square + 8 - 1);
                possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
              }

              //Eventually getting promoted if it is moving to the last row
              if((square / 8) == 6) {
                newBoard[square + 8 - 1] = white_knight;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(square) + "N" + std::to_string(square + 8 - 1);
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }

                newBoard[square + 8 - 1] = white_bishop;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(square) + "B" + std::to_string(square + 8 - 1);
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }

                newBoard[square + 8 - 1] = white_rook;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(square) + "R" + std::to_string(square + 8 - 1);
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }

                newBoard[square + 8 - 1] = white_queen;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(square) + "Q" + std::to_string(square + 8 - 1);
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
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
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                newBoard[square] = empty;
                newBoard[square + (9 * n)] = white_bishop;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  moveName  = "B" + std::to_string(square) + "B" + std::to_string(square + (9 * n));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + (9 * n)]] == -1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  newBoard[square] = empty;
                  newBoard[square + (9 * n)] = white_bishop;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    moveName  = "B" + std::to_string(square) + "B" + std::to_string(square + (9 * n));
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
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
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                newBoard[square] = empty;
                newBoard[square - (9 * n)] = white_bishop;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  moveName  = "B" + std::to_string(square) + "B" + std::to_string(square - (9 * n));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - (9 * n)]] == -1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  newBoard[square] = empty;
                  newBoard[square - (9 * n)] = white_bishop;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    moveName  = "B" + std::to_string(square) + "B" + std::to_string(square - (9 * n));
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
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
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                newBoard[square] = empty;
                newBoard[square + (7 * n)] = white_bishop;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  moveName  = "B" + std::to_string(square) + "B" + std::to_string(square + (7 * n));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + (7 * n)]] == -1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  newBoard[square] = empty;
                  newBoard[square + (7 * n)] = white_bishop;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    moveName  = "B" + std::to_string(square) + "B" + std::to_string(square + (7 * n));
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
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
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                newBoard[square] = empty;
                newBoard[square - (7 * n)] = white_bishop;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  moveName  = "B" + std::to_string(square) + "B" + std::to_string(square - (7 * n));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - (7 * n)]] == -1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  newBoard[square] = empty;
                  newBoard[square - (7 * n)] = white_bishop;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    moveName  = "B" + std::to_string(square) + "B" + std::to_string(square - (7 * n));
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
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
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                if(square == 0) {
                  newPossibleCastling[0][0] = 0;
                }
                if(square == 7) {
                  newPossibleCastling[0][1] = 0;
                }
                newBoard[square] = empty;
                newBoard[square + (8 * i)] = white_rook;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  moveName  = "R" + std::to_string(square) + "R" + std::to_string(square + (8 * i));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + (8 * i)]] == -1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  if(square == 0) {
                    newPossibleCastling[0][0] = 0;
                  }
                  if(square == 7) {
                    newPossibleCastling[0][1] = 0;
                  }
                  newBoard[square] = empty;
                  newBoard[square + (8 * i)] = white_rook;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    moveName  = "R" + std::to_string(square) + "R" + std::to_string(square + (8 * i));
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
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
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                if(square == 0) {
                  newPossibleCastling[0][0] = 0;
                }
                if(square == 7) {
                  newPossibleCastling[0][1] = 0;
                }
                newBoard[square] = empty;
                newBoard[square - (8 * i)] = white_rook;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  moveName  = "R" + std::to_string(square) + "R" + std::to_string(square - (8 * i));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - (8 * i)]] == -1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  if(square == 0) {
                    newPossibleCastling[0][0] = 0;
                  }
                  if(square == 7) {
                    newPossibleCastling[0][1] = 0;
                  }
                  newBoard[square] = empty;
                  newBoard[square - (8 * i)] = white_rook;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    moveName  = "R" + std::to_string(square) + "R" + std::to_string(square - (8 * i));
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
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
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                if(square == 0) {
                  newPossibleCastling[0][0] = 0;
                }
                if(square == 7) {
                  newPossibleCastling[0][1] = 0;
                }
                newBoard[square] = empty;
                newBoard[square + j] = white_rook;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  moveName  = "R" + std::to_string(square) + "R" + std::to_string(square + j);
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + j]] == -1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  if(square == 0) {
                    newPossibleCastling[0][0] = 0;
                  }
                  if(square == 7) {
                    newPossibleCastling[0][1] = 0;
                  }
                  newBoard[square] = empty;
                  newBoard[square + j] = white_rook;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    moveName  = "R" + std::to_string(square) + "R" + std::to_string(square + j);
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
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
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                if(square == 0) {
                  newPossibleCastling[0][0] = 0;
                }
                if(square == 7) {
                  newPossibleCastling[0][1] = 0;
                }
                newBoard[square] = empty;
                newBoard[square - j] = white_rook;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  moveName  = "R" + std::to_string(square) + "R" + std::to_string(square - j);
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - j]] == -1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  if(square == 0) {
                    newPossibleCastling[0][0] = 0;
                  }
                  if(square == 7) {
                    newPossibleCastling[0][1] = 0;
                  }
                  newBoard[square] = empty;
                  newBoard[square - j] = white_rook;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    moveName  = "R" + std::to_string(square) + "R" + std::to_string(square - j);
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
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
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                newBoard[square] = empty;
                newBoard[square + (8 * i)] = white_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  moveName  = "Q" + std::to_string(square) + "Q" + std::to_string(square + (8 * i));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + (8 * i)]] == -1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  newBoard[square] = empty;
                  newBoard[square + (8 * i)] = white_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    moveName  = "Q" + std::to_string(square) + "Q" + std::to_string(square + (8 * i));
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
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
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                newBoard[square] = empty;
                newBoard[square - (8 * i)] = white_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  moveName  = "Q" + std::to_string(square) + "Q" + std::to_string(square - (8 * i));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - (8 * i)]] == -1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  newBoard[square] = empty;
                  newBoard[square - (8 * i)] = white_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    moveName  = "Q" + std::to_string(square) + "Q" + std::to_string(square - (8 * i));
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
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
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                newBoard[square] = empty;
                newBoard[square + j] = white_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  moveName  = "Q" + std::to_string(square) + "Q" + std::to_string(square + j);
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + j]] == -1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  newBoard[square] = empty;
                  newBoard[square + j] = white_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    moveName  = "Q" + std::to_string(square) + "Q" + std::to_string(square + j);
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
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
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                newBoard[square] = empty;
                newBoard[square - j] = white_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  moveName  = "Q" + std::to_string(square) + "Q" + std::to_string(square - j);
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - j]] == -1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  newBoard[square] = empty;
                  newBoard[square - j] = white_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    moveName  = "Q" + std::to_string(square) + "Q" + std::to_string(square - j);
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
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
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                newBoard[square] = empty;
                newBoard[square + (9 * n)] = white_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  moveName  = "Q" + std::to_string(square) + "Q" + std::to_string(square + (9 * n));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + (9 * n)]] == -1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  newBoard[square] = empty;
                  newBoard[square + (9 * n)] = white_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    moveName  = "Q" + std::to_string(square) + "Q" + std::to_string(square + (9 * n));
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
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
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                newBoard[square] = empty;
                newBoard[square - (9 * n)] = white_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  moveName  = "Q" + std::to_string(square) + "Q" + std::to_string(square - (9 * n));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - (9 * n)]] == -1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  newBoard[square] = empty;
                  newBoard[square - (9 * n)] = white_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    moveName  = "Q" + std::to_string(square) + "Q" + std::to_string(square - (9 * n));
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
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
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                newBoard[square] = empty;
                newBoard[square + (7 * n)] = white_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  moveName  = "Q" + std::to_string(square) + "Q" + std::to_string(square + (7 * n));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + (7 * n)]] == -1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  newBoard[square] = empty;
                  newBoard[square + (7 * n)] = white_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    moveName  = "Q" + std::to_string(square) + "Q" + std::to_string(square + (7 * n));
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
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
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                newBoard[square] = empty;
                newBoard[square - (7 * n)] = white_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  moveName  = "Q" + std::to_string(square) + "Q" + std::to_string(square - (7 * n));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - (7 * n)]] == -1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  newBoard[square] = empty;
                  newBoard[square - (7 * n)] = white_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    moveName  = "Q" + std::to_string(square) + "Q" + std::to_string(square - (7 * n));
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
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
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  newBoard[square] = empty;
                  newBoard[(8 * i) + j] = white_knight;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    moveName  = "N" + std::to_string(square) + "N" + std::to_string((8 * i) + j);
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
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
                      std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                      newPossibleCastling[0][0] = 0;
                      newPossibleCastling[0][1] = 0;
                      newKingPositions[0] = square + (8 * i) + j;
                      newBoard[square] = empty;
                      newBoard[square + (8 * i) + j] = white_king;

                      //And if the move does not lead to check it a possible move
                      if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                      {
                        moveName  = "K" + std::to_string(square) + "K" + std::to_string(square + (8 * i) + j);
                        possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
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
              std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
              newBoard[square] = empty;
              newBoard[square - 8] = black_pawn;

              //And if the move does not lead to check it a possible move
              if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
              {
                moveName  = "P" + std::to_string(63 - square) + "P" + std::to_string(63 - (square - 8));
                possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
              }

              //Eventually getting promoted if it is moving to the last row
              if((square / 8) == 1) {
                newBoard[square - 8] = black_knight;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(63 - square) + "N" + std::to_string(63 - (square - 8));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }

                newBoard[square - 8] = black_bishop;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(63 - square) + "B" + std::to_string(63 - (square - 8));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }

                newBoard[square - 8] = black_rook;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(63 - square) + "R" + std::to_string(63 - (square - 8));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }

                newBoard[square - 8] = black_queen;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(63 - square) + "Q" + std::to_string(63 - (square - 8));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
            }

            //Or eat on the right and left
            if((square >= 8) && ((square % 8) < 7) && (PIECES_COLORS[this->board[square - 8 + 1]] == 1)) {
              ChessBoard newBoard = this->board;
              std::array<int,2> newKingPositions = this->kingPositions;
              std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
              newBoard[square] = empty;
              newBoard[square - 8 + 1] = black_pawn;

              //And if the move does not lead to check it a possible move
              if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
              {
                moveName  = "P" + std::to_string(63 - square) + "P" + std::to_string(63 - (square - 8 + 1));
                possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
              }

              //Eventually getting promoted if it is moving to the last row
              if((square / 8) == 1) {
                newBoard[square - 8 + 1] = black_knight;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(63 - square) + "N" + std::to_string(63 - (square - 8 + 1));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }

                newBoard[square - 8 + 1] = black_bishop;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(63 - square) + "B" + std::to_string(63 - (square - 8 + 1));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }

                newBoard[square - 8 + 1] = black_rook;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(63 - square) + "R" + std::to_string(63 - (square - 8 + 1));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }

                newBoard[square - 8 + 1] = black_queen;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(63 - square) + "Q" + std::to_string(63 - (square - 8 + 1));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
            }
            if((square >= 8) && ((square % 8) > 0) && (PIECES_COLORS[this->board[square - 8 - 1]] == 1)) {
              ChessBoard newBoard = this->board;
              std::array<int,2> newKingPositions = this->kingPositions;
              std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
              newBoard[square] = empty;
              newBoard[square - 8 - 1] = black_pawn;

              //And if the move does not lead to check it a possible move
              if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
              {
                moveName  = "P" + std::to_string(63 - square) + "P" + std::to_string(63 - (square - 8 - 1));
                possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
              }

              //Eventually getting promoted if it is moving to the last row
              if((square / 8) == 1) {
                newBoard[square - 8 - 1] = black_knight;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(63 - square) + "N" + std::to_string(63 - (square - 8 - 1));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }

                newBoard[square - 8 - 1] = black_bishop;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(63 - square) + "B" + std::to_string(63 - (square - 8 - 1));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }

                newBoard[square - 8 - 1] = black_rook;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                 moveName  = "P" + std::to_string(63 - square) + "R" + std::to_string(63 - (square - 8 - 1));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }

                newBoard[square - 8 - 1] = black_queen;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(63 - square) + "Q" + std::to_string(63 - (square - 8 - 1));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
            }

            //If it is in his starting cell it can move ahead of two cells too
            if(((square / 8) == 6) && (this->board[square - 8] == empty) && (this->board[square - 16] == empty)) {
              ChessBoard newBoard = this->board;
              std::array<int,2> newKingPositions = this->kingPositions;
              std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
              newBoard[square] = empty;
              newBoard[square - 16] = black_pawn2;

              //And if the move does not lead to check it a possible move
              if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
              {
                moveName  = "P" + std::to_string(63 - square) + "P" + std::to_string(63 - (square - 16));
                possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
              }
            }

            //If it is in the 4th row, a capture en passant is possible
            if((square >= 24) && (square < 31)) {
              if((this->board[(square + 1)] == white_pawn2) && (this->board[square - 7] == empty)) {
                ChessBoard newBoard = this->board;
                std::array<int,2> newKingPositions = this->kingPositions;
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                newBoard[square] = empty;
                newBoard[square + 1] = empty;
                newBoard[square - 7] = black_pawn;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(63 - square) + "P" + std::to_string(63 - (square - 7));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
            }

            //If it is in the 4th row, a capture en passant is possible
            if((square > 24) && (square <= 31)) {
              if((this->board[(square - 1)] == white_pawn2) && (this->board[square - 9] == empty)) {
                ChessBoard newBoard = this->board;
                std::array<int,2> newKingPositions = this->kingPositions;
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                newBoard[square] = empty;
                newBoard[square - 1] = empty;
                newBoard[square - 9] = black_pawn;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(63 - square) + "P" + std::to_string(63 - (square - 9));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
            }
            break;

          case black_pawn2:
            //A pawn can either move ahead of one step not eating
            if((square >= 8) && (this->board[square - 8] == empty)) {
              ChessBoard newBoard = this->board;
              std::array<int,2> newKingPositions = this->kingPositions;
              std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
              newBoard[square] = empty;
              newBoard[square - 8] = black_pawn;

              //And if the move does not lead to check it a possible move
              if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
              {
                moveName  = "P" + std::to_string(63 - square) + "P" + std::to_string(63 - (square - 8));
                possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
              }

              //Eventually getting promoted if it is moving to the last row
              if((square / 8) == 1) {
                newBoard[square - 8] = black_knight;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(63 - square) + "N" + std::to_string(63 - (square - 8));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }

                newBoard[square - 8] = black_bishop;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(63 - square) + "B" + std::to_string(63 - (square - 8));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }

                newBoard[square - 8] = black_rook;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(63 - square) + "R" + std::to_string(63 - (square - 8));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }

                newBoard[square - 8] = black_queen;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(63 - square) + "Q" + std::to_string(63 - (square - 8));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
            }

            //Or eat on the right and left
            if((square >= 8) && ((square % 8) < 7) && (PIECES_COLORS[this->board[square - 8 + 1]] == 1)) {
              ChessBoard newBoard = this->board;
              std::array<int,2> newKingPositions = this->kingPositions;
              std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
              newBoard[square] = empty;
              newBoard[square - 8 + 1] = black_pawn;

              //And if the move does not lead to check it a possible move
              if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
              {
                moveName  = "P" + std::to_string(63 - square) + "P" + std::to_string(63 - (square - 8 + 1));
                possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
              }

              //Eventually getting promoted if it is moving to the last row
              if((square / 8) == 1) {
                newBoard[square - 8 + 1] = black_knight;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(63 - square) + "N" + std::to_string(63 - (square - 8 + 1));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }

                newBoard[square - 8 + 1] = black_bishop;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(63 - square) + "B" + std::to_string(63 - (square - 8 + 1));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }

                newBoard[square - 8 + 1] = black_rook;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(63 - square) + "R" + std::to_string(63 - (square - 8 + 1));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }

                newBoard[square - 8 + 1] = black_queen;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(63 - square) + "Q" + std::to_string(63 - (square - 8 + 1));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
            }
            if((square >= 8) && ((square % 8) > 0) && (PIECES_COLORS[this->board[square - 8 - 1]] == 1)) {
              ChessBoard newBoard = this->board;
              std::array<int,2> newKingPositions = this->kingPositions;
              std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
              newBoard[square] = empty;
              newBoard[square - 8 - 1] = black_pawn;

              //And if the move does not lead to check it a possible move
              if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
              {
                moveName  = "P" + std::to_string(63 - square) + "P" + std::to_string(63 - (square - 8 - 1));
                possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
              }

              //Eventually getting promoted if it is moving to the last row
              if((square / 8) == 1) {
                newBoard[square - 8 - 1] = black_knight;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(63 - square) + "N" + std::to_string(63 - (square - 8 - 1));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }

                newBoard[square - 8 - 1] = black_bishop;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(63 - square) + "B" + std::to_string(63 - (square - 8 - 1));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }

                newBoard[square - 8 - 1] = black_rook;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(63 - square) + "R" + std::to_string(63 - (square - 8 - 1));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }

                newBoard[square - 8 - 1] = black_queen;
                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  moveName  = "P" + std::to_string(63 - square) + "Q" + std::to_string(63 - (square - 8 - 1));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
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
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                newBoard[square] = empty;
                newBoard[square + (9 * n)] = black_bishop;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  moveName  = "B" + std::to_string(63 - square) + "B" + std::to_string(63 - (square + (9 * n)));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + (9 * n)]] == 1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  newBoard[square] = empty;
                  newBoard[square + (9 * n)] = black_bishop;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    moveName  = "B" + std::to_string(63 - square) + "B" + std::to_string(63 - (square + (9 * n)));
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
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
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                newBoard[square] = empty;
                newBoard[square - (9 * n)] = black_bishop;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  moveName  = "B" + std::to_string(63 - square) + "B" + std::to_string(63 - (square - (9 * n)));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - (9 * n)]] == 1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  newBoard[square] = empty;
                  newBoard[square - (9 * n)] = black_bishop;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    moveName  = "B" + std::to_string(63 - square) + "B" + std::to_string(63 - (square - (9 * n)));
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
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
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                newBoard[square] = empty;
                newBoard[square + (7 * n)] = black_bishop;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  moveName  = "B" + std::to_string(63 - square) + "B" + std::to_string(63 - (square + (7 * n)));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + (7 * n)]] == 1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  newBoard[square] = empty;
                  newBoard[square + (7 * n)] = black_bishop;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    moveName  = "B" + std::to_string(63 - square) + "B" + std::to_string(63 - (square + (7 * n)));
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
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
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                newBoard[square] = empty;
                newBoard[square - (7 * n)] = black_bishop;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  moveName  = "B" + std::to_string(63 - square) + "B" + std::to_string(63 - (square - (7 * n)));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - (7 * n)]] == 1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  newBoard[square] = empty;
                  newBoard[square - (7 * n)] = black_bishop;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    moveName  = "B" + std::to_string(63 - square) + "B" + std::to_string(63 - (square - (7 * n)));
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
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
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                if(square == 56) {
                  newPossibleCastling[1][0] = 0;
                }
                if(square == 63) {
                  newPossibleCastling[1][1] = 0;
                }
                newBoard[square] = empty;
                newBoard[square + (8 * i)] = black_rook;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  moveName  = "R" + std::to_string(63 - square) + "R" + std::to_string(63 - (square + (8 * i)));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + (8 * i)]] == 1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  if(square == 56) {
                    newPossibleCastling[1][0] = 0;
                  }
                  if(square == 63) {
                    newPossibleCastling[1][1] = 0;
                  }
                  newBoard[square] = empty;
                  newBoard[square + (8 * i)] = black_rook;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    moveName  = "R" + std::to_string(63 - square) + "R" + std::to_string(63 - (square + (8 * i)));
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
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
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                if(square == 56) {
                  newPossibleCastling[1][0] = 0;
                }
                if(square == 63) {
                  newPossibleCastling[1][1] = 0;
                }
                newBoard[square] = empty;
                newBoard[square - (8 * i)] = black_rook;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  moveName  = "R" + std::to_string(63 - square) + "R" + std::to_string(63 - (square - (8 * i)));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - (8 * i)]] == 1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  if(square == 56) {
                    newPossibleCastling[1][0] = 0;
                  }
                  if(square == 63) {
                    newPossibleCastling[1][1] = 0;
                  }
                  newBoard[square] = empty;
                  newBoard[square - (8 * i)] = black_rook;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    moveName  = "R" + std::to_string(63 - square) + "R" + std::to_string(63 - (square - (8 * i)));
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
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
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                if(square == 56) {
                  newPossibleCastling[1][0] = 0;
                }
                if(square == 63) {
                  newPossibleCastling[1][1] = 0;
                }
                newBoard[square] = empty;
                newBoard[square + j] = black_rook;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  moveName  = "R" + std::to_string(63 - square) + "R" + std::to_string(63 - (square + j));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + j]] == 1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  if(square == 56) {
                    newPossibleCastling[1][0] = 0;
                  }
                  if(square == 63) {
                    newPossibleCastling[1][1] = 0;
                  }
                  newBoard[square] = empty;
                  newBoard[square + j] = black_rook;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    moveName  = "R" + std::to_string(63 - square) + "R" + std::to_string(63 - (square + j));
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
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
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                if(square == 56) {
                  newPossibleCastling[1][0] = 0;
                }
                if(square == 63) {
                  newPossibleCastling[1][1] = 0;
                }
                newBoard[square] = empty;
                newBoard[square - j] = black_rook;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  moveName  = "R" + std::to_string(63 - square) + "R" + std::to_string(63 - (square - j));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - j]] == 1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  if(square == 56) {
                    newPossibleCastling[1][0] = 0;
                  }
                  if(square == 63) {
                    newPossibleCastling[1][1] = 0;
                  }
                  newBoard[square] = empty;
                  newBoard[square - j] = black_rook;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    moveName  = "R" + std::to_string(63 - square) + "R" + std::to_string(63 - (square - j));
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
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
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                newBoard[square] = empty;
                newBoard[square + (8 * i)] = black_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  moveName  = "Q" + std::to_string(63 - square) + "Q" + std::to_string(63 - (square + (8 * i)));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + (8 * i)]] == 1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  newBoard[square] = empty;
                  newBoard[square + (8 * i)] = black_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    moveName  = "Q" + std::to_string(63 - square) + "Q" + std::to_string(63 - (square + (8 * i)));
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
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
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                newBoard[square] = empty;
                newBoard[square - (8 * i)] = black_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  moveName  = "Q" + std::to_string(63 - square) + "Q" + std::to_string(63 - (square - (8 * i)));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - (8 * i)]] == 1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  newBoard[square] = empty;
                  newBoard[square - (8 * i)] = black_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    moveName  = "Q" + std::to_string(63 - square) + "Q" + std::to_string(63 - (square - (8 * i)));
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
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
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                newBoard[square] = empty;
                newBoard[square + j] = black_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  moveName  = "Q" + std::to_string(63 - square) + "Q" + std::to_string(63 - (square + j));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + j]] == 1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  newBoard[square] = empty;
                  newBoard[square + j] = black_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    moveName  = "Q" + std::to_string(63 - square) + "Q" + std::to_string(63 - (square + j));
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
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
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                newBoard[square] = empty;
                newBoard[square - j] = black_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  moveName  = "Q" + std::to_string(63 - square) + "Q" + std::to_string(63 - (square - j));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - j]] == 1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  newBoard[square] = empty;
                  newBoard[square - j] = black_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    moveName  = "Q" + std::to_string(63 - square) + "Q" + std::to_string(63 - (square - j));
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
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
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                newBoard[square] = empty;
                newBoard[square + (9 * n)] = black_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  moveName  = "Q" + std::to_string(63 - square) + "Q" + std::to_string(63 - (square + (9 * n)));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + (9 * n)]] == 1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  newBoard[square] = empty;
                  newBoard[square + (9 * n)] = black_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    moveName  = "Q" + std::to_string(63 - square) + "Q" + std::to_string(63 - (square + (9 * n)));
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
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
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                newBoard[square] = empty;
                newBoard[square - (9 * n)] = black_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  moveName  = "Q" + std::to_string(63 - square) + "Q" + std::to_string(63 - (square - (9 * n)));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - (9 * n)]] == 1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  newBoard[square] = empty;
                  newBoard[square - (9 * n)] = black_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    moveName  = "Q" + std::to_string(63 - square) + "Q" + std::to_string(63 - (square - (9 * n)));
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
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
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                newBoard[square] = empty;
                newBoard[square + (7 * n)] = black_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  moveName  = "Q" + std::to_string(63 - square) + "Q" + std::to_string(63 - (square + (7 * n)));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + (7 * n)]] == 1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  newBoard[square] = empty;
                  newBoard[square + (7 * n)] = black_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    moveName  = "Q" + std::to_string(63 - square) + "Q" + std::to_string(63 - (square + (7 * n)));
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
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
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                newBoard[square] = empty;
                newBoard[square - (7 * n)] = black_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  moveName  = "Q" + std::to_string(63 - square) + "Q" + std::to_string(63 - (square - (7 * n)));
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - (7 * n)]] == 1) {
                  ChessBoard newBoard = this->board;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  newBoard[square] = empty;
                  newBoard[square - (7 * n)] = black_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    moveName  = "Q" + std::to_string(63 - square) + "Q" + std::to_string(63 - (square - (7 * n)));
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
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
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  newBoard[square] = empty;
                  newBoard[(8 * i) + j] = black_knight;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    moveName  = "N" + std::to_string(63 - square) + "N" + std::to_string(63 - ((8 * i) + j));
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
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
                      std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                      newPossibleCastling[1][0] = 0;
                      newPossibleCastling[1][1] = 0;
                      newKingPositions[1] = square + (8 * i) + j;
                      newBoard[square] = empty;
                      newBoard[square + (8 * i) + j] = black_king;

                      //And if the move does not lead to check it a possible move
                      if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                      {
                        moveName  = "K" + std::to_string(63 - square) + "K" + std::to_string(63 - (square + (8 * i) + j));
                        possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary[moveName]));
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

    //We also have to check if castling is possible
    if(this->player == 1) {  
      if(possibleCastling[0][0]) {
        if((this->board[1] == empty) && (this->board[2] == empty) && (this->board[3] == empty)) {
          if((ChessState::isUnderAttack(this->board, 2, -1) == false) && (ChessState::isUnderAttack(this->board, 3, -1) == false)) {
            //The castling is possible
            ChessBoard newBoard = this->board;
            std::array<int,2> newKingPositions = this->kingPositions;
            std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
            newPossibleCastling[0][0] = 0;
            newPossibleCastling[0][1] = 0;
            newKingPositions[0] = 2;
            newBoard[0] = empty;
            newBoard[4] = empty;
            newBoard[2] = white_king;
            newBoard[3] = white_rook;
            possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary["CL"]));
          }
        }
      }
      if(possibleCastling[0][1]) {
        if((this->board[5] == empty) && (this->board[6] == empty)) {
          if((ChessState::isUnderAttack(this->board, 5, -1) == false) && (ChessState::isUnderAttack(this->board, 6, -1) == false)) {
            //The castling is possible
            ChessBoard newBoard = this->board;
            std::array<int,2> newKingPositions = this->kingPositions;
            std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
            newPossibleCastling[0][0] = 0;
            newPossibleCastling[0][1] = 0;
            newKingPositions[0] = 6;
            newBoard[7] = empty;
            newBoard[4] = empty;
            newBoard[6] = white_king;
            newBoard[5] = white_rook;
            possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary["CR"]));
          }
        }
      }
    }
    else {
      if(possibleCastling[1][0]) {
        if((this->board[57] == empty) && (this->board[58] == empty) && (this->board[59] == empty)) {
          if((ChessState::isUnderAttack(this->board, 58, 1) == false) && (ChessState::isUnderAttack(this->board, 59, 1) == false)) {
            //The castling is possible
            ChessBoard newBoard = this->board;
            std::array<int,2> newKingPositions = this->kingPositions;
            std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
            newPossibleCastling[1][0] = 0;
            newPossibleCastling[1][1] = 0;
            newKingPositions[1] = 58;
            newBoard[56] = empty;
            newBoard[60] = empty;
            newBoard[58] = black_king;
            newBoard[59] = black_rook;
            possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary["CL"]));
          }
        }
      }
      if(possibleCastling[1][1]) {
        if((this->board[61] == empty) && (this->board[62] == empty)) {
          if((ChessState::isUnderAttack(this->board, 61, 1) == false) && (ChessState::isUnderAttack(this->board, 62, 1) == false)) {
            //The castling is possible
            ChessBoard newBoard = this->board;
            std::array<int,2> newKingPositions = this->kingPositions;
            std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
            newPossibleCastling[1][0] = 0;
            newPossibleCastling[1][1] = 0;
            newKingPositions[1] = 62;
            newBoard[63] = empty;
            newBoard[60] = empty;
            newBoard[62] = white_king;
            newBoard[61] = white_rook;
            possibleMoves.push_back(ChessMove(new ChessState(newBoard, newKingPositions, newPossibleCastling, (-1 * this->player)), MovesDictionary["CR"]));
          }
        }
      }
    }

    return possibleMoves;
}


std::vector<double> ChessState::getNetworkInput(void) {
  std::vector<double> netInput;

  for(int sign=1;sign>=-1;sign-=2) {
    for(int square=0;square<64;square++) {
      if(PIECES_COLORS[this->board[square]] != (-1 * sign * this->player)) {
        netInput.push_back(this->board[square]);
      }
      else{
          netInput.push_back(0);
      }
    }

    if(sign *  this->player == 1) {
      netInput.push_back(this->possibleCastling[0][0]);
      netInput.push_back(this->possibleCastling[0][1]);
    } else {
      netInput.push_back(this->possibleCastling[1][0]);
      netInput.push_back(this->possibleCastling[1][1]);
    }
  }

  return netInput;
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


//All the legal moves from this state have to be built
void CreateChessMovesDictionary(void) {
	int i, j, n;
    int square1, square2;
    std::vector<std::string> moveNames;

    std::ofstream dict_file;

    dict_file.open("Chess_Dictionary.txt", std::ios::out);

    //Let's start with the pawn
    //For all the square a pawn can start from
    for(square1=8;square1<56;square1++) {
    	//It can move ahead of one row
    	square2 = square1 + 8;
    	moveNames.push_back("P" + std::to_string(square1) + "P" + std::to_string(square2));
    	//Eventually promoting if it started from the last line
    	if((square1 / 8) == 6) {
    		moveNames.push_back("P" + std::to_string(square1) + "Q" + std::to_string(square2));
    		moveNames.push_back("P" + std::to_string(square1) + "N" + std::to_string(square2));
    		moveNames.push_back("P" + std::to_string(square1) + "B" + std::to_string(square2));
    		moveNames.push_back("P" + std::to_string(square1) + "R" + std::to_string(square2));
    	}

    	//Or, eating, one step to the left or right (if it is not at the chessboard border)
    	if((square1 % 8) > 0) {
    		square2 = square1 + 7;
    		moveNames.push_back("P" + std::to_string(square1) + "P" + std::to_string(square2));
	    	//Eventually promoting if it started from the last line
	    	if((square1 / 8) == 6) {
	    		moveNames.push_back("P" + std::to_string(square1) + "Q" + std::to_string(square2));
	    		moveNames.push_back("P" + std::to_string(square1) + "N" + std::to_string(square2));
	    		moveNames.push_back("P" + std::to_string(square1) + "B" + std::to_string(square2));
	    		moveNames.push_back("P" + std::to_string(square1) + "R" + std::to_string(square2));
	    	}
    	}
    	if((square1 % 8) < 7) {
    		square2 = square1 + 9;
    		moveNames.push_back("P" + std::to_string(square1) + "P" + std::to_string(square2));
	    	//Eventually promoting if it started from the last line
	    	if((square1 / 8) == 6) {
	    		moveNames.push_back("P" + std::to_string(square1) + "Q" + std::to_string(square2));
	    		moveNames.push_back("P" + std::to_string(square1) + "N" + std::to_string(square2));
	    		moveNames.push_back("P" + std::to_string(square1) + "B" + std::to_string(square2));
	    		moveNames.push_back("P" + std::to_string(square1) + "R" + std::to_string(square2));
	    	}
    	}

    	//Moreover, if it is in the first line, it can move ahead two steps
    	if((square1 / 8) == 1) {
    		square2 = square1 + 16;
    		moveNames.push_back("P" + std::to_string(square1) + "P" + std::to_string(square2));
    	}
    }

    //Then, let's consider the bishop
    //From every starting square
    for(square1=8;square1<56;square1++) {
    	//A bishop can move on the positive or negative diagonal
    	//Starting from the positive diagonal
    	n = 1;
        while(((square1 + (9 * n)) < 64) && (((square1 + (9 * n)) % 8) != 0)) {
            square2 = square1 + (9 * n);
    		moveNames.push_back("B" + std::to_string(square1) + "B" + std::to_string(square2));

            n++;
        }
    	n = 1;
        while(((square1 - (9 * n)) >= 0) && (((square1 - (9 * n)) % 8) != 7)) {
            square2 = square1 - (9 * n);
    		moveNames.push_back("B" + std::to_string(square1) + "B" + std::to_string(square2));

            n++;
        }

        //And then considering the negative diagonal
        n = 1;
        while(((square1 + (7 * n)) < 64) && (((square1 + (7 * n)) % 8) != 7)) {
            square2 = square1 + (7 * n);
    		moveNames.push_back("B" + std::to_string(square1) + "B" + std::to_string(square2));

    		n++;
        }
        n = 1;
        while(((square1 - (7 * n)) >= 0) && (((square1 - (7 * n)) % 8) != 0)) {
        	square2 = square1 - (7 * n);
    		moveNames.push_back("B" + std::to_string(square1) + "B" + std::to_string(square2));

        	n++;
        }
    }


    //Then, let's consider the rook
    //From every starting square
    for(square1=8;square1<56;square1++) {
    	//The rook can move along the vertical line
        i = 1;
        while((square1 + (8 * i)) < 64) {
        	square2 = square1 + (8 * i);
	    	moveNames.push_back("R" + std::to_string(square1) + "R" + std::to_string(square2));

	    	i++;
        } 
        i = 1;
        while((square1 - (8 * i)) >= 0) {
        	square2 = square1 - (8 * i);
	    	moveNames.push_back("R" + std::to_string(square1) + "R" + std::to_string(square2));

	    	i++;
        } 

        //Or along the horizontal line
        j = 1;
        while(((square1 + j) % 8) != 0) {
        	square2 = square1  + j;
	    	moveNames.push_back("R" + std::to_string(square1) + "R" + std::to_string(square2));

	    	j++;
        }
        j = 1;
        while((square1 - j >= 0) && (((square1 - j) % 8) != 7)) {
        	square2 = square1  - j;
	    	moveNames.push_back("R" + std::to_string(square1) + "R" + std::to_string(square2));

	    	j++;
        }
    }


    //Then, let's consider the queen
    //From every starting square
    for(square1=8;square1<56;square1++) {
    	//The queen can move along the positive diagonal
    	n = 1;
        while(((square1 + (9 * n)) < 64) && (((square1 + (9 * n)) % 8) != 0)) {
            square2 = square1 + (9 * n);
    		moveNames.push_back("Q" + std::to_string(square1) + "Q" + std::to_string(square2));

            n++;
        }
    	n = 1;
        while(((square1 - (9 * n)) >= 0) && (((square1 - (9 * n)) % 8) != 7)) {
            square2 = square1 - (9 * n);
    		moveNames.push_back("Q" + std::to_string(square1) + "Q" + std::to_string(square2));

            n++;
        }

        //And the negative one
        n = 1;
        while(((square1 + (7 * n)) < 64) && (((square1 + (7 * n)) % 8) != 7)) {
            square2 = square1 + (7 * n);
    		moveNames.push_back("Q" + std::to_string(square1) + "Q" + std::to_string(square2));

    		n++;
        }
        n = 1;
        while(((square1 - (7 * n)) >= 0) && (((square1 - (7 * n)) % 8) != 0)) {
        	square2 = square1 - (7 * n);
    		moveNames.push_back("Q" + std::to_string(square1) + "Q" + std::to_string(square2));

        	n++;
        }


    	//As well as along the vertical line
        i = 1;
        while((square1 + (8 * i)) < 64) {
        	square2 = square1 + (8 * i);
	    	moveNames.push_back("Q" + std::to_string(square1) + "Q" + std::to_string(square2));

	    	i++;
        } 
        i = 1;
        while((square1 - (8 * i)) >= 0) {
        	square2 = square1 - (8 * i);
	    	moveNames.push_back("Q" + std::to_string(square1) + "Q" + std::to_string(square2));

	    	i++;
        } 

        //Or along the horizontal line
        j = 1;
        while(((square1 + j) % 8) != 0) {
        	square2 = square1  + j;
	    	moveNames.push_back("Q" + std::to_string(square1) + "Q" + std::to_string(square2));

	    	j++;
        }
        j = 1;
        while((square1 - j >= 0) && (((square1 - j) % 8) != 7)) {
        	square2 = square1  - j;
	    	moveNames.push_back("Q" + std::to_string(square1) + "Q" + std::to_string(square2));

	    	j++;
        }
    }


    //Then, consider the knight
    //From every starting square
    for(square1=8;square1<56;square1++) {
    	//For every of the possible jumping squares
    	for(n=0;n<8;n++)
        {
            //If the jumping cell is a real cell
            i = (square1 / 8) + KNIGHT_JUMPS[n][0];
            j = (square1 % 8) + KNIGHT_JUMPS[n][1];

            //Add the move
            square2 = (8 * i) + j;
	    	moveNames.push_back("N" + std::to_string(square1) + "N" + std::to_string(square2));
	   	}
    }

    //And, in the end, consider the king
    //From every starting square
    for(square1=8;square1<56;square1++) {
    	//The king can move in all the directions, but of only one square
        for(i=-1;i<=1;i++) {
            for(j=-1;j<=1;j++) {
                //And if it exists
                if((i != 0) || (j != 0))
                {
                	if((((square1 / 8) + i)>=0) && (((square1 / 8) + i)<8) && (((square1 % 8) + j)>=0) && (((square1 % 8) + j)<8))
                    {
                    	//Add the move
                    	square2 = square1 + (8 * i) + j;
	    				moveNames.push_back("K" + std::to_string(square1) + "K" + std::to_string(square2));
                    }
                }
            }
        }
    }

    //And, in the end, add the castling left and the castling right
	moveNames.push_back("CL");
	moveNames.push_back("CR");


	dict_file << "std::unordered_map<std::string, int> ChessState::MovesDictionary = {\n";

	//Then, output the dictionary
    for(n=0;n<moveNames.size();n++) {
    	if(n<(moveNames.size() - 1)) {
    		dict_file << "    {\"" << moveNames[n] << "\", " << n << "},\n";
    	}
    	else {
    		dict_file << "    {\"" << moveNames[n] << "\", " << n << "}\n";
    	}
    }

	dict_file << "};\n";

    dict_file.close();
}



std::unordered_map<std::string, int> ChessState::MovesDictionary = {
    {"P8P16", 0},
    {"P8P17", 1},
    {"P8P24", 2},
    {"P9P17", 3},
    {"P9P16", 4},
    {"P9P18", 5},
    {"P9P25", 6},
    {"P10P18", 7},
    {"P10P17", 8},
    {"P10P19", 9},
    {"P10P26", 10},
    {"P11P19", 11},
    {"P11P18", 12},
    {"P11P20", 13},
    {"P11P27", 14},
    {"P12P20", 15},
    {"P12P19", 16},
    {"P12P21", 17},
    {"P12P28", 18},
    {"P13P21", 19},
    {"P13P20", 20},
    {"P13P22", 21},
    {"P13P29", 22},
    {"P14P22", 23},
    {"P14P21", 24},
    {"P14P23", 25},
    {"P14P30", 26},
    {"P15P23", 27},
    {"P15P22", 28},
    {"P15P31", 29},
    {"P16P24", 30},
    {"P16P25", 31},
    {"P17P25", 32},
    {"P17P24", 33},
    {"P17P26", 34},
    {"P18P26", 35},
    {"P18P25", 36},
    {"P18P27", 37},
    {"P19P27", 38},
    {"P19P26", 39},
    {"P19P28", 40},
    {"P20P28", 41},
    {"P20P27", 42},
    {"P20P29", 43},
    {"P21P29", 44},
    {"P21P28", 45},
    {"P21P30", 46},
    {"P22P30", 47},
    {"P22P29", 48},
    {"P22P31", 49},
    {"P23P31", 50},
    {"P23P30", 51},
    {"P24P32", 52},
    {"P24P33", 53},
    {"P25P33", 54},
    {"P25P32", 55},
    {"P25P34", 56},
    {"P26P34", 57},
    {"P26P33", 58},
    {"P26P35", 59},
    {"P27P35", 60},
    {"P27P34", 61},
    {"P27P36", 62},
    {"P28P36", 63},
    {"P28P35", 64},
    {"P28P37", 65},
    {"P29P37", 66},
    {"P29P36", 67},
    {"P29P38", 68},
    {"P30P38", 69},
    {"P30P37", 70},
    {"P30P39", 71},
    {"P31P39", 72},
    {"P31P38", 73},
    {"P32P40", 74},
    {"P32P41", 75},
    {"P33P41", 76},
    {"P33P40", 77},
    {"P33P42", 78},
    {"P34P42", 79},
    {"P34P41", 80},
    {"P34P43", 81},
    {"P35P43", 82},
    {"P35P42", 83},
    {"P35P44", 84},
    {"P36P44", 85},
    {"P36P43", 86},
    {"P36P45", 87},
    {"P37P45", 88},
    {"P37P44", 89},
    {"P37P46", 90},
    {"P38P46", 91},
    {"P38P45", 92},
    {"P38P47", 93},
    {"P39P47", 94},
    {"P39P46", 95},
    {"P40P48", 96},
    {"P40P49", 97},
    {"P41P49", 98},
    {"P41P48", 99},
    {"P41P50", 100},
    {"P42P50", 101},
    {"P42P49", 102},
    {"P42P51", 103},
    {"P43P51", 104},
    {"P43P50", 105},
    {"P43P52", 106},
    {"P44P52", 107},
    {"P44P51", 108},
    {"P44P53", 109},
    {"P45P53", 110},
    {"P45P52", 111},
    {"P45P54", 112},
    {"P46P54", 113},
    {"P46P53", 114},
    {"P46P55", 115},
    {"P47P55", 116},
    {"P47P54", 117},
    {"P48P56", 118},
    {"P48Q56", 119},
    {"P48N56", 120},
    {"P48B56", 121},
    {"P48R56", 122},
    {"P48P57", 123},
    {"P48Q57", 124},
    {"P48N57", 125},
    {"P48B57", 126},
    {"P48R57", 127},
    {"P49P57", 128},
    {"P49Q57", 129},
    {"P49N57", 130},
    {"P49B57", 131},
    {"P49R57", 132},
    {"P49P56", 133},
    {"P49Q56", 134},
    {"P49N56", 135},
    {"P49B56", 136},
    {"P49R56", 137},
    {"P49P58", 138},
    {"P49Q58", 139},
    {"P49N58", 140},
    {"P49B58", 141},
    {"P49R58", 142},
    {"P50P58", 143},
    {"P50Q58", 144},
    {"P50N58", 145},
    {"P50B58", 146},
    {"P50R58", 147},
    {"P50P57", 148},
    {"P50Q57", 149},
    {"P50N57", 150},
    {"P50B57", 151},
    {"P50R57", 152},
    {"P50P59", 153},
    {"P50Q59", 154},
    {"P50N59", 155},
    {"P50B59", 156},
    {"P50R59", 157},
    {"P51P59", 158},
    {"P51Q59", 159},
    {"P51N59", 160},
    {"P51B59", 161},
    {"P51R59", 162},
    {"P51P58", 163},
    {"P51Q58", 164},
    {"P51N58", 165},
    {"P51B58", 166},
    {"P51R58", 167},
    {"P51P60", 168},
    {"P51Q60", 169},
    {"P51N60", 170},
    {"P51B60", 171},
    {"P51R60", 172},
    {"P52P60", 173},
    {"P52Q60", 174},
    {"P52N60", 175},
    {"P52B60", 176},
    {"P52R60", 177},
    {"P52P59", 178},
    {"P52Q59", 179},
    {"P52N59", 180},
    {"P52B59", 181},
    {"P52R59", 182},
    {"P52P61", 183},
    {"P52Q61", 184},
    {"P52N61", 185},
    {"P52B61", 186},
    {"P52R61", 187},
    {"P53P61", 188},
    {"P53Q61", 189},
    {"P53N61", 190},
    {"P53B61", 191},
    {"P53R61", 192},
    {"P53P60", 193},
    {"P53Q60", 194},
    {"P53N60", 195},
    {"P53B60", 196},
    {"P53R60", 197},
    {"P53P62", 198},
    {"P53Q62", 199},
    {"P53N62", 200},
    {"P53B62", 201},
    {"P53R62", 202},
    {"P54P62", 203},
    {"P54Q62", 204},
    {"P54N62", 205},
    {"P54B62", 206},
    {"P54R62", 207},
    {"P54P61", 208},
    {"P54Q61", 209},
    {"P54N61", 210},
    {"P54B61", 211},
    {"P54R61", 212},
    {"P54P63", 213},
    {"P54Q63", 214},
    {"P54N63", 215},
    {"P54B63", 216},
    {"P54R63", 217},
    {"P55P63", 218},
    {"P55Q63", 219},
    {"P55N63", 220},
    {"P55B63", 221},
    {"P55R63", 222},
    {"P55P62", 223},
    {"P55Q62", 224},
    {"P55N62", 225},
    {"P55B62", 226},
    {"P55R62", 227},
    {"B8B17", 228},
    {"B8B26", 229},
    {"B8B35", 230},
    {"B8B44", 231},
    {"B8B53", 232},
    {"B8B62", 233},
    {"B8B1", 234},
    {"B9B18", 235},
    {"B9B27", 236},
    {"B9B36", 237},
    {"B9B45", 238},
    {"B9B54", 239},
    {"B9B63", 240},
    {"B9B0", 241},
    {"B9B16", 242},
    {"B9B2", 243},
    {"B10B19", 244},
    {"B10B28", 245},
    {"B10B37", 246},
    {"B10B46", 247},
    {"B10B55", 248},
    {"B10B1", 249},
    {"B10B17", 250},
    {"B10B24", 251},
    {"B10B3", 252},
    {"B11B20", 253},
    {"B11B29", 254},
    {"B11B38", 255},
    {"B11B47", 256},
    {"B11B2", 257},
    {"B11B18", 258},
    {"B11B25", 259},
    {"B11B32", 260},
    {"B11B4", 261},
    {"B12B21", 262},
    {"B12B30", 263},
    {"B12B39", 264},
    {"B12B3", 265},
    {"B12B19", 266},
    {"B12B26", 267},
    {"B12B33", 268},
    {"B12B40", 269},
    {"B12B5", 270},
    {"B13B22", 271},
    {"B13B31", 272},
    {"B13B4", 273},
    {"B13B20", 274},
    {"B13B27", 275},
    {"B13B34", 276},
    {"B13B41", 277},
    {"B13B48", 278},
    {"B13B6", 279},
    {"B14B23", 280},
    {"B14B5", 281},
    {"B14B21", 282},
    {"B14B28", 283},
    {"B14B35", 284},
    {"B14B42", 285},
    {"B14B49", 286},
    {"B14B56", 287},
    {"B14B7", 288},
    {"B15B6", 289},
    {"B15B22", 290},
    {"B15B29", 291},
    {"B15B36", 292},
    {"B15B43", 293},
    {"B15B50", 294},
    {"B15B57", 295},
    {"B16B25", 296},
    {"B16B34", 297},
    {"B16B43", 298},
    {"B16B52", 299},
    {"B16B61", 300},
    {"B16B9", 301},
    {"B16B2", 302},
    {"B17B26", 303},
    {"B17B35", 304},
    {"B17B44", 305},
    {"B17B53", 306},
    {"B17B62", 307},
    {"B17B8", 308},
    {"B17B24", 309},
    {"B17B10", 310},
    {"B17B3", 311},
    {"B18B27", 312},
    {"B18B36", 313},
    {"B18B45", 314},
    {"B18B54", 315},
    {"B18B63", 316},
    {"B18B9", 317},
    {"B18B0", 318},
    {"B18B25", 319},
    {"B18B32", 320},
    {"B18B11", 321},
    {"B18B4", 322},
    {"B19B28", 323},
    {"B19B37", 324},
    {"B19B46", 325},
    {"B19B55", 326},
    {"B19B10", 327},
    {"B19B1", 328},
    {"B19B26", 329},
    {"B19B33", 330},
    {"B19B40", 331},
    {"B19B12", 332},
    {"B19B5", 333},
    {"B20B29", 334},
    {"B20B38", 335},
    {"B20B47", 336},
    {"B20B11", 337},
    {"B20B2", 338},
    {"B20B27", 339},
    {"B20B34", 340},
    {"B20B41", 341},
    {"B20B48", 342},
    {"B20B13", 343},
    {"B20B6", 344},
    {"B21B30", 345},
    {"B21B39", 346},
    {"B21B12", 347},
    {"B21B3", 348},
    {"B21B28", 349},
    {"B21B35", 350},
    {"B21B42", 351},
    {"B21B49", 352},
    {"B21B56", 353},
    {"B21B14", 354},
    {"B21B7", 355},
    {"B22B31", 356},
    {"B22B13", 357},
    {"B22B4", 358},
    {"B22B29", 359},
    {"B22B36", 360},
    {"B22B43", 361},
    {"B22B50", 362},
    {"B22B57", 363},
    {"B22B15", 364},
    {"B23B14", 365},
    {"B23B5", 366},
    {"B23B30", 367},
    {"B23B37", 368},
    {"B23B44", 369},
    {"B23B51", 370},
    {"B23B58", 371},
    {"B24B33", 372},
    {"B24B42", 373},
    {"B24B51", 374},
    {"B24B60", 375},
    {"B24B17", 376},
    {"B24B10", 377},
    {"B24B3", 378},
    {"B25B34", 379},
    {"B25B43", 380},
    {"B25B52", 381},
    {"B25B61", 382},
    {"B25B16", 383},
    {"B25B32", 384},
    {"B25B18", 385},
    {"B25B11", 386},
    {"B25B4", 387},
    {"B26B35", 388},
    {"B26B44", 389},
    {"B26B53", 390},
    {"B26B62", 391},
    {"B26B17", 392},
    {"B26B8", 393},
    {"B26B33", 394},
    {"B26B40", 395},
    {"B26B19", 396},
    {"B26B12", 397},
    {"B26B5", 398},
    {"B27B36", 399},
    {"B27B45", 400},
    {"B27B54", 401},
    {"B27B63", 402},
    {"B27B18", 403},
    {"B27B9", 404},
    {"B27B0", 405},
    {"B27B34", 406},
    {"B27B41", 407},
    {"B27B48", 408},
    {"B27B20", 409},
    {"B27B13", 410},
    {"B27B6", 411},
    {"B28B37", 412},
    {"B28B46", 413},
    {"B28B55", 414},
    {"B28B19", 415},
    {"B28B10", 416},
    {"B28B1", 417},
    {"B28B35", 418},
    {"B28B42", 419},
    {"B28B49", 420},
    {"B28B56", 421},
    {"B28B21", 422},
    {"B28B14", 423},
    {"B28B7", 424},
    {"B29B38", 425},
    {"B29B47", 426},
    {"B29B20", 427},
    {"B29B11", 428},
    {"B29B2", 429},
    {"B29B36", 430},
    {"B29B43", 431},
    {"B29B50", 432},
    {"B29B57", 433},
    {"B29B22", 434},
    {"B29B15", 435},
    {"B30B39", 436},
    {"B30B21", 437},
    {"B30B12", 438},
    {"B30B3", 439},
    {"B30B37", 440},
    {"B30B44", 441},
    {"B30B51", 442},
    {"B30B58", 443},
    {"B30B23", 444},
    {"B31B22", 445},
    {"B31B13", 446},
    {"B31B4", 447},
    {"B31B38", 448},
    {"B31B45", 449},
    {"B31B52", 450},
    {"B31B59", 451},
    {"B32B41", 452},
    {"B32B50", 453},
    {"B32B59", 454},
    {"B32B25", 455},
    {"B32B18", 456},
    {"B32B11", 457},
    {"B32B4", 458},
    {"B33B42", 459},
    {"B33B51", 460},
    {"B33B60", 461},
    {"B33B24", 462},
    {"B33B40", 463},
    {"B33B26", 464},
    {"B33B19", 465},
    {"B33B12", 466},
    {"B33B5", 467},
    {"B34B43", 468},
    {"B34B52", 469},
    {"B34B61", 470},
    {"B34B25", 471},
    {"B34B16", 472},
    {"B34B41", 473},
    {"B34B48", 474},
    {"B34B27", 475},
    {"B34B20", 476},
    {"B34B13", 477},
    {"B34B6", 478},
    {"B35B44", 479},
    {"B35B53", 480},
    {"B35B62", 481},
    {"B35B26", 482},
    {"B35B17", 483},
    {"B35B8", 484},
    {"B35B42", 485},
    {"B35B49", 486},
    {"B35B56", 487},
    {"B35B28", 488},
    {"B35B21", 489},
    {"B35B14", 490},
    {"B35B7", 491},
    {"B36B45", 492},
    {"B36B54", 493},
    {"B36B63", 494},
    {"B36B27", 495},
    {"B36B18", 496},
    {"B36B9", 497},
    {"B36B0", 498},
    {"B36B43", 499},
    {"B36B50", 500},
    {"B36B57", 501},
    {"B36B29", 502},
    {"B36B22", 503},
    {"B36B15", 504},
    {"B37B46", 505},
    {"B37B55", 506},
    {"B37B28", 507},
    {"B37B19", 508},
    {"B37B10", 509},
    {"B37B1", 510},
    {"B37B44", 511},
    {"B37B51", 512},
    {"B37B58", 513},
    {"B37B30", 514},
    {"B37B23", 515},
    {"B38B47", 516},
    {"B38B29", 517},
    {"B38B20", 518},
    {"B38B11", 519},
    {"B38B2", 520},
    {"B38B45", 521},
    {"B38B52", 522},
    {"B38B59", 523},
    {"B38B31", 524},
    {"B39B30", 525},
    {"B39B21", 526},
    {"B39B12", 527},
    {"B39B3", 528},
    {"B39B46", 529},
    {"B39B53", 530},
    {"B39B60", 531},
    {"B40B49", 532},
    {"B40B58", 533},
    {"B40B33", 534},
    {"B40B26", 535},
    {"B40B19", 536},
    {"B40B12", 537},
    {"B40B5", 538},
    {"B41B50", 539},
    {"B41B59", 540},
    {"B41B32", 541},
    {"B41B48", 542},
    {"B41B34", 543},
    {"B41B27", 544},
    {"B41B20", 545},
    {"B41B13", 546},
    {"B41B6", 547},
    {"B42B51", 548},
    {"B42B60", 549},
    {"B42B33", 550},
    {"B42B24", 551},
    {"B42B49", 552},
    {"B42B56", 553},
    {"B42B35", 554},
    {"B42B28", 555},
    {"B42B21", 556},
    {"B42B14", 557},
    {"B42B7", 558},
    {"B43B52", 559},
    {"B43B61", 560},
    {"B43B34", 561},
    {"B43B25", 562},
    {"B43B16", 563},
    {"B43B50", 564},
    {"B43B57", 565},
    {"B43B36", 566},
    {"B43B29", 567},
    {"B43B22", 568},
    {"B43B15", 569},
    {"B44B53", 570},
    {"B44B62", 571},
    {"B44B35", 572},
    {"B44B26", 573},
    {"B44B17", 574},
    {"B44B8", 575},
    {"B44B51", 576},
    {"B44B58", 577},
    {"B44B37", 578},
    {"B44B30", 579},
    {"B44B23", 580},
    {"B45B54", 581},
    {"B45B63", 582},
    {"B45B36", 583},
    {"B45B27", 584},
    {"B45B18", 585},
    {"B45B9", 586},
    {"B45B0", 587},
    {"B45B52", 588},
    {"B45B59", 589},
    {"B45B38", 590},
    {"B45B31", 591},
    {"B46B55", 592},
    {"B46B37", 593},
    {"B46B28", 594},
    {"B46B19", 595},
    {"B46B10", 596},
    {"B46B1", 597},
    {"B46B53", 598},
    {"B46B60", 599},
    {"B46B39", 600},
    {"B47B38", 601},
    {"B47B29", 602},
    {"B47B20", 603},
    {"B47B11", 604},
    {"B47B2", 605},
    {"B47B54", 606},
    {"B47B61", 607},
    {"B48B57", 608},
    {"B48B41", 609},
    {"B48B34", 610},
    {"B48B27", 611},
    {"B48B20", 612},
    {"B48B13", 613},
    {"B48B6", 614},
    {"B49B58", 615},
    {"B49B40", 616},
    {"B49B56", 617},
    {"B49B42", 618},
    {"B49B35", 619},
    {"B49B28", 620},
    {"B49B21", 621},
    {"B49B14", 622},
    {"B49B7", 623},
    {"B50B59", 624},
    {"B50B41", 625},
    {"B50B32", 626},
    {"B50B57", 627},
    {"B50B43", 628},
    {"B50B36", 629},
    {"B50B29", 630},
    {"B50B22", 631},
    {"B50B15", 632},
    {"B51B60", 633},
    {"B51B42", 634},
    {"B51B33", 635},
    {"B51B24", 636},
    {"B51B58", 637},
    {"B51B44", 638},
    {"B51B37", 639},
    {"B51B30", 640},
    {"B51B23", 641},
    {"B52B61", 642},
    {"B52B43", 643},
    {"B52B34", 644},
    {"B52B25", 645},
    {"B52B16", 646},
    {"B52B59", 647},
    {"B52B45", 648},
    {"B52B38", 649},
    {"B52B31", 650},
    {"B53B62", 651},
    {"B53B44", 652},
    {"B53B35", 653},
    {"B53B26", 654},
    {"B53B17", 655},
    {"B53B8", 656},
    {"B53B60", 657},
    {"B53B46", 658},
    {"B53B39", 659},
    {"B54B63", 660},
    {"B54B45", 661},
    {"B54B36", 662},
    {"B54B27", 663},
    {"B54B18", 664},
    {"B54B9", 665},
    {"B54B0", 666},
    {"B54B61", 667},
    {"B54B47", 668},
    {"B55B46", 669},
    {"B55B37", 670},
    {"B55B28", 671},
    {"B55B19", 672},
    {"B55B10", 673},
    {"B55B1", 674},
    {"B55B62", 675},
    {"R8R16", 676},
    {"R8R24", 677},
    {"R8R32", 678},
    {"R8R40", 679},
    {"R8R48", 680},
    {"R8R56", 681},
    {"R8R0", 682},
    {"R8R9", 683},
    {"R8R10", 684},
    {"R8R11", 685},
    {"R8R12", 686},
    {"R8R13", 687},
    {"R8R14", 688},
    {"R8R15", 689},
    {"R9R17", 690},
    {"R9R25", 691},
    {"R9R33", 692},
    {"R9R41", 693},
    {"R9R49", 694},
    {"R9R57", 695},
    {"R9R1", 696},
    {"R9R10", 697},
    {"R9R11", 698},
    {"R9R12", 699},
    {"R9R13", 700},
    {"R9R14", 701},
    {"R9R15", 702},
    {"R9R8", 703},
    {"R10R18", 704},
    {"R10R26", 705},
    {"R10R34", 706},
    {"R10R42", 707},
    {"R10R50", 708},
    {"R10R58", 709},
    {"R10R2", 710},
    {"R10R11", 711},
    {"R10R12", 712},
    {"R10R13", 713},
    {"R10R14", 714},
    {"R10R15", 715},
    {"R10R9", 716},
    {"R10R8", 717},
    {"R11R19", 718},
    {"R11R27", 719},
    {"R11R35", 720},
    {"R11R43", 721},
    {"R11R51", 722},
    {"R11R59", 723},
    {"R11R3", 724},
    {"R11R12", 725},
    {"R11R13", 726},
    {"R11R14", 727},
    {"R11R15", 728},
    {"R11R10", 729},
    {"R11R9", 730},
    {"R11R8", 731},
    {"R12R20", 732},
    {"R12R28", 733},
    {"R12R36", 734},
    {"R12R44", 735},
    {"R12R52", 736},
    {"R12R60", 737},
    {"R12R4", 738},
    {"R12R13", 739},
    {"R12R14", 740},
    {"R12R15", 741},
    {"R12R11", 742},
    {"R12R10", 743},
    {"R12R9", 744},
    {"R12R8", 745},
    {"R13R21", 746},
    {"R13R29", 747},
    {"R13R37", 748},
    {"R13R45", 749},
    {"R13R53", 750},
    {"R13R61", 751},
    {"R13R5", 752},
    {"R13R14", 753},
    {"R13R15", 754},
    {"R13R12", 755},
    {"R13R11", 756},
    {"R13R10", 757},
    {"R13R9", 758},
    {"R13R8", 759},
    {"R14R22", 760},
    {"R14R30", 761},
    {"R14R38", 762},
    {"R14R46", 763},
    {"R14R54", 764},
    {"R14R62", 765},
    {"R14R6", 766},
    {"R14R15", 767},
    {"R14R13", 768},
    {"R14R12", 769},
    {"R14R11", 770},
    {"R14R10", 771},
    {"R14R9", 772},
    {"R14R8", 773},
    {"R15R23", 774},
    {"R15R31", 775},
    {"R15R39", 776},
    {"R15R47", 777},
    {"R15R55", 778},
    {"R15R63", 779},
    {"R15R7", 780},
    {"R15R14", 781},
    {"R15R13", 782},
    {"R15R12", 783},
    {"R15R11", 784},
    {"R15R10", 785},
    {"R15R9", 786},
    {"R15R8", 787},
    {"R16R24", 788},
    {"R16R32", 789},
    {"R16R40", 790},
    {"R16R48", 791},
    {"R16R56", 792},
    {"R16R8", 793},
    {"R16R0", 794},
    {"R16R17", 795},
    {"R16R18", 796},
    {"R16R19", 797},
    {"R16R20", 798},
    {"R16R21", 799},
    {"R16R22", 800},
    {"R16R23", 801},
    {"R17R25", 802},
    {"R17R33", 803},
    {"R17R41", 804},
    {"R17R49", 805},
    {"R17R57", 806},
    {"R17R9", 807},
    {"R17R1", 808},
    {"R17R18", 809},
    {"R17R19", 810},
    {"R17R20", 811},
    {"R17R21", 812},
    {"R17R22", 813},
    {"R17R23", 814},
    {"R17R16", 815},
    {"R18R26", 816},
    {"R18R34", 817},
    {"R18R42", 818},
    {"R18R50", 819},
    {"R18R58", 820},
    {"R18R10", 821},
    {"R18R2", 822},
    {"R18R19", 823},
    {"R18R20", 824},
    {"R18R21", 825},
    {"R18R22", 826},
    {"R18R23", 827},
    {"R18R17", 828},
    {"R18R16", 829},
    {"R19R27", 830},
    {"R19R35", 831},
    {"R19R43", 832},
    {"R19R51", 833},
    {"R19R59", 834},
    {"R19R11", 835},
    {"R19R3", 836},
    {"R19R20", 837},
    {"R19R21", 838},
    {"R19R22", 839},
    {"R19R23", 840},
    {"R19R18", 841},
    {"R19R17", 842},
    {"R19R16", 843},
    {"R20R28", 844},
    {"R20R36", 845},
    {"R20R44", 846},
    {"R20R52", 847},
    {"R20R60", 848},
    {"R20R12", 849},
    {"R20R4", 850},
    {"R20R21", 851},
    {"R20R22", 852},
    {"R20R23", 853},
    {"R20R19", 854},
    {"R20R18", 855},
    {"R20R17", 856},
    {"R20R16", 857},
    {"R21R29", 858},
    {"R21R37", 859},
    {"R21R45", 860},
    {"R21R53", 861},
    {"R21R61", 862},
    {"R21R13", 863},
    {"R21R5", 864},
    {"R21R22", 865},
    {"R21R23", 866},
    {"R21R20", 867},
    {"R21R19", 868},
    {"R21R18", 869},
    {"R21R17", 870},
    {"R21R16", 871},
    {"R22R30", 872},
    {"R22R38", 873},
    {"R22R46", 874},
    {"R22R54", 875},
    {"R22R62", 876},
    {"R22R14", 877},
    {"R22R6", 878},
    {"R22R23", 879},
    {"R22R21", 880},
    {"R22R20", 881},
    {"R22R19", 882},
    {"R22R18", 883},
    {"R22R17", 884},
    {"R22R16", 885},
    {"R23R31", 886},
    {"R23R39", 887},
    {"R23R47", 888},
    {"R23R55", 889},
    {"R23R63", 890},
    {"R23R15", 891},
    {"R23R7", 892},
    {"R23R22", 893},
    {"R23R21", 894},
    {"R23R20", 895},
    {"R23R19", 896},
    {"R23R18", 897},
    {"R23R17", 898},
    {"R23R16", 899},
    {"R24R32", 900},
    {"R24R40", 901},
    {"R24R48", 902},
    {"R24R56", 903},
    {"R24R16", 904},
    {"R24R8", 905},
    {"R24R0", 906},
    {"R24R25", 907},
    {"R24R26", 908},
    {"R24R27", 909},
    {"R24R28", 910},
    {"R24R29", 911},
    {"R24R30", 912},
    {"R24R31", 913},
    {"R25R33", 914},
    {"R25R41", 915},
    {"R25R49", 916},
    {"R25R57", 917},
    {"R25R17", 918},
    {"R25R9", 919},
    {"R25R1", 920},
    {"R25R26", 921},
    {"R25R27", 922},
    {"R25R28", 923},
    {"R25R29", 924},
    {"R25R30", 925},
    {"R25R31", 926},
    {"R25R24", 927},
    {"R26R34", 928},
    {"R26R42", 929},
    {"R26R50", 930},
    {"R26R58", 931},
    {"R26R18", 932},
    {"R26R10", 933},
    {"R26R2", 934},
    {"R26R27", 935},
    {"R26R28", 936},
    {"R26R29", 937},
    {"R26R30", 938},
    {"R26R31", 939},
    {"R26R25", 940},
    {"R26R24", 941},
    {"R27R35", 942},
    {"R27R43", 943},
    {"R27R51", 944},
    {"R27R59", 945},
    {"R27R19", 946},
    {"R27R11", 947},
    {"R27R3", 948},
    {"R27R28", 949},
    {"R27R29", 950},
    {"R27R30", 951},
    {"R27R31", 952},
    {"R27R26", 953},
    {"R27R25", 954},
    {"R27R24", 955},
    {"R28R36", 956},
    {"R28R44", 957},
    {"R28R52", 958},
    {"R28R60", 959},
    {"R28R20", 960},
    {"R28R12", 961},
    {"R28R4", 962},
    {"R28R29", 963},
    {"R28R30", 964},
    {"R28R31", 965},
    {"R28R27", 966},
    {"R28R26", 967},
    {"R28R25", 968},
    {"R28R24", 969},
    {"R29R37", 970},
    {"R29R45", 971},
    {"R29R53", 972},
    {"R29R61", 973},
    {"R29R21", 974},
    {"R29R13", 975},
    {"R29R5", 976},
    {"R29R30", 977},
    {"R29R31", 978},
    {"R29R28", 979},
    {"R29R27", 980},
    {"R29R26", 981},
    {"R29R25", 982},
    {"R29R24", 983},
    {"R30R38", 984},
    {"R30R46", 985},
    {"R30R54", 986},
    {"R30R62", 987},
    {"R30R22", 988},
    {"R30R14", 989},
    {"R30R6", 990},
    {"R30R31", 991},
    {"R30R29", 992},
    {"R30R28", 993},
    {"R30R27", 994},
    {"R30R26", 995},
    {"R30R25", 996},
    {"R30R24", 997},
    {"R31R39", 998},
    {"R31R47", 999},
    {"R31R55", 1000},
    {"R31R63", 1001},
    {"R31R23", 1002},
    {"R31R15", 1003},
    {"R31R7", 1004},
    {"R31R30", 1005},
    {"R31R29", 1006},
    {"R31R28", 1007},
    {"R31R27", 1008},
    {"R31R26", 1009},
    {"R31R25", 1010},
    {"R31R24", 1011},
    {"R32R40", 1012},
    {"R32R48", 1013},
    {"R32R56", 1014},
    {"R32R24", 1015},
    {"R32R16", 1016},
    {"R32R8", 1017},
    {"R32R0", 1018},
    {"R32R33", 1019},
    {"R32R34", 1020},
    {"R32R35", 1021},
    {"R32R36", 1022},
    {"R32R37", 1023},
    {"R32R38", 1024},
    {"R32R39", 1025},
    {"R33R41", 1026},
    {"R33R49", 1027},
    {"R33R57", 1028},
    {"R33R25", 1029},
    {"R33R17", 1030},
    {"R33R9", 1031},
    {"R33R1", 1032},
    {"R33R34", 1033},
    {"R33R35", 1034},
    {"R33R36", 1035},
    {"R33R37", 1036},
    {"R33R38", 1037},
    {"R33R39", 1038},
    {"R33R32", 1039},
    {"R34R42", 1040},
    {"R34R50", 1041},
    {"R34R58", 1042},
    {"R34R26", 1043},
    {"R34R18", 1044},
    {"R34R10", 1045},
    {"R34R2", 1046},
    {"R34R35", 1047},
    {"R34R36", 1048},
    {"R34R37", 1049},
    {"R34R38", 1050},
    {"R34R39", 1051},
    {"R34R33", 1052},
    {"R34R32", 1053},
    {"R35R43", 1054},
    {"R35R51", 1055},
    {"R35R59", 1056},
    {"R35R27", 1057},
    {"R35R19", 1058},
    {"R35R11", 1059},
    {"R35R3", 1060},
    {"R35R36", 1061},
    {"R35R37", 1062},
    {"R35R38", 1063},
    {"R35R39", 1064},
    {"R35R34", 1065},
    {"R35R33", 1066},
    {"R35R32", 1067},
    {"R36R44", 1068},
    {"R36R52", 1069},
    {"R36R60", 1070},
    {"R36R28", 1071},
    {"R36R20", 1072},
    {"R36R12", 1073},
    {"R36R4", 1074},
    {"R36R37", 1075},
    {"R36R38", 1076},
    {"R36R39", 1077},
    {"R36R35", 1078},
    {"R36R34", 1079},
    {"R36R33", 1080},
    {"R36R32", 1081},
    {"R37R45", 1082},
    {"R37R53", 1083},
    {"R37R61", 1084},
    {"R37R29", 1085},
    {"R37R21", 1086},
    {"R37R13", 1087},
    {"R37R5", 1088},
    {"R37R38", 1089},
    {"R37R39", 1090},
    {"R37R36", 1091},
    {"R37R35", 1092},
    {"R37R34", 1093},
    {"R37R33", 1094},
    {"R37R32", 1095},
    {"R38R46", 1096},
    {"R38R54", 1097},
    {"R38R62", 1098},
    {"R38R30", 1099},
    {"R38R22", 1100},
    {"R38R14", 1101},
    {"R38R6", 1102},
    {"R38R39", 1103},
    {"R38R37", 1104},
    {"R38R36", 1105},
    {"R38R35", 1106},
    {"R38R34", 1107},
    {"R38R33", 1108},
    {"R38R32", 1109},
    {"R39R47", 1110},
    {"R39R55", 1111},
    {"R39R63", 1112},
    {"R39R31", 1113},
    {"R39R23", 1114},
    {"R39R15", 1115},
    {"R39R7", 1116},
    {"R39R38", 1117},
    {"R39R37", 1118},
    {"R39R36", 1119},
    {"R39R35", 1120},
    {"R39R34", 1121},
    {"R39R33", 1122},
    {"R39R32", 1123},
    {"R40R48", 1124},
    {"R40R56", 1125},
    {"R40R32", 1126},
    {"R40R24", 1127},
    {"R40R16", 1128},
    {"R40R8", 1129},
    {"R40R0", 1130},
    {"R40R41", 1131},
    {"R40R42", 1132},
    {"R40R43", 1133},
    {"R40R44", 1134},
    {"R40R45", 1135},
    {"R40R46", 1136},
    {"R40R47", 1137},
    {"R41R49", 1138},
    {"R41R57", 1139},
    {"R41R33", 1140},
    {"R41R25", 1141},
    {"R41R17", 1142},
    {"R41R9", 1143},
    {"R41R1", 1144},
    {"R41R42", 1145},
    {"R41R43", 1146},
    {"R41R44", 1147},
    {"R41R45", 1148},
    {"R41R46", 1149},
    {"R41R47", 1150},
    {"R41R40", 1151},
    {"R42R50", 1152},
    {"R42R58", 1153},
    {"R42R34", 1154},
    {"R42R26", 1155},
    {"R42R18", 1156},
    {"R42R10", 1157},
    {"R42R2", 1158},
    {"R42R43", 1159},
    {"R42R44", 1160},
    {"R42R45", 1161},
    {"R42R46", 1162},
    {"R42R47", 1163},
    {"R42R41", 1164},
    {"R42R40", 1165},
    {"R43R51", 1166},
    {"R43R59", 1167},
    {"R43R35", 1168},
    {"R43R27", 1169},
    {"R43R19", 1170},
    {"R43R11", 1171},
    {"R43R3", 1172},
    {"R43R44", 1173},
    {"R43R45", 1174},
    {"R43R46", 1175},
    {"R43R47", 1176},
    {"R43R42", 1177},
    {"R43R41", 1178},
    {"R43R40", 1179},
    {"R44R52", 1180},
    {"R44R60", 1181},
    {"R44R36", 1182},
    {"R44R28", 1183},
    {"R44R20", 1184},
    {"R44R12", 1185},
    {"R44R4", 1186},
    {"R44R45", 1187},
    {"R44R46", 1188},
    {"R44R47", 1189},
    {"R44R43", 1190},
    {"R44R42", 1191},
    {"R44R41", 1192},
    {"R44R40", 1193},
    {"R45R53", 1194},
    {"R45R61", 1195},
    {"R45R37", 1196},
    {"R45R29", 1197},
    {"R45R21", 1198},
    {"R45R13", 1199},
    {"R45R5", 1200},
    {"R45R46", 1201},
    {"R45R47", 1202},
    {"R45R44", 1203},
    {"R45R43", 1204},
    {"R45R42", 1205},
    {"R45R41", 1206},
    {"R45R40", 1207},
    {"R46R54", 1208},
    {"R46R62", 1209},
    {"R46R38", 1210},
    {"R46R30", 1211},
    {"R46R22", 1212},
    {"R46R14", 1213},
    {"R46R6", 1214},
    {"R46R47", 1215},
    {"R46R45", 1216},
    {"R46R44", 1217},
    {"R46R43", 1218},
    {"R46R42", 1219},
    {"R46R41", 1220},
    {"R46R40", 1221},
    {"R47R55", 1222},
    {"R47R63", 1223},
    {"R47R39", 1224},
    {"R47R31", 1225},
    {"R47R23", 1226},
    {"R47R15", 1227},
    {"R47R7", 1228},
    {"R47R46", 1229},
    {"R47R45", 1230},
    {"R47R44", 1231},
    {"R47R43", 1232},
    {"R47R42", 1233},
    {"R47R41", 1234},
    {"R47R40", 1235},
    {"R48R56", 1236},
    {"R48R40", 1237},
    {"R48R32", 1238},
    {"R48R24", 1239},
    {"R48R16", 1240},
    {"R48R8", 1241},
    {"R48R0", 1242},
    {"R48R49", 1243},
    {"R48R50", 1244},
    {"R48R51", 1245},
    {"R48R52", 1246},
    {"R48R53", 1247},
    {"R48R54", 1248},
    {"R48R55", 1249},
    {"R49R57", 1250},
    {"R49R41", 1251},
    {"R49R33", 1252},
    {"R49R25", 1253},
    {"R49R17", 1254},
    {"R49R9", 1255},
    {"R49R1", 1256},
    {"R49R50", 1257},
    {"R49R51", 1258},
    {"R49R52", 1259},
    {"R49R53", 1260},
    {"R49R54", 1261},
    {"R49R55", 1262},
    {"R49R48", 1263},
    {"R50R58", 1264},
    {"R50R42", 1265},
    {"R50R34", 1266},
    {"R50R26", 1267},
    {"R50R18", 1268},
    {"R50R10", 1269},
    {"R50R2", 1270},
    {"R50R51", 1271},
    {"R50R52", 1272},
    {"R50R53", 1273},
    {"R50R54", 1274},
    {"R50R55", 1275},
    {"R50R49", 1276},
    {"R50R48", 1277},
    {"R51R59", 1278},
    {"R51R43", 1279},
    {"R51R35", 1280},
    {"R51R27", 1281},
    {"R51R19", 1282},
    {"R51R11", 1283},
    {"R51R3", 1284},
    {"R51R52", 1285},
    {"R51R53", 1286},
    {"R51R54", 1287},
    {"R51R55", 1288},
    {"R51R50", 1289},
    {"R51R49", 1290},
    {"R51R48", 1291},
    {"R52R60", 1292},
    {"R52R44", 1293},
    {"R52R36", 1294},
    {"R52R28", 1295},
    {"R52R20", 1296},
    {"R52R12", 1297},
    {"R52R4", 1298},
    {"R52R53", 1299},
    {"R52R54", 1300},
    {"R52R55", 1301},
    {"R52R51", 1302},
    {"R52R50", 1303},
    {"R52R49", 1304},
    {"R52R48", 1305},
    {"R53R61", 1306},
    {"R53R45", 1307},
    {"R53R37", 1308},
    {"R53R29", 1309},
    {"R53R21", 1310},
    {"R53R13", 1311},
    {"R53R5", 1312},
    {"R53R54", 1313},
    {"R53R55", 1314},
    {"R53R52", 1315},
    {"R53R51", 1316},
    {"R53R50", 1317},
    {"R53R49", 1318},
    {"R53R48", 1319},
    {"R54R62", 1320},
    {"R54R46", 1321},
    {"R54R38", 1322},
    {"R54R30", 1323},
    {"R54R22", 1324},
    {"R54R14", 1325},
    {"R54R6", 1326},
    {"R54R55", 1327},
    {"R54R53", 1328},
    {"R54R52", 1329},
    {"R54R51", 1330},
    {"R54R50", 1331},
    {"R54R49", 1332},
    {"R54R48", 1333},
    {"R55R63", 1334},
    {"R55R47", 1335},
    {"R55R39", 1336},
    {"R55R31", 1337},
    {"R55R23", 1338},
    {"R55R15", 1339},
    {"R55R7", 1340},
    {"R55R54", 1341},
    {"R55R53", 1342},
    {"R55R52", 1343},
    {"R55R51", 1344},
    {"R55R50", 1345},
    {"R55R49", 1346},
    {"R55R48", 1347},
    {"Q8Q17", 1348},
    {"Q8Q26", 1349},
    {"Q8Q35", 1350},
    {"Q8Q44", 1351},
    {"Q8Q53", 1352},
    {"Q8Q62", 1353},
    {"Q8Q1", 1354},
    {"Q8Q16", 1355},
    {"Q8Q24", 1356},
    {"Q8Q32", 1357},
    {"Q8Q40", 1358},
    {"Q8Q48", 1359},
    {"Q8Q56", 1360},
    {"Q8Q0", 1361},
    {"Q8Q9", 1362},
    {"Q8Q10", 1363},
    {"Q8Q11", 1364},
    {"Q8Q12", 1365},
    {"Q8Q13", 1366},
    {"Q8Q14", 1367},
    {"Q8Q15", 1368},
    {"Q9Q18", 1369},
    {"Q9Q27", 1370},
    {"Q9Q36", 1371},
    {"Q9Q45", 1372},
    {"Q9Q54", 1373},
    {"Q9Q63", 1374},
    {"Q9Q0", 1375},
    {"Q9Q16", 1376},
    {"Q9Q2", 1377},
    {"Q9Q17", 1378},
    {"Q9Q25", 1379},
    {"Q9Q33", 1380},
    {"Q9Q41", 1381},
    {"Q9Q49", 1382},
    {"Q9Q57", 1383},
    {"Q9Q1", 1384},
    {"Q9Q10", 1385},
    {"Q9Q11", 1386},
    {"Q9Q12", 1387},
    {"Q9Q13", 1388},
    {"Q9Q14", 1389},
    {"Q9Q15", 1390},
    {"Q9Q8", 1391},
    {"Q10Q19", 1392},
    {"Q10Q28", 1393},
    {"Q10Q37", 1394},
    {"Q10Q46", 1395},
    {"Q10Q55", 1396},
    {"Q10Q1", 1397},
    {"Q10Q17", 1398},
    {"Q10Q24", 1399},
    {"Q10Q3", 1400},
    {"Q10Q18", 1401},
    {"Q10Q26", 1402},
    {"Q10Q34", 1403},
    {"Q10Q42", 1404},
    {"Q10Q50", 1405},
    {"Q10Q58", 1406},
    {"Q10Q2", 1407},
    {"Q10Q11", 1408},
    {"Q10Q12", 1409},
    {"Q10Q13", 1410},
    {"Q10Q14", 1411},
    {"Q10Q15", 1412},
    {"Q10Q9", 1413},
    {"Q10Q8", 1414},
    {"Q11Q20", 1415},
    {"Q11Q29", 1416},
    {"Q11Q38", 1417},
    {"Q11Q47", 1418},
    {"Q11Q2", 1419},
    {"Q11Q18", 1420},
    {"Q11Q25", 1421},
    {"Q11Q32", 1422},
    {"Q11Q4", 1423},
    {"Q11Q19", 1424},
    {"Q11Q27", 1425},
    {"Q11Q35", 1426},
    {"Q11Q43", 1427},
    {"Q11Q51", 1428},
    {"Q11Q59", 1429},
    {"Q11Q3", 1430},
    {"Q11Q12", 1431},
    {"Q11Q13", 1432},
    {"Q11Q14", 1433},
    {"Q11Q15", 1434},
    {"Q11Q10", 1435},
    {"Q11Q9", 1436},
    {"Q11Q8", 1437},
    {"Q12Q21", 1438},
    {"Q12Q30", 1439},
    {"Q12Q39", 1440},
    {"Q12Q3", 1441},
    {"Q12Q19", 1442},
    {"Q12Q26", 1443},
    {"Q12Q33", 1444},
    {"Q12Q40", 1445},
    {"Q12Q5", 1446},
    {"Q12Q20", 1447},
    {"Q12Q28", 1448},
    {"Q12Q36", 1449},
    {"Q12Q44", 1450},
    {"Q12Q52", 1451},
    {"Q12Q60", 1452},
    {"Q12Q4", 1453},
    {"Q12Q13", 1454},
    {"Q12Q14", 1455},
    {"Q12Q15", 1456},
    {"Q12Q11", 1457},
    {"Q12Q10", 1458},
    {"Q12Q9", 1459},
    {"Q12Q8", 1460},
    {"Q13Q22", 1461},
    {"Q13Q31", 1462},
    {"Q13Q4", 1463},
    {"Q13Q20", 1464},
    {"Q13Q27", 1465},
    {"Q13Q34", 1466},
    {"Q13Q41", 1467},
    {"Q13Q48", 1468},
    {"Q13Q6", 1469},
    {"Q13Q21", 1470},
    {"Q13Q29", 1471},
    {"Q13Q37", 1472},
    {"Q13Q45", 1473},
    {"Q13Q53", 1474},
    {"Q13Q61", 1475},
    {"Q13Q5", 1476},
    {"Q13Q14", 1477},
    {"Q13Q15", 1478},
    {"Q13Q12", 1479},
    {"Q13Q11", 1480},
    {"Q13Q10", 1481},
    {"Q13Q9", 1482},
    {"Q13Q8", 1483},
    {"Q14Q23", 1484},
    {"Q14Q5", 1485},
    {"Q14Q21", 1486},
    {"Q14Q28", 1487},
    {"Q14Q35", 1488},
    {"Q14Q42", 1489},
    {"Q14Q49", 1490},
    {"Q14Q56", 1491},
    {"Q14Q7", 1492},
    {"Q14Q22", 1493},
    {"Q14Q30", 1494},
    {"Q14Q38", 1495},
    {"Q14Q46", 1496},
    {"Q14Q54", 1497},
    {"Q14Q62", 1498},
    {"Q14Q6", 1499},
    {"Q14Q15", 1500},
    {"Q14Q13", 1501},
    {"Q14Q12", 1502},
    {"Q14Q11", 1503},
    {"Q14Q10", 1504},
    {"Q14Q9", 1505},
    {"Q14Q8", 1506},
    {"Q15Q6", 1507},
    {"Q15Q22", 1508},
    {"Q15Q29", 1509},
    {"Q15Q36", 1510},
    {"Q15Q43", 1511},
    {"Q15Q50", 1512},
    {"Q15Q57", 1513},
    {"Q15Q23", 1514},
    {"Q15Q31", 1515},
    {"Q15Q39", 1516},
    {"Q15Q47", 1517},
    {"Q15Q55", 1518},
    {"Q15Q63", 1519},
    {"Q15Q7", 1520},
    {"Q15Q14", 1521},
    {"Q15Q13", 1522},
    {"Q15Q12", 1523},
    {"Q15Q11", 1524},
    {"Q15Q10", 1525},
    {"Q15Q9", 1526},
    {"Q15Q8", 1527},
    {"Q16Q25", 1528},
    {"Q16Q34", 1529},
    {"Q16Q43", 1530},
    {"Q16Q52", 1531},
    {"Q16Q61", 1532},
    {"Q16Q9", 1533},
    {"Q16Q2", 1534},
    {"Q16Q24", 1535},
    {"Q16Q32", 1536},
    {"Q16Q40", 1537},
    {"Q16Q48", 1538},
    {"Q16Q56", 1539},
    {"Q16Q8", 1540},
    {"Q16Q0", 1541},
    {"Q16Q17", 1542},
    {"Q16Q18", 1543},
    {"Q16Q19", 1544},
    {"Q16Q20", 1545},
    {"Q16Q21", 1546},
    {"Q16Q22", 1547},
    {"Q16Q23", 1548},
    {"Q17Q26", 1549},
    {"Q17Q35", 1550},
    {"Q17Q44", 1551},
    {"Q17Q53", 1552},
    {"Q17Q62", 1553},
    {"Q17Q8", 1554},
    {"Q17Q24", 1555},
    {"Q17Q10", 1556},
    {"Q17Q3", 1557},
    {"Q17Q25", 1558},
    {"Q17Q33", 1559},
    {"Q17Q41", 1560},
    {"Q17Q49", 1561},
    {"Q17Q57", 1562},
    {"Q17Q9", 1563},
    {"Q17Q1", 1564},
    {"Q17Q18", 1565},
    {"Q17Q19", 1566},
    {"Q17Q20", 1567},
    {"Q17Q21", 1568},
    {"Q17Q22", 1569},
    {"Q17Q23", 1570},
    {"Q17Q16", 1571},
    {"Q18Q27", 1572},
    {"Q18Q36", 1573},
    {"Q18Q45", 1574},
    {"Q18Q54", 1575},
    {"Q18Q63", 1576},
    {"Q18Q9", 1577},
    {"Q18Q0", 1578},
    {"Q18Q25", 1579},
    {"Q18Q32", 1580},
    {"Q18Q11", 1581},
    {"Q18Q4", 1582},
    {"Q18Q26", 1583},
    {"Q18Q34", 1584},
    {"Q18Q42", 1585},
    {"Q18Q50", 1586},
    {"Q18Q58", 1587},
    {"Q18Q10", 1588},
    {"Q18Q2", 1589},
    {"Q18Q19", 1590},
    {"Q18Q20", 1591},
    {"Q18Q21", 1592},
    {"Q18Q22", 1593},
    {"Q18Q23", 1594},
    {"Q18Q17", 1595},
    {"Q18Q16", 1596},
    {"Q19Q28", 1597},
    {"Q19Q37", 1598},
    {"Q19Q46", 1599},
    {"Q19Q55", 1600},
    {"Q19Q10", 1601},
    {"Q19Q1", 1602},
    {"Q19Q26", 1603},
    {"Q19Q33", 1604},
    {"Q19Q40", 1605},
    {"Q19Q12", 1606},
    {"Q19Q5", 1607},
    {"Q19Q27", 1608},
    {"Q19Q35", 1609},
    {"Q19Q43", 1610},
    {"Q19Q51", 1611},
    {"Q19Q59", 1612},
    {"Q19Q11", 1613},
    {"Q19Q3", 1614},
    {"Q19Q20", 1615},
    {"Q19Q21", 1616},
    {"Q19Q22", 1617},
    {"Q19Q23", 1618},
    {"Q19Q18", 1619},
    {"Q19Q17", 1620},
    {"Q19Q16", 1621},
    {"Q20Q29", 1622},
    {"Q20Q38", 1623},
    {"Q20Q47", 1624},
    {"Q20Q11", 1625},
    {"Q20Q2", 1626},
    {"Q20Q27", 1627},
    {"Q20Q34", 1628},
    {"Q20Q41", 1629},
    {"Q20Q48", 1630},
    {"Q20Q13", 1631},
    {"Q20Q6", 1632},
    {"Q20Q28", 1633},
    {"Q20Q36", 1634},
    {"Q20Q44", 1635},
    {"Q20Q52", 1636},
    {"Q20Q60", 1637},
    {"Q20Q12", 1638},
    {"Q20Q4", 1639},
    {"Q20Q21", 1640},
    {"Q20Q22", 1641},
    {"Q20Q23", 1642},
    {"Q20Q19", 1643},
    {"Q20Q18", 1644},
    {"Q20Q17", 1645},
    {"Q20Q16", 1646},
    {"Q21Q30", 1647},
    {"Q21Q39", 1648},
    {"Q21Q12", 1649},
    {"Q21Q3", 1650},
    {"Q21Q28", 1651},
    {"Q21Q35", 1652},
    {"Q21Q42", 1653},
    {"Q21Q49", 1654},
    {"Q21Q56", 1655},
    {"Q21Q14", 1656},
    {"Q21Q7", 1657},
    {"Q21Q29", 1658},
    {"Q21Q37", 1659},
    {"Q21Q45", 1660},
    {"Q21Q53", 1661},
    {"Q21Q61", 1662},
    {"Q21Q13", 1663},
    {"Q21Q5", 1664},
    {"Q21Q22", 1665},
    {"Q21Q23", 1666},
    {"Q21Q20", 1667},
    {"Q21Q19", 1668},
    {"Q21Q18", 1669},
    {"Q21Q17", 1670},
    {"Q21Q16", 1671},
    {"Q22Q31", 1672},
    {"Q22Q13", 1673},
    {"Q22Q4", 1674},
    {"Q22Q29", 1675},
    {"Q22Q36", 1676},
    {"Q22Q43", 1677},
    {"Q22Q50", 1678},
    {"Q22Q57", 1679},
    {"Q22Q15", 1680},
    {"Q22Q30", 1681},
    {"Q22Q38", 1682},
    {"Q22Q46", 1683},
    {"Q22Q54", 1684},
    {"Q22Q62", 1685},
    {"Q22Q14", 1686},
    {"Q22Q6", 1687},
    {"Q22Q23", 1688},
    {"Q22Q21", 1689},
    {"Q22Q20", 1690},
    {"Q22Q19", 1691},
    {"Q22Q18", 1692},
    {"Q22Q17", 1693},
    {"Q22Q16", 1694},
    {"Q23Q14", 1695},
    {"Q23Q5", 1696},
    {"Q23Q30", 1697},
    {"Q23Q37", 1698},
    {"Q23Q44", 1699},
    {"Q23Q51", 1700},
    {"Q23Q58", 1701},
    {"Q23Q31", 1702},
    {"Q23Q39", 1703},
    {"Q23Q47", 1704},
    {"Q23Q55", 1705},
    {"Q23Q63", 1706},
    {"Q23Q15", 1707},
    {"Q23Q7", 1708},
    {"Q23Q22", 1709},
    {"Q23Q21", 1710},
    {"Q23Q20", 1711},
    {"Q23Q19", 1712},
    {"Q23Q18", 1713},
    {"Q23Q17", 1714},
    {"Q23Q16", 1715},
    {"Q24Q33", 1716},
    {"Q24Q42", 1717},
    {"Q24Q51", 1718},
    {"Q24Q60", 1719},
    {"Q24Q17", 1720},
    {"Q24Q10", 1721},
    {"Q24Q3", 1722},
    {"Q24Q32", 1723},
    {"Q24Q40", 1724},
    {"Q24Q48", 1725},
    {"Q24Q56", 1726},
    {"Q24Q16", 1727},
    {"Q24Q8", 1728},
    {"Q24Q0", 1729},
    {"Q24Q25", 1730},
    {"Q24Q26", 1731},
    {"Q24Q27", 1732},
    {"Q24Q28", 1733},
    {"Q24Q29", 1734},
    {"Q24Q30", 1735},
    {"Q24Q31", 1736},
    {"Q25Q34", 1737},
    {"Q25Q43", 1738},
    {"Q25Q52", 1739},
    {"Q25Q61", 1740},
    {"Q25Q16", 1741},
    {"Q25Q32", 1742},
    {"Q25Q18", 1743},
    {"Q25Q11", 1744},
    {"Q25Q4", 1745},
    {"Q25Q33", 1746},
    {"Q25Q41", 1747},
    {"Q25Q49", 1748},
    {"Q25Q57", 1749},
    {"Q25Q17", 1750},
    {"Q25Q9", 1751},
    {"Q25Q1", 1752},
    {"Q25Q26", 1753},
    {"Q25Q27", 1754},
    {"Q25Q28", 1755},
    {"Q25Q29", 1756},
    {"Q25Q30", 1757},
    {"Q25Q31", 1758},
    {"Q25Q24", 1759},
    {"Q26Q35", 1760},
    {"Q26Q44", 1761},
    {"Q26Q53", 1762},
    {"Q26Q62", 1763},
    {"Q26Q17", 1764},
    {"Q26Q8", 1765},
    {"Q26Q33", 1766},
    {"Q26Q40", 1767},
    {"Q26Q19", 1768},
    {"Q26Q12", 1769},
    {"Q26Q5", 1770},
    {"Q26Q34", 1771},
    {"Q26Q42", 1772},
    {"Q26Q50", 1773},
    {"Q26Q58", 1774},
    {"Q26Q18", 1775},
    {"Q26Q10", 1776},
    {"Q26Q2", 1777},
    {"Q26Q27", 1778},
    {"Q26Q28", 1779},
    {"Q26Q29", 1780},
    {"Q26Q30", 1781},
    {"Q26Q31", 1782},
    {"Q26Q25", 1783},
    {"Q26Q24", 1784},
    {"Q27Q36", 1785},
    {"Q27Q45", 1786},
    {"Q27Q54", 1787},
    {"Q27Q63", 1788},
    {"Q27Q18", 1789},
    {"Q27Q9", 1790},
    {"Q27Q0", 1791},
    {"Q27Q34", 1792},
    {"Q27Q41", 1793},
    {"Q27Q48", 1794},
    {"Q27Q20", 1795},
    {"Q27Q13", 1796},
    {"Q27Q6", 1797},
    {"Q27Q35", 1798},
    {"Q27Q43", 1799},
    {"Q27Q51", 1800},
    {"Q27Q59", 1801},
    {"Q27Q19", 1802},
    {"Q27Q11", 1803},
    {"Q27Q3", 1804},
    {"Q27Q28", 1805},
    {"Q27Q29", 1806},
    {"Q27Q30", 1807},
    {"Q27Q31", 1808},
    {"Q27Q26", 1809},
    {"Q27Q25", 1810},
    {"Q27Q24", 1811},
    {"Q28Q37", 1812},
    {"Q28Q46", 1813},
    {"Q28Q55", 1814},
    {"Q28Q19", 1815},
    {"Q28Q10", 1816},
    {"Q28Q1", 1817},
    {"Q28Q35", 1818},
    {"Q28Q42", 1819},
    {"Q28Q49", 1820},
    {"Q28Q56", 1821},
    {"Q28Q21", 1822},
    {"Q28Q14", 1823},
    {"Q28Q7", 1824},
    {"Q28Q36", 1825},
    {"Q28Q44", 1826},
    {"Q28Q52", 1827},
    {"Q28Q60", 1828},
    {"Q28Q20", 1829},
    {"Q28Q12", 1830},
    {"Q28Q4", 1831},
    {"Q28Q29", 1832},
    {"Q28Q30", 1833},
    {"Q28Q31", 1834},
    {"Q28Q27", 1835},
    {"Q28Q26", 1836},
    {"Q28Q25", 1837},
    {"Q28Q24", 1838},
    {"Q29Q38", 1839},
    {"Q29Q47", 1840},
    {"Q29Q20", 1841},
    {"Q29Q11", 1842},
    {"Q29Q2", 1843},
    {"Q29Q36", 1844},
    {"Q29Q43", 1845},
    {"Q29Q50", 1846},
    {"Q29Q57", 1847},
    {"Q29Q22", 1848},
    {"Q29Q15", 1849},
    {"Q29Q37", 1850},
    {"Q29Q45", 1851},
    {"Q29Q53", 1852},
    {"Q29Q61", 1853},
    {"Q29Q21", 1854},
    {"Q29Q13", 1855},
    {"Q29Q5", 1856},
    {"Q29Q30", 1857},
    {"Q29Q31", 1858},
    {"Q29Q28", 1859},
    {"Q29Q27", 1860},
    {"Q29Q26", 1861},
    {"Q29Q25", 1862},
    {"Q29Q24", 1863},
    {"Q30Q39", 1864},
    {"Q30Q21", 1865},
    {"Q30Q12", 1866},
    {"Q30Q3", 1867},
    {"Q30Q37", 1868},
    {"Q30Q44", 1869},
    {"Q30Q51", 1870},
    {"Q30Q58", 1871},
    {"Q30Q23", 1872},
    {"Q30Q38", 1873},
    {"Q30Q46", 1874},
    {"Q30Q54", 1875},
    {"Q30Q62", 1876},
    {"Q30Q22", 1877},
    {"Q30Q14", 1878},
    {"Q30Q6", 1879},
    {"Q30Q31", 1880},
    {"Q30Q29", 1881},
    {"Q30Q28", 1882},
    {"Q30Q27", 1883},
    {"Q30Q26", 1884},
    {"Q30Q25", 1885},
    {"Q30Q24", 1886},
    {"Q31Q22", 1887},
    {"Q31Q13", 1888},
    {"Q31Q4", 1889},
    {"Q31Q38", 1890},
    {"Q31Q45", 1891},
    {"Q31Q52", 1892},
    {"Q31Q59", 1893},
    {"Q31Q39", 1894},
    {"Q31Q47", 1895},
    {"Q31Q55", 1896},
    {"Q31Q63", 1897},
    {"Q31Q23", 1898},
    {"Q31Q15", 1899},
    {"Q31Q7", 1900},
    {"Q31Q30", 1901},
    {"Q31Q29", 1902},
    {"Q31Q28", 1903},
    {"Q31Q27", 1904},
    {"Q31Q26", 1905},
    {"Q31Q25", 1906},
    {"Q31Q24", 1907},
    {"Q32Q41", 1908},
    {"Q32Q50", 1909},
    {"Q32Q59", 1910},
    {"Q32Q25", 1911},
    {"Q32Q18", 1912},
    {"Q32Q11", 1913},
    {"Q32Q4", 1914},
    {"Q32Q40", 1915},
    {"Q32Q48", 1916},
    {"Q32Q56", 1917},
    {"Q32Q24", 1918},
    {"Q32Q16", 1919},
    {"Q32Q8", 1920},
    {"Q32Q0", 1921},
    {"Q32Q33", 1922},
    {"Q32Q34", 1923},
    {"Q32Q35", 1924},
    {"Q32Q36", 1925},
    {"Q32Q37", 1926},
    {"Q32Q38", 1927},
    {"Q32Q39", 1928},
    {"Q33Q42", 1929},
    {"Q33Q51", 1930},
    {"Q33Q60", 1931},
    {"Q33Q24", 1932},
    {"Q33Q40", 1933},
    {"Q33Q26", 1934},
    {"Q33Q19", 1935},
    {"Q33Q12", 1936},
    {"Q33Q5", 1937},
    {"Q33Q41", 1938},
    {"Q33Q49", 1939},
    {"Q33Q57", 1940},
    {"Q33Q25", 1941},
    {"Q33Q17", 1942},
    {"Q33Q9", 1943},
    {"Q33Q1", 1944},
    {"Q33Q34", 1945},
    {"Q33Q35", 1946},
    {"Q33Q36", 1947},
    {"Q33Q37", 1948},
    {"Q33Q38", 1949},
    {"Q33Q39", 1950},
    {"Q33Q32", 1951},
    {"Q34Q43", 1952},
    {"Q34Q52", 1953},
    {"Q34Q61", 1954},
    {"Q34Q25", 1955},
    {"Q34Q16", 1956},
    {"Q34Q41", 1957},
    {"Q34Q48", 1958},
    {"Q34Q27", 1959},
    {"Q34Q20", 1960},
    {"Q34Q13", 1961},
    {"Q34Q6", 1962},
    {"Q34Q42", 1963},
    {"Q34Q50", 1964},
    {"Q34Q58", 1965},
    {"Q34Q26", 1966},
    {"Q34Q18", 1967},
    {"Q34Q10", 1968},
    {"Q34Q2", 1969},
    {"Q34Q35", 1970},
    {"Q34Q36", 1971},
    {"Q34Q37", 1972},
    {"Q34Q38", 1973},
    {"Q34Q39", 1974},
    {"Q34Q33", 1975},
    {"Q34Q32", 1976},
    {"Q35Q44", 1977},
    {"Q35Q53", 1978},
    {"Q35Q62", 1979},
    {"Q35Q26", 1980},
    {"Q35Q17", 1981},
    {"Q35Q8", 1982},
    {"Q35Q42", 1983},
    {"Q35Q49", 1984},
    {"Q35Q56", 1985},
    {"Q35Q28", 1986},
    {"Q35Q21", 1987},
    {"Q35Q14", 1988},
    {"Q35Q7", 1989},
    {"Q35Q43", 1990},
    {"Q35Q51", 1991},
    {"Q35Q59", 1992},
    {"Q35Q27", 1993},
    {"Q35Q19", 1994},
    {"Q35Q11", 1995},
    {"Q35Q3", 1996},
    {"Q35Q36", 1997},
    {"Q35Q37", 1998},
    {"Q35Q38", 1999},
    {"Q35Q39", 2000},
    {"Q35Q34", 2001},
    {"Q35Q33", 2002},
    {"Q35Q32", 2003},
    {"Q36Q45", 2004},
    {"Q36Q54", 2005},
    {"Q36Q63", 2006},
    {"Q36Q27", 2007},
    {"Q36Q18", 2008},
    {"Q36Q9", 2009},
    {"Q36Q0", 2010},
    {"Q36Q43", 2011},
    {"Q36Q50", 2012},
    {"Q36Q57", 2013},
    {"Q36Q29", 2014},
    {"Q36Q22", 2015},
    {"Q36Q15", 2016},
    {"Q36Q44", 2017},
    {"Q36Q52", 2018},
    {"Q36Q60", 2019},
    {"Q36Q28", 2020},
    {"Q36Q20", 2021},
    {"Q36Q12", 2022},
    {"Q36Q4", 2023},
    {"Q36Q37", 2024},
    {"Q36Q38", 2025},
    {"Q36Q39", 2026},
    {"Q36Q35", 2027},
    {"Q36Q34", 2028},
    {"Q36Q33", 2029},
    {"Q36Q32", 2030},
    {"Q37Q46", 2031},
    {"Q37Q55", 2032},
    {"Q37Q28", 2033},
    {"Q37Q19", 2034},
    {"Q37Q10", 2035},
    {"Q37Q1", 2036},
    {"Q37Q44", 2037},
    {"Q37Q51", 2038},
    {"Q37Q58", 2039},
    {"Q37Q30", 2040},
    {"Q37Q23", 2041},
    {"Q37Q45", 2042},
    {"Q37Q53", 2043},
    {"Q37Q61", 2044},
    {"Q37Q29", 2045},
    {"Q37Q21", 2046},
    {"Q37Q13", 2047},
    {"Q37Q5", 2048},
    {"Q37Q38", 2049},
    {"Q37Q39", 2050},
    {"Q37Q36", 2051},
    {"Q37Q35", 2052},
    {"Q37Q34", 2053},
    {"Q37Q33", 2054},
    {"Q37Q32", 2055},
    {"Q38Q47", 2056},
    {"Q38Q29", 2057},
    {"Q38Q20", 2058},
    {"Q38Q11", 2059},
    {"Q38Q2", 2060},
    {"Q38Q45", 2061},
    {"Q38Q52", 2062},
    {"Q38Q59", 2063},
    {"Q38Q31", 2064},
    {"Q38Q46", 2065},
    {"Q38Q54", 2066},
    {"Q38Q62", 2067},
    {"Q38Q30", 2068},
    {"Q38Q22", 2069},
    {"Q38Q14", 2070},
    {"Q38Q6", 2071},
    {"Q38Q39", 2072},
    {"Q38Q37", 2073},
    {"Q38Q36", 2074},
    {"Q38Q35", 2075},
    {"Q38Q34", 2076},
    {"Q38Q33", 2077},
    {"Q38Q32", 2078},
    {"Q39Q30", 2079},
    {"Q39Q21", 2080},
    {"Q39Q12", 2081},
    {"Q39Q3", 2082},
    {"Q39Q46", 2083},
    {"Q39Q53", 2084},
    {"Q39Q60", 2085},
    {"Q39Q47", 2086},
    {"Q39Q55", 2087},
    {"Q39Q63", 2088},
    {"Q39Q31", 2089},
    {"Q39Q23", 2090},
    {"Q39Q15", 2091},
    {"Q39Q7", 2092},
    {"Q39Q38", 2093},
    {"Q39Q37", 2094},
    {"Q39Q36", 2095},
    {"Q39Q35", 2096},
    {"Q39Q34", 2097},
    {"Q39Q33", 2098},
    {"Q39Q32", 2099},
    {"Q40Q49", 2100},
    {"Q40Q58", 2101},
    {"Q40Q33", 2102},
    {"Q40Q26", 2103},
    {"Q40Q19", 2104},
    {"Q40Q12", 2105},
    {"Q40Q5", 2106},
    {"Q40Q48", 2107},
    {"Q40Q56", 2108},
    {"Q40Q32", 2109},
    {"Q40Q24", 2110},
    {"Q40Q16", 2111},
    {"Q40Q8", 2112},
    {"Q40Q0", 2113},
    {"Q40Q41", 2114},
    {"Q40Q42", 2115},
    {"Q40Q43", 2116},
    {"Q40Q44", 2117},
    {"Q40Q45", 2118},
    {"Q40Q46", 2119},
    {"Q40Q47", 2120},
    {"Q41Q50", 2121},
    {"Q41Q59", 2122},
    {"Q41Q32", 2123},
    {"Q41Q48", 2124},
    {"Q41Q34", 2125},
    {"Q41Q27", 2126},
    {"Q41Q20", 2127},
    {"Q41Q13", 2128},
    {"Q41Q6", 2129},
    {"Q41Q49", 2130},
    {"Q41Q57", 2131},
    {"Q41Q33", 2132},
    {"Q41Q25", 2133},
    {"Q41Q17", 2134},
    {"Q41Q9", 2135},
    {"Q41Q1", 2136},
    {"Q41Q42", 2137},
    {"Q41Q43", 2138},
    {"Q41Q44", 2139},
    {"Q41Q45", 2140},
    {"Q41Q46", 2141},
    {"Q41Q47", 2142},
    {"Q41Q40", 2143},
    {"Q42Q51", 2144},
    {"Q42Q60", 2145},
    {"Q42Q33", 2146},
    {"Q42Q24", 2147},
    {"Q42Q49", 2148},
    {"Q42Q56", 2149},
    {"Q42Q35", 2150},
    {"Q42Q28", 2151},
    {"Q42Q21", 2152},
    {"Q42Q14", 2153},
    {"Q42Q7", 2154},
    {"Q42Q50", 2155},
    {"Q42Q58", 2156},
    {"Q42Q34", 2157},
    {"Q42Q26", 2158},
    {"Q42Q18", 2159},
    {"Q42Q10", 2160},
    {"Q42Q2", 2161},
    {"Q42Q43", 2162},
    {"Q42Q44", 2163},
    {"Q42Q45", 2164},
    {"Q42Q46", 2165},
    {"Q42Q47", 2166},
    {"Q42Q41", 2167},
    {"Q42Q40", 2168},
    {"Q43Q52", 2169},
    {"Q43Q61", 2170},
    {"Q43Q34", 2171},
    {"Q43Q25", 2172},
    {"Q43Q16", 2173},
    {"Q43Q50", 2174},
    {"Q43Q57", 2175},
    {"Q43Q36", 2176},
    {"Q43Q29", 2177},
    {"Q43Q22", 2178},
    {"Q43Q15", 2179},
    {"Q43Q51", 2180},
    {"Q43Q59", 2181},
    {"Q43Q35", 2182},
    {"Q43Q27", 2183},
    {"Q43Q19", 2184},
    {"Q43Q11", 2185},
    {"Q43Q3", 2186},
    {"Q43Q44", 2187},
    {"Q43Q45", 2188},
    {"Q43Q46", 2189},
    {"Q43Q47", 2190},
    {"Q43Q42", 2191},
    {"Q43Q41", 2192},
    {"Q43Q40", 2193},
    {"Q44Q53", 2194},
    {"Q44Q62", 2195},
    {"Q44Q35", 2196},
    {"Q44Q26", 2197},
    {"Q44Q17", 2198},
    {"Q44Q8", 2199},
    {"Q44Q51", 2200},
    {"Q44Q58", 2201},
    {"Q44Q37", 2202},
    {"Q44Q30", 2203},
    {"Q44Q23", 2204},
    {"Q44Q52", 2205},
    {"Q44Q60", 2206},
    {"Q44Q36", 2207},
    {"Q44Q28", 2208},
    {"Q44Q20", 2209},
    {"Q44Q12", 2210},
    {"Q44Q4", 2211},
    {"Q44Q45", 2212},
    {"Q44Q46", 2213},
    {"Q44Q47", 2214},
    {"Q44Q43", 2215},
    {"Q44Q42", 2216},
    {"Q44Q41", 2217},
    {"Q44Q40", 2218},
    {"Q45Q54", 2219},
    {"Q45Q63", 2220},
    {"Q45Q36", 2221},
    {"Q45Q27", 2222},
    {"Q45Q18", 2223},
    {"Q45Q9", 2224},
    {"Q45Q0", 2225},
    {"Q45Q52", 2226},
    {"Q45Q59", 2227},
    {"Q45Q38", 2228},
    {"Q45Q31", 2229},
    {"Q45Q53", 2230},
    {"Q45Q61", 2231},
    {"Q45Q37", 2232},
    {"Q45Q29", 2233},
    {"Q45Q21", 2234},
    {"Q45Q13", 2235},
    {"Q45Q5", 2236},
    {"Q45Q46", 2237},
    {"Q45Q47", 2238},
    {"Q45Q44", 2239},
    {"Q45Q43", 2240},
    {"Q45Q42", 2241},
    {"Q45Q41", 2242},
    {"Q45Q40", 2243},
    {"Q46Q55", 2244},
    {"Q46Q37", 2245},
    {"Q46Q28", 2246},
    {"Q46Q19", 2247},
    {"Q46Q10", 2248},
    {"Q46Q1", 2249},
    {"Q46Q53", 2250},
    {"Q46Q60", 2251},
    {"Q46Q39", 2252},
    {"Q46Q54", 2253},
    {"Q46Q62", 2254},
    {"Q46Q38", 2255},
    {"Q46Q30", 2256},
    {"Q46Q22", 2257},
    {"Q46Q14", 2258},
    {"Q46Q6", 2259},
    {"Q46Q47", 2260},
    {"Q46Q45", 2261},
    {"Q46Q44", 2262},
    {"Q46Q43", 2263},
    {"Q46Q42", 2264},
    {"Q46Q41", 2265},
    {"Q46Q40", 2266},
    {"Q47Q38", 2267},
    {"Q47Q29", 2268},
    {"Q47Q20", 2269},
    {"Q47Q11", 2270},
    {"Q47Q2", 2271},
    {"Q47Q54", 2272},
    {"Q47Q61", 2273},
    {"Q47Q55", 2274},
    {"Q47Q63", 2275},
    {"Q47Q39", 2276},
    {"Q47Q31", 2277},
    {"Q47Q23", 2278},
    {"Q47Q15", 2279},
    {"Q47Q7", 2280},
    {"Q47Q46", 2281},
    {"Q47Q45", 2282},
    {"Q47Q44", 2283},
    {"Q47Q43", 2284},
    {"Q47Q42", 2285},
    {"Q47Q41", 2286},
    {"Q47Q40", 2287},
    {"Q48Q57", 2288},
    {"Q48Q41", 2289},
    {"Q48Q34", 2290},
    {"Q48Q27", 2291},
    {"Q48Q20", 2292},
    {"Q48Q13", 2293},
    {"Q48Q6", 2294},
    {"Q48Q56", 2295},
    {"Q48Q40", 2296},
    {"Q48Q32", 2297},
    {"Q48Q24", 2298},
    {"Q48Q16", 2299},
    {"Q48Q8", 2300},
    {"Q48Q0", 2301},
    {"Q48Q49", 2302},
    {"Q48Q50", 2303},
    {"Q48Q51", 2304},
    {"Q48Q52", 2305},
    {"Q48Q53", 2306},
    {"Q48Q54", 2307},
    {"Q48Q55", 2308},
    {"Q49Q58", 2309},
    {"Q49Q40", 2310},
    {"Q49Q56", 2311},
    {"Q49Q42", 2312},
    {"Q49Q35", 2313},
    {"Q49Q28", 2314},
    {"Q49Q21", 2315},
    {"Q49Q14", 2316},
    {"Q49Q7", 2317},
    {"Q49Q57", 2318},
    {"Q49Q41", 2319},
    {"Q49Q33", 2320},
    {"Q49Q25", 2321},
    {"Q49Q17", 2322},
    {"Q49Q9", 2323},
    {"Q49Q1", 2324},
    {"Q49Q50", 2325},
    {"Q49Q51", 2326},
    {"Q49Q52", 2327},
    {"Q49Q53", 2328},
    {"Q49Q54", 2329},
    {"Q49Q55", 2330},
    {"Q49Q48", 2331},
    {"Q50Q59", 2332},
    {"Q50Q41", 2333},
    {"Q50Q32", 2334},
    {"Q50Q57", 2335},
    {"Q50Q43", 2336},
    {"Q50Q36", 2337},
    {"Q50Q29", 2338},
    {"Q50Q22", 2339},
    {"Q50Q15", 2340},
    {"Q50Q58", 2341},
    {"Q50Q42", 2342},
    {"Q50Q34", 2343},
    {"Q50Q26", 2344},
    {"Q50Q18", 2345},
    {"Q50Q10", 2346},
    {"Q50Q2", 2347},
    {"Q50Q51", 2348},
    {"Q50Q52", 2349},
    {"Q50Q53", 2350},
    {"Q50Q54", 2351},
    {"Q50Q55", 2352},
    {"Q50Q49", 2353},
    {"Q50Q48", 2354},
    {"Q51Q60", 2355},
    {"Q51Q42", 2356},
    {"Q51Q33", 2357},
    {"Q51Q24", 2358},
    {"Q51Q58", 2359},
    {"Q51Q44", 2360},
    {"Q51Q37", 2361},
    {"Q51Q30", 2362},
    {"Q51Q23", 2363},
    {"Q51Q59", 2364},
    {"Q51Q43", 2365},
    {"Q51Q35", 2366},
    {"Q51Q27", 2367},
    {"Q51Q19", 2368},
    {"Q51Q11", 2369},
    {"Q51Q3", 2370},
    {"Q51Q52", 2371},
    {"Q51Q53", 2372},
    {"Q51Q54", 2373},
    {"Q51Q55", 2374},
    {"Q51Q50", 2375},
    {"Q51Q49", 2376},
    {"Q51Q48", 2377},
    {"Q52Q61", 2378},
    {"Q52Q43", 2379},
    {"Q52Q34", 2380},
    {"Q52Q25", 2381},
    {"Q52Q16", 2382},
    {"Q52Q59", 2383},
    {"Q52Q45", 2384},
    {"Q52Q38", 2385},
    {"Q52Q31", 2386},
    {"Q52Q60", 2387},
    {"Q52Q44", 2388},
    {"Q52Q36", 2389},
    {"Q52Q28", 2390},
    {"Q52Q20", 2391},
    {"Q52Q12", 2392},
    {"Q52Q4", 2393},
    {"Q52Q53", 2394},
    {"Q52Q54", 2395},
    {"Q52Q55", 2396},
    {"Q52Q51", 2397},
    {"Q52Q50", 2398},
    {"Q52Q49", 2399},
    {"Q52Q48", 2400},
    {"Q53Q62", 2401},
    {"Q53Q44", 2402},
    {"Q53Q35", 2403},
    {"Q53Q26", 2404},
    {"Q53Q17", 2405},
    {"Q53Q8", 2406},
    {"Q53Q60", 2407},
    {"Q53Q46", 2408},
    {"Q53Q39", 2409},
    {"Q53Q61", 2410},
    {"Q53Q45", 2411},
    {"Q53Q37", 2412},
    {"Q53Q29", 2413},
    {"Q53Q21", 2414},
    {"Q53Q13", 2415},
    {"Q53Q5", 2416},
    {"Q53Q54", 2417},
    {"Q53Q55", 2418},
    {"Q53Q52", 2419},
    {"Q53Q51", 2420},
    {"Q53Q50", 2421},
    {"Q53Q49", 2422},
    {"Q53Q48", 2423},
    {"Q54Q63", 2424},
    {"Q54Q45", 2425},
    {"Q54Q36", 2426},
    {"Q54Q27", 2427},
    {"Q54Q18", 2428},
    {"Q54Q9", 2429},
    {"Q54Q0", 2430},
    {"Q54Q61", 2431},
    {"Q54Q47", 2432},
    {"Q54Q62", 2433},
    {"Q54Q46", 2434},
    {"Q54Q38", 2435},
    {"Q54Q30", 2436},
    {"Q54Q22", 2437},
    {"Q54Q14", 2438},
    {"Q54Q6", 2439},
    {"Q54Q55", 2440},
    {"Q54Q53", 2441},
    {"Q54Q52", 2442},
    {"Q54Q51", 2443},
    {"Q54Q50", 2444},
    {"Q54Q49", 2445},
    {"Q54Q48", 2446},
    {"Q55Q46", 2447},
    {"Q55Q37", 2448},
    {"Q55Q28", 2449},
    {"Q55Q19", 2450},
    {"Q55Q10", 2451},
    {"Q55Q1", 2452},
    {"Q55Q62", 2453},
    {"Q55Q63", 2454},
    {"Q55Q47", 2455},
    {"Q55Q39", 2456},
    {"Q55Q31", 2457},
    {"Q55Q23", 2458},
    {"Q55Q15", 2459},
    {"Q55Q7", 2460},
    {"Q55Q54", 2461},
    {"Q55Q53", 2462},
    {"Q55Q52", 2463},
    {"Q55Q51", 2464},
    {"Q55Q50", 2465},
    {"Q55Q49", 2466},
    {"Q55Q48", 2467},
    {"N8N-9", 2468},
    {"N8N-7", 2469},
    {"N8N-2", 2470},
    {"N8N2", 2471},
    {"N8N14", 2472},
    {"N8N18", 2473},
    {"N8N23", 2474},
    {"N8N25", 2475},
    {"N9N-8", 2476},
    {"N9N-6", 2477},
    {"N9N-1", 2478},
    {"N9N3", 2479},
    {"N9N15", 2480},
    {"N9N19", 2481},
    {"N9N24", 2482},
    {"N9N26", 2483},
    {"N10N-7", 2484},
    {"N10N-5", 2485},
    {"N10N0", 2486},
    {"N10N4", 2487},
    {"N10N16", 2488},
    {"N10N20", 2489},
    {"N10N25", 2490},
    {"N10N27", 2491},
    {"N11N-6", 2492},
    {"N11N-4", 2493},
    {"N11N1", 2494},
    {"N11N5", 2495},
    {"N11N17", 2496},
    {"N11N21", 2497},
    {"N11N26", 2498},
    {"N11N28", 2499},
    {"N12N-5", 2500},
    {"N12N-3", 2501},
    {"N12N2", 2502},
    {"N12N6", 2503},
    {"N12N18", 2504},
    {"N12N22", 2505},
    {"N12N27", 2506},
    {"N12N29", 2507},
    {"N13N-4", 2508},
    {"N13N-2", 2509},
    {"N13N3", 2510},
    {"N13N7", 2511},
    {"N13N19", 2512},
    {"N13N23", 2513},
    {"N13N28", 2514},
    {"N13N30", 2515},
    {"N14N-3", 2516},
    {"N14N-1", 2517},
    {"N14N4", 2518},
    {"N14N8", 2519},
    {"N14N20", 2520},
    {"N14N24", 2521},
    {"N14N29", 2522},
    {"N14N31", 2523},
    {"N15N-2", 2524},
    {"N15N0", 2525},
    {"N15N5", 2526},
    {"N15N9", 2527},
    {"N15N21", 2528},
    {"N15N25", 2529},
    {"N15N30", 2530},
    {"N15N32", 2531},
    {"N16N-1", 2532},
    {"N16N1", 2533},
    {"N16N6", 2534},
    {"N16N10", 2535},
    {"N16N22", 2536},
    {"N16N26", 2537},
    {"N16N31", 2538},
    {"N16N33", 2539},
    {"N17N0", 2540},
    {"N17N2", 2541},
    {"N17N7", 2542},
    {"N17N11", 2543},
    {"N17N23", 2544},
    {"N17N27", 2545},
    {"N17N32", 2546},
    {"N17N34", 2547},
    {"N18N1", 2548},
    {"N18N3", 2549},
    {"N18N8", 2550},
    {"N18N12", 2551},
    {"N18N24", 2552},
    {"N18N28", 2553},
    {"N18N33", 2554},
    {"N18N35", 2555},
    {"N19N2", 2556},
    {"N19N4", 2557},
    {"N19N9", 2558},
    {"N19N13", 2559},
    {"N19N25", 2560},
    {"N19N29", 2561},
    {"N19N34", 2562},
    {"N19N36", 2563},
    {"N20N3", 2564},
    {"N20N5", 2565},
    {"N20N10", 2566},
    {"N20N14", 2567},
    {"N20N26", 2568},
    {"N20N30", 2569},
    {"N20N35", 2570},
    {"N20N37", 2571},
    {"N21N4", 2572},
    {"N21N6", 2573},
    {"N21N11", 2574},
    {"N21N15", 2575},
    {"N21N27", 2576},
    {"N21N31", 2577},
    {"N21N36", 2578},
    {"N21N38", 2579},
    {"N22N5", 2580},
    {"N22N7", 2581},
    {"N22N12", 2582},
    {"N22N16", 2583},
    {"N22N28", 2584},
    {"N22N32", 2585},
    {"N22N37", 2586},
    {"N22N39", 2587},
    {"N23N6", 2588},
    {"N23N8", 2589},
    {"N23N13", 2590},
    {"N23N17", 2591},
    {"N23N29", 2592},
    {"N23N33", 2593},
    {"N23N38", 2594},
    {"N23N40", 2595},
    {"N24N7", 2596},
    {"N24N9", 2597},
    {"N24N14", 2598},
    {"N24N18", 2599},
    {"N24N30", 2600},
    {"N24N34", 2601},
    {"N24N39", 2602},
    {"N24N41", 2603},
    {"N25N8", 2604},
    {"N25N10", 2605},
    {"N25N15", 2606},
    {"N25N19", 2607},
    {"N25N31", 2608},
    {"N25N35", 2609},
    {"N25N40", 2610},
    {"N25N42", 2611},
    {"N26N9", 2612},
    {"N26N11", 2613},
    {"N26N16", 2614},
    {"N26N20", 2615},
    {"N26N32", 2616},
    {"N26N36", 2617},
    {"N26N41", 2618},
    {"N26N43", 2619},
    {"N27N10", 2620},
    {"N27N12", 2621},
    {"N27N17", 2622},
    {"N27N21", 2623},
    {"N27N33", 2624},
    {"N27N37", 2625},
    {"N27N42", 2626},
    {"N27N44", 2627},
    {"N28N11", 2628},
    {"N28N13", 2629},
    {"N28N18", 2630},
    {"N28N22", 2631},
    {"N28N34", 2632},
    {"N28N38", 2633},
    {"N28N43", 2634},
    {"N28N45", 2635},
    {"N29N12", 2636},
    {"N29N14", 2637},
    {"N29N19", 2638},
    {"N29N23", 2639},
    {"N29N35", 2640},
    {"N29N39", 2641},
    {"N29N44", 2642},
    {"N29N46", 2643},
    {"N30N13", 2644},
    {"N30N15", 2645},
    {"N30N20", 2646},
    {"N30N24", 2647},
    {"N30N36", 2648},
    {"N30N40", 2649},
    {"N30N45", 2650},
    {"N30N47", 2651},
    {"N31N14", 2652},
    {"N31N16", 2653},
    {"N31N21", 2654},
    {"N31N25", 2655},
    {"N31N37", 2656},
    {"N31N41", 2657},
    {"N31N46", 2658},
    {"N31N48", 2659},
    {"N32N15", 2660},
    {"N32N17", 2661},
    {"N32N22", 2662},
    {"N32N26", 2663},
    {"N32N38", 2664},
    {"N32N42", 2665},
    {"N32N47", 2666},
    {"N32N49", 2667},
    {"N33N16", 2668},
    {"N33N18", 2669},
    {"N33N23", 2670},
    {"N33N27", 2671},
    {"N33N39", 2672},
    {"N33N43", 2673},
    {"N33N48", 2674},
    {"N33N50", 2675},
    {"N34N17", 2676},
    {"N34N19", 2677},
    {"N34N24", 2678},
    {"N34N28", 2679},
    {"N34N40", 2680},
    {"N34N44", 2681},
    {"N34N49", 2682},
    {"N34N51", 2683},
    {"N35N18", 2684},
    {"N35N20", 2685},
    {"N35N25", 2686},
    {"N35N29", 2687},
    {"N35N41", 2688},
    {"N35N45", 2689},
    {"N35N50", 2690},
    {"N35N52", 2691},
    {"N36N19", 2692},
    {"N36N21", 2693},
    {"N36N26", 2694},
    {"N36N30", 2695},
    {"N36N42", 2696},
    {"N36N46", 2697},
    {"N36N51", 2698},
    {"N36N53", 2699},
    {"N37N20", 2700},
    {"N37N22", 2701},
    {"N37N27", 2702},
    {"N37N31", 2703},
    {"N37N43", 2704},
    {"N37N47", 2705},
    {"N37N52", 2706},
    {"N37N54", 2707},
    {"N38N21", 2708},
    {"N38N23", 2709},
    {"N38N28", 2710},
    {"N38N32", 2711},
    {"N38N44", 2712},
    {"N38N48", 2713},
    {"N38N53", 2714},
    {"N38N55", 2715},
    {"N39N22", 2716},
    {"N39N24", 2717},
    {"N39N29", 2718},
    {"N39N33", 2719},
    {"N39N45", 2720},
    {"N39N49", 2721},
    {"N39N54", 2722},
    {"N39N56", 2723},
    {"N40N23", 2724},
    {"N40N25", 2725},
    {"N40N30", 2726},
    {"N40N34", 2727},
    {"N40N46", 2728},
    {"N40N50", 2729},
    {"N40N55", 2730},
    {"N40N57", 2731},
    {"N41N24", 2732},
    {"N41N26", 2733},
    {"N41N31", 2734},
    {"N41N35", 2735},
    {"N41N47", 2736},
    {"N41N51", 2737},
    {"N41N56", 2738},
    {"N41N58", 2739},
    {"N42N25", 2740},
    {"N42N27", 2741},
    {"N42N32", 2742},
    {"N42N36", 2743},
    {"N42N48", 2744},
    {"N42N52", 2745},
    {"N42N57", 2746},
    {"N42N59", 2747},
    {"N43N26", 2748},
    {"N43N28", 2749},
    {"N43N33", 2750},
    {"N43N37", 2751},
    {"N43N49", 2752},
    {"N43N53", 2753},
    {"N43N58", 2754},
    {"N43N60", 2755},
    {"N44N27", 2756},
    {"N44N29", 2757},
    {"N44N34", 2758},
    {"N44N38", 2759},
    {"N44N50", 2760},
    {"N44N54", 2761},
    {"N44N59", 2762},
    {"N44N61", 2763},
    {"N45N28", 2764},
    {"N45N30", 2765},
    {"N45N35", 2766},
    {"N45N39", 2767},
    {"N45N51", 2768},
    {"N45N55", 2769},
    {"N45N60", 2770},
    {"N45N62", 2771},
    {"N46N29", 2772},
    {"N46N31", 2773},
    {"N46N36", 2774},
    {"N46N40", 2775},
    {"N46N52", 2776},
    {"N46N56", 2777},
    {"N46N61", 2778},
    {"N46N63", 2779},
    {"N47N30", 2780},
    {"N47N32", 2781},
    {"N47N37", 2782},
    {"N47N41", 2783},
    {"N47N53", 2784},
    {"N47N57", 2785},
    {"N47N62", 2786},
    {"N47N64", 2787},
    {"N48N31", 2788},
    {"N48N33", 2789},
    {"N48N38", 2790},
    {"N48N42", 2791},
    {"N48N54", 2792},
    {"N48N58", 2793},
    {"N48N63", 2794},
    {"N48N65", 2795},
    {"N49N32", 2796},
    {"N49N34", 2797},
    {"N49N39", 2798},
    {"N49N43", 2799},
    {"N49N55", 2800},
    {"N49N59", 2801},
    {"N49N64", 2802},
    {"N49N66", 2803},
    {"N50N33", 2804},
    {"N50N35", 2805},
    {"N50N40", 2806},
    {"N50N44", 2807},
    {"N50N56", 2808},
    {"N50N60", 2809},
    {"N50N65", 2810},
    {"N50N67", 2811},
    {"N51N34", 2812},
    {"N51N36", 2813},
    {"N51N41", 2814},
    {"N51N45", 2815},
    {"N51N57", 2816},
    {"N51N61", 2817},
    {"N51N66", 2818},
    {"N51N68", 2819},
    {"N52N35", 2820},
    {"N52N37", 2821},
    {"N52N42", 2822},
    {"N52N46", 2823},
    {"N52N58", 2824},
    {"N52N62", 2825},
    {"N52N67", 2826},
    {"N52N69", 2827},
    {"N53N36", 2828},
    {"N53N38", 2829},
    {"N53N43", 2830},
    {"N53N47", 2831},
    {"N53N59", 2832},
    {"N53N63", 2833},
    {"N53N68", 2834},
    {"N53N70", 2835},
    {"N54N37", 2836},
    {"N54N39", 2837},
    {"N54N44", 2838},
    {"N54N48", 2839},
    {"N54N60", 2840},
    {"N54N64", 2841},
    {"N54N69", 2842},
    {"N54N71", 2843},
    {"N55N38", 2844},
    {"N55N40", 2845},
    {"N55N45", 2846},
    {"N55N49", 2847},
    {"N55N61", 2848},
    {"N55N65", 2849},
    {"N55N70", 2850},
    {"N55N72", 2851},
    {"K8K0", 2852},
    {"K8K1", 2853},
    {"K8K9", 2854},
    {"K8K16", 2855},
    {"K8K17", 2856},
    {"K9K0", 2857},
    {"K9K1", 2858},
    {"K9K2", 2859},
    {"K9K8", 2860},
    {"K9K10", 2861},
    {"K9K16", 2862},
    {"K9K17", 2863},
    {"K9K18", 2864},
    {"K10K1", 2865},
    {"K10K2", 2866},
    {"K10K3", 2867},
    {"K10K9", 2868},
    {"K10K11", 2869},
    {"K10K17", 2870},
    {"K10K18", 2871},
    {"K10K19", 2872},
    {"K11K2", 2873},
    {"K11K3", 2874},
    {"K11K4", 2875},
    {"K11K10", 2876},
    {"K11K12", 2877},
    {"K11K18", 2878},
    {"K11K19", 2879},
    {"K11K20", 2880},
    {"K12K3", 2881},
    {"K12K4", 2882},
    {"K12K5", 2883},
    {"K12K11", 2884},
    {"K12K13", 2885},
    {"K12K19", 2886},
    {"K12K20", 2887},
    {"K12K21", 2888},
    {"K13K4", 2889},
    {"K13K5", 2890},
    {"K13K6", 2891},
    {"K13K12", 2892},
    {"K13K14", 2893},
    {"K13K20", 2894},
    {"K13K21", 2895},
    {"K13K22", 2896},
    {"K14K5", 2897},
    {"K14K6", 2898},
    {"K14K7", 2899},
    {"K14K13", 2900},
    {"K14K15", 2901},
    {"K14K21", 2902},
    {"K14K22", 2903},
    {"K14K23", 2904},
    {"K15K6", 2905},
    {"K15K7", 2906},
    {"K15K14", 2907},
    {"K15K22", 2908},
    {"K15K23", 2909},
    {"K16K8", 2910},
    {"K16K9", 2911},
    {"K16K17", 2912},
    {"K16K24", 2913},
    {"K16K25", 2914},
    {"K17K8", 2915},
    {"K17K9", 2916},
    {"K17K10", 2917},
    {"K17K16", 2918},
    {"K17K18", 2919},
    {"K17K24", 2920},
    {"K17K25", 2921},
    {"K17K26", 2922},
    {"K18K9", 2923},
    {"K18K10", 2924},
    {"K18K11", 2925},
    {"K18K17", 2926},
    {"K18K19", 2927},
    {"K18K25", 2928},
    {"K18K26", 2929},
    {"K18K27", 2930},
    {"K19K10", 2931},
    {"K19K11", 2932},
    {"K19K12", 2933},
    {"K19K18", 2934},
    {"K19K20", 2935},
    {"K19K26", 2936},
    {"K19K27", 2937},
    {"K19K28", 2938},
    {"K20K11", 2939},
    {"K20K12", 2940},
    {"K20K13", 2941},
    {"K20K19", 2942},
    {"K20K21", 2943},
    {"K20K27", 2944},
    {"K20K28", 2945},
    {"K20K29", 2946},
    {"K21K12", 2947},
    {"K21K13", 2948},
    {"K21K14", 2949},
    {"K21K20", 2950},
    {"K21K22", 2951},
    {"K21K28", 2952},
    {"K21K29", 2953},
    {"K21K30", 2954},
    {"K22K13", 2955},
    {"K22K14", 2956},
    {"K22K15", 2957},
    {"K22K21", 2958},
    {"K22K23", 2959},
    {"K22K29", 2960},
    {"K22K30", 2961},
    {"K22K31", 2962},
    {"K23K14", 2963},
    {"K23K15", 2964},
    {"K23K22", 2965},
    {"K23K30", 2966},
    {"K23K31", 2967},
    {"K24K16", 2968},
    {"K24K17", 2969},
    {"K24K25", 2970},
    {"K24K32", 2971},
    {"K24K33", 2972},
    {"K25K16", 2973},
    {"K25K17", 2974},
    {"K25K18", 2975},
    {"K25K24", 2976},
    {"K25K26", 2977},
    {"K25K32", 2978},
    {"K25K33", 2979},
    {"K25K34", 2980},
    {"K26K17", 2981},
    {"K26K18", 2982},
    {"K26K19", 2983},
    {"K26K25", 2984},
    {"K26K27", 2985},
    {"K26K33", 2986},
    {"K26K34", 2987},
    {"K26K35", 2988},
    {"K27K18", 2989},
    {"K27K19", 2990},
    {"K27K20", 2991},
    {"K27K26", 2992},
    {"K27K28", 2993},
    {"K27K34", 2994},
    {"K27K35", 2995},
    {"K27K36", 2996},
    {"K28K19", 2997},
    {"K28K20", 2998},
    {"K28K21", 2999},
    {"K28K27", 3000},
    {"K28K29", 3001},
    {"K28K35", 3002},
    {"K28K36", 3003},
    {"K28K37", 3004},
    {"K29K20", 3005},
    {"K29K21", 3006},
    {"K29K22", 3007},
    {"K29K28", 3008},
    {"K29K30", 3009},
    {"K29K36", 3010},
    {"K29K37", 3011},
    {"K29K38", 3012},
    {"K30K21", 3013},
    {"K30K22", 3014},
    {"K30K23", 3015},
    {"K30K29", 3016},
    {"K30K31", 3017},
    {"K30K37", 3018},
    {"K30K38", 3019},
    {"K30K39", 3020},
    {"K31K22", 3021},
    {"K31K23", 3022},
    {"K31K30", 3023},
    {"K31K38", 3024},
    {"K31K39", 3025},
    {"K32K24", 3026},
    {"K32K25", 3027},
    {"K32K33", 3028},
    {"K32K40", 3029},
    {"K32K41", 3030},
    {"K33K24", 3031},
    {"K33K25", 3032},
    {"K33K26", 3033},
    {"K33K32", 3034},
    {"K33K34", 3035},
    {"K33K40", 3036},
    {"K33K41", 3037},
    {"K33K42", 3038},
    {"K34K25", 3039},
    {"K34K26", 3040},
    {"K34K27", 3041},
    {"K34K33", 3042},
    {"K34K35", 3043},
    {"K34K41", 3044},
    {"K34K42", 3045},
    {"K34K43", 3046},
    {"K35K26", 3047},
    {"K35K27", 3048},
    {"K35K28", 3049},
    {"K35K34", 3050},
    {"K35K36", 3051},
    {"K35K42", 3052},
    {"K35K43", 3053},
    {"K35K44", 3054},
    {"K36K27", 3055},
    {"K36K28", 3056},
    {"K36K29", 3057},
    {"K36K35", 3058},
    {"K36K37", 3059},
    {"K36K43", 3060},
    {"K36K44", 3061},
    {"K36K45", 3062},
    {"K37K28", 3063},
    {"K37K29", 3064},
    {"K37K30", 3065},
    {"K37K36", 3066},
    {"K37K38", 3067},
    {"K37K44", 3068},
    {"K37K45", 3069},
    {"K37K46", 3070},
    {"K38K29", 3071},
    {"K38K30", 3072},
    {"K38K31", 3073},
    {"K38K37", 3074},
    {"K38K39", 3075},
    {"K38K45", 3076},
    {"K38K46", 3077},
    {"K38K47", 3078},
    {"K39K30", 3079},
    {"K39K31", 3080},
    {"K39K38", 3081},
    {"K39K46", 3082},
    {"K39K47", 3083},
    {"K40K32", 3084},
    {"K40K33", 3085},
    {"K40K41", 3086},
    {"K40K48", 3087},
    {"K40K49", 3088},
    {"K41K32", 3089},
    {"K41K33", 3090},
    {"K41K34", 3091},
    {"K41K40", 3092},
    {"K41K42", 3093},
    {"K41K48", 3094},
    {"K41K49", 3095},
    {"K41K50", 3096},
    {"K42K33", 3097},
    {"K42K34", 3098},
    {"K42K35", 3099},
    {"K42K41", 3100},
    {"K42K43", 3101},
    {"K42K49", 3102},
    {"K42K50", 3103},
    {"K42K51", 3104},
    {"K43K34", 3105},
    {"K43K35", 3106},
    {"K43K36", 3107},
    {"K43K42", 3108},
    {"K43K44", 3109},
    {"K43K50", 3110},
    {"K43K51", 3111},
    {"K43K52", 3112},
    {"K44K35", 3113},
    {"K44K36", 3114},
    {"K44K37", 3115},
    {"K44K43", 3116},
    {"K44K45", 3117},
    {"K44K51", 3118},
    {"K44K52", 3119},
    {"K44K53", 3120},
    {"K45K36", 3121},
    {"K45K37", 3122},
    {"K45K38", 3123},
    {"K45K44", 3124},
    {"K45K46", 3125},
    {"K45K52", 3126},
    {"K45K53", 3127},
    {"K45K54", 3128},
    {"K46K37", 3129},
    {"K46K38", 3130},
    {"K46K39", 3131},
    {"K46K45", 3132},
    {"K46K47", 3133},
    {"K46K53", 3134},
    {"K46K54", 3135},
    {"K46K55", 3136},
    {"K47K38", 3137},
    {"K47K39", 3138},
    {"K47K46", 3139},
    {"K47K54", 3140},
    {"K47K55", 3141},
    {"K48K40", 3142},
    {"K48K41", 3143},
    {"K48K49", 3144},
    {"K48K56", 3145},
    {"K48K57", 3146},
    {"K49K40", 3147},
    {"K49K41", 3148},
    {"K49K42", 3149},
    {"K49K48", 3150},
    {"K49K50", 3151},
    {"K49K56", 3152},
    {"K49K57", 3153},
    {"K49K58", 3154},
    {"K50K41", 3155},
    {"K50K42", 3156},
    {"K50K43", 3157},
    {"K50K49", 3158},
    {"K50K51", 3159},
    {"K50K57", 3160},
    {"K50K58", 3161},
    {"K50K59", 3162},
    {"K51K42", 3163},
    {"K51K43", 3164},
    {"K51K44", 3165},
    {"K51K50", 3166},
    {"K51K52", 3167},
    {"K51K58", 3168},
    {"K51K59", 3169},
    {"K51K60", 3170},
    {"K52K43", 3171},
    {"K52K44", 3172},
    {"K52K45", 3173},
    {"K52K51", 3174},
    {"K52K53", 3175},
    {"K52K59", 3176},
    {"K52K60", 3177},
    {"K52K61", 3178},
    {"K53K44", 3179},
    {"K53K45", 3180},
    {"K53K46", 3181},
    {"K53K52", 3182},
    {"K53K54", 3183},
    {"K53K60", 3184},
    {"K53K61", 3185},
    {"K53K62", 3186},
    {"K54K45", 3187},
    {"K54K46", 3188},
    {"K54K47", 3189},
    {"K54K53", 3190},
    {"K54K55", 3191},
    {"K54K61", 3192},
    {"K54K62", 3193},
    {"K54K63", 3194},
    {"K55K46", 3195},
    {"K55K47", 3196},
    {"K55K54", 3197},
    {"K55K62", 3198},
    {"K55K63", 3199},
    {"CL", 3200},
    {"CR", 3201}
};






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

    
                                        
                                        
                                

         
         
      
