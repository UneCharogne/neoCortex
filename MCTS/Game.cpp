#include <array>
#include <vector>
#include <stdlib.h>
#include <algorithm>
#include <iostream>
#include "Game.hpp"


Move::Move(GameState *finalState) : finalState(finalState) {}


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
        //std::cout << "PROVA\n";
        //std::cout << "simulatedGame.size() = " << simulatedGame.size() << "\n";
        for(int i=simulatedGame.size()-1;i>0;i--) {
            //std::cout << "i = " << i << "\n";
            //std::cout << "simulatedGame.[" << i << "]->legalMoves.size() = " << simulatedGame[i]->legalMoves.size() << "\n";
            for(int j=0;j<simulatedGame[i]->legalMoves.size();j++) {
                //std::cout << "j = " << j << "\n";
                delete simulatedGame[i]->legalMoves[j].finalState;
            }
            simulatedGame[i]->computedLegalMoves = false;
        }
    }
    //std::cout << "PROVONA\n";
    
    return winner;
}



DraughtsMove::DraughtsMove(DraughtsState *finalState, int movingPiece, int piecesTaken, int kingsTaken) : Move(finalState), movingPiece(movingPiece), piecesTaken(piecesTaken), kingsTaken(kingsTaken) {}
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
            possibleMoves.push_back(TicTacToeMove(new TicTacToeState(newBoard, (-1 * this->player))));
        }
    }
    
    std::vector<Move> legalMoves = possibleMoves;
    return legalMoves;
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
            if(this->board[(3*i)+j] == 1)
                std::cout << "o";
            if(this->board[(3*i)+j] == -1)
                std::cout << "x";
            if(this->board[(3*i)+j] == 0)
                std::cout << " ";
            std::cout << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n\n";
}

    
                                        
                                        
                                

         
         
      
