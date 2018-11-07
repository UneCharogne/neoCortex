#include <array>
#include <vector>
#include <stdlib.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include "Chess.hpp"


#define DEBUG_MODE 0






ChessMove::ChessMove(ChessState *finalState, int piece, int startingSquare, int id) : finalState(finalState), piece(piece), startingSquare(startingSquare), id(id) { }







ChessState::ChessState(ChessBoard board, std::vector<ChessBoard> previousBoards, std::array<int,2> kingPositions, std::array<std::array<int,2>,2> possibleCastling, int Nmove, int CounterToDraw, int player) : kingPositions(kingPositions), previousBoards(previousBoards), possibleCastling(possibleCastling), Nmove(Nmove), CounterToDraw(CounterToDraw), board(board) {
    this->player = player;
    this->computedLegalMoves = false;
    this->Repetition = 1;
}
ChessState::ChessState(ChessBoard board, int player) : board(board) {
    this->player = player;
    this->computedLegalMoves = false;

    //Run over the chess to locate the white and black kings
    for(int square = 0;square<64;square++) {
      if(this->board[square] == white_king)
      {
        this->kingPositions[0] = square;
      }
      if(this->board[square] == black_king)
      {
        this->kingPositions[1] = square;
      }
    }

    //If nothing is stated we assume that castling is possible if the king and the rooks are in their starting place
    if(this->kingPositions[0] == 4) {
      if(this->board[0] == white_rook) {
        this->possibleCastling[0][0] = 1;
      }
      else {
        this->possibleCastling[0][0] = 0;
      }
      if(this->board[7] == white_rook) {
        this->possibleCastling[0][1] = 1;
      }
      else {
        this->possibleCastling[0][1] = 0;
      }
    } 
    else {
      this->possibleCastling[0][0] = 0;
      this->possibleCastling[0][1] = 0;
    }

    if(this->kingPositions[1] == 60) {
      if(this->board[56] == black_rook) {
        this->possibleCastling[1][0] = 1;
      }
      else {
        this->possibleCastling[1][0] = 0;
      }
      if(this->board[63] == black_rook) {
        this->possibleCastling[1][1] = 1;
      }
      else {
        this->possibleCastling[1][1] = 0;
      }
    } 
    else {
      this->possibleCastling[1][0] = 0;
      this->possibleCastling[1][1] = 0;
    }


    //And that this is the starting move of the game
    this->previousBoards = std::vector<ChessBoard>();
    this->Nmove = 0;
    this->CounterToDraw = 0;
    this->Repetition = 1;
}
ChessState::ChessState(void) : ChessState(CHESS_STARTING_BOARD, 1) {}


//Returns the player
int ChessState::getPlayer(void) {
    return this->player;
}

//Return the legal moves from this state
std::vector<ChessMove> ChessState::getLegalMoves(void) {
    //If the legal moves have not yet been calculated, calculate them
    if(this->computedLegalMoves == false)
    {
        this->legalMoves = computeLegalMoves();
        this->computedLegalMoves = true;
    }
    
    //Then, return them
    return this->legalMoves;
}

ChessBoard ChessState::getBoard(void) {
    return this->board;
}


//Returns the winner given chess rules
int ChessState::getWinner(void) {
    //If this is a final state
    if(this->isFinalState()) {
        //If the king of the player is under check, he lost
        if(ChessState::isUnderAttack(this->board, this->kingPositions[((1 + (-1 * this->player)) / 2)], -1 * this->player)) {
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
	//If a three-fold repetition happend, or if the counter to draw reached 50, the game is over
	if(this->previousBoards.size() == 8) {
		if(this->board == this->previousBoards[3]) {
	        if (this->board == this->previousBoards[7]) {
				//A three-fold repetition happened
				this->Repetition = 3;
				return true;
			}
			else {
				this->Repetition = 2;
			}
		}
	}
	if(this->CounterToDraw == MAX_COUNTER_TO_DRAW) {
		return true;
	}

    //If the player has no more legal moves available, the game is over
    std::vector<ChessMove> legalMoves = this->getLegalMoves();
    
    if(legalMoves.size() == 0)
    {
        return true;
    }
    
    //Otherwise, the game is not over yet
    return false;
}


//Simulates a random draughts game starting from the game state
int ChessState::simulateGame(void) {
    //Get the starting state
    ChessState *currentState = this;
    //and check if it is final
    bool isFinal = currentState->isFinalState();
    
    //If it's not, perform a random simulation starting from this state
    std::vector<ChessState*> simulatedGame;
    simulatedGame.push_back(currentState);
    int Nmoves = 0;
    while((!isFinal) && (Nmoves < MAX_SIMULATION_LENGTH)) {
        //Get the possible legal moves from the current state
        std::vector<ChessMove> legalMoves = simulatedGame[simulatedGame.size()-1]->getLegalMoves();
        
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


std::array<int,2> ChessState::getKingPositions(void) {
    return this->kingPositions;
}


//All the legal moves from this state have to be built
std::vector<ChessMove> ChessState::computeLegalMoves(void) {
    int i, j, n;
    int check;
    std::vector<ChessMove> possibleMoves;


    ChessBoard oldBoard = this->board;

    for(int square=0;square<64;square++) {
      if(this->player == 1) {
    	if(oldBoard[square] == white_pawn2) {
    		oldBoard[square] = white_pawn;
    	}
      } else {
    	if(oldBoard[square] == black_pawn2) {
    		oldBoard[square] = black_pawn;
    	}
      }
    }

    //Run over all the cells
    for(int square=0;square<64;square++) {
      if(this->player == 1) {
        switch(oldBoard[square]) {
          case white_pawn:
            //A pawn can either move ahead of one step not eating
            if((square < 48) && (this->board[square + 8] == empty)) {
              ChessBoard newBoard = oldBoard;
              std::array<int,2> newKingPositions = this->kingPositions;
              std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
              std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
              if(newPreviousBoards.size() == 8) {
              	std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
              	newPreviousBoards[0] = this->board;
              } else if(newPreviousBoards.size() < 8) {
              	newPreviousBoards.insert(newPreviousBoards.begin(), board);
              } 
              int NewNmove = this->Nmove + 1;
              int newCounterToDraw = 0;
              newBoard[square] = empty;
              newBoard[square + 8] = white_pawn;

              //And if the move does not lead to check it a possible move
              if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
              {
                possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), PAWN, square, 0));
              }
            } //Eventually getting promoted if it is moving to the last row
            else if((square < 56) && (this->board[square + 8] == empty)) {
              ChessBoard newBoard = oldBoard;
              std::array<int,2> newKingPositions = this->kingPositions;
              std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
              std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
              if(newPreviousBoards.size() == 8) {
              	std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
              	newPreviousBoards[0] = this->board;
              } else if(newPreviousBoards.size() < 8) {
              	newPreviousBoards.insert(newPreviousBoards.begin(), board);
              }
              int NewNmove = this->Nmove + 1;
              int newCounterToDraw = 0;
              newBoard[square] = empty;
              newBoard[square + 8] = white_knight;
              //And if the move does not lead to check it a possible move
              if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
              {
                possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), PAWN, square, 9));
              }

              newBoard[square + 8] = white_queen;
              //And if the move does not lead to check it a possible move
              if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
              {
                possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), PAWN, square, 6));
              }
            }

            //Or eat on the right and left
            if((square < 48) && ((square % 8) < 7) && (PIECES_COLORS[this->board[square + 8 + 1]] == -1)) {
              ChessBoard newBoard = oldBoard;
              std::array <int,2> newKingPositions = this->kingPositions;
              std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
              std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
              if(newPreviousBoards.size() == 8) {
              	std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
              	newPreviousBoards[0] = this->board;
              } else if(newPreviousBoards.size() < 8) {
              	newPreviousBoards.insert(newPreviousBoards.begin(), board);
              }
              int NewNmove = this->Nmove + 1;
              int newCounterToDraw = 0;
              newBoard[square] = empty;
              newBoard[square + 8 + 1] = white_pawn;

              //And if the move does not lead to check it a possible move
              if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
              {
                possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), PAWN, square, 2));
              }
            } //Eventually getting promoted if it is moving to the last row
            else if((square < 56) && ((square % 8) < 7) && (PIECES_COLORS[this->board[square + 8 + 1]] == -1)) {
              ChessBoard newBoard = oldBoard;
              std::array <int,2> newKingPositions = this->kingPositions;
              std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
              std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
              if(newPreviousBoards.size() == 8) {
              	std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
              	newPreviousBoards[0] = this->board;
              } else if(newPreviousBoards.size() < 8) {
              	newPreviousBoards.insert(newPreviousBoards.begin(), board);
              }
              int NewNmove = this->Nmove + 1;
              int newCounterToDraw = 0;
              newBoard[square] = empty;
			  newBoard[square + 8 + 1] = white_knight;
              //And if the move does not lead to check it a possible move
              if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
              {
                possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), PAWN, square, 11));
              }

              newBoard[square + 8 + 1] = white_queen;
              //And if the move does not lead to check it a possible move
              if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
              {
                possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), PAWN, square, 8));
              }
            }
            if((square < 48) && ((square % 8) > 0) && (PIECES_COLORS[this->board[square + 8 - 1]] == -1)) {
              ChessBoard newBoard = oldBoard;
              std::array<int,2> newKingPositions = this->kingPositions;
              std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
              std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
              if(newPreviousBoards.size() == 8) {
              	std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
              	newPreviousBoards[0] = this->board;
              } else if(newPreviousBoards.size() < 8) {
              	newPreviousBoards.insert(newPreviousBoards.begin(), board);
              }
              int NewNmove = this->Nmove + 1;
              int newCounterToDraw = 0;
              newBoard[square] = empty;
              newBoard[square + 8 - 1] = white_pawn;

              //And if the move does not lead to check it a possible move
              if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
              {
                possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), PAWN, square, 1));
              }
            } //Eventually getting promoted if it is moving to the last row
            else if((square < 56) && ((square % 8) > 0) && (PIECES_COLORS[this->board[square + 8 - 1]] == -1)) {
              ChessBoard newBoard = oldBoard;
              std::array<int,2> newKingPositions = this->kingPositions;
              std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
              std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
              if(newPreviousBoards.size() == 8) {
              	std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
              	newPreviousBoards[0] = this->board;
              } else if(newPreviousBoards.size() < 8) {
              	newPreviousBoards.insert(newPreviousBoards.begin(), board);
              }
              int NewNmove = this->Nmove + 1;
              int newCounterToDraw = 0;
              newBoard[square] = empty;
              newBoard[square + 8 - 1] = white_knight;
              //And if the move does not lead to check it a possible move
              if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
              {
                possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), PAWN, square, 10));
              }

              newBoard[square + 8 - 1] = white_queen;
              //And if the move does not lead to check it a possible move
              if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
              {
                possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), PAWN, square, 7));
              }
            }

            //If it is in his starting cell it can move ahead of two cells too
            if(((square / 8) == 1) && (this->board[square + 8] == empty) && (this->board[square + 16] == empty)) {
              ChessBoard newBoard = oldBoard;
              std::array<int,2> newKingPositions = this->kingPositions;
              std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
              std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
              if(newPreviousBoards.size() == 8) {
              	std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
              	newPreviousBoards[0] = this->board;
              } else if(newPreviousBoards.size() < 8) {
              	newPreviousBoards.insert(newPreviousBoards.begin(), board);
              }
              int NewNmove = this->Nmove + 1;
              int newCounterToDraw = 0;
              newBoard[square] = empty;
              newBoard[square + 16] = white_pawn2;

              //And if the move does not lead to check it a possible move
              if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
              {
                possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), PAWN, square, 3));
              }
            }

            //If it is in the 5th row, a capture en passant is possible
            if((square >= 32) && (square < 39)) {
              if((this->board[(square + 1)] == black_pawn2) && (this->board[square + 9] == empty)) {
                ChessBoard newBoard = oldBoard;
                std::array<int,2> newKingPositions = this->kingPositions;
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
                if(newPreviousBoards.size() == 8) {
                  std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
                  newPreviousBoards[0] = this->board;
                } else if(newPreviousBoards.size() < 8) {
                	newPreviousBoards.insert(newPreviousBoards.begin(), board);
                }
                int NewNmove = this->Nmove + 1;
                int newCounterToDraw = 0;
                newBoard[square] = empty;
                newBoard[square + 1] = empty;
                newBoard[square + 9] = white_pawn;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), PAWN, square, 5));
                }
              }
            }

            //If it is in the 5th row, a capture en passant is possible
            if((square > 32) && (square <= 39)) {
              if((this->board[(square - 1)] == black_pawn2) && (this->board[square + 7] == empty)) {
                ChessBoard newBoard = oldBoard;
                std::array<int,2> newKingPositions = this->kingPositions;
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
                if(newPreviousBoards.size() == 8) {
              	  std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
              	  newPreviousBoards[0] = this->board;
                } else if(newPreviousBoards.size() < 8) {
               	  newPreviousBoards.insert(newPreviousBoards.begin(), board);
                }
                int NewNmove = this->Nmove + 1;
                int newCounterToDraw = 0;
                newBoard[square] = empty;
                newBoard[square - 1] = empty;
                newBoard[square + 7] = white_pawn;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), PAWN, square, 4));
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
                ChessBoard newBoard = oldBoard;
                std::array<int,2> newKingPositions = this->kingPositions;
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
                if(newPreviousBoards.size() == 8) {
              	  std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
              	  newPreviousBoards[0] = this->board;
                } else if(newPreviousBoards.size() < 8) {
              	  newPreviousBoards.insert(newPreviousBoards.begin(), board);
                }
                int NewNmove = this->Nmove + 1;
                int newCounterToDraw = this->CounterToDraw + 1;
                newBoard[square] = empty;
                newBoard[square + (9 * n)] = white_bishop;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), BISHOP, square, (20+n)));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + (9 * n)]] == -1) {
                  ChessBoard newBoard = oldBoard;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
                  if(newPreviousBoards.size() == 8) {
              	    std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
              	    newPreviousBoards[0] = this->board;
                  } else if(newPreviousBoards.size() < 8) {
              	    newPreviousBoards.insert(newPreviousBoards.begin(), board);
                  }
                  int NewNmove = this->Nmove + 1;
                  int newCounterToDraw = 0;
                  newBoard[square] = empty;
                  newBoard[square + (9 * n)] = white_bishop;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), BISHOP, square, (20+n)));
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
                ChessBoard newBoard = oldBoard;
                std::array<int,2> newKingPositions = this->kingPositions;
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
                if(newPreviousBoards.size() == 8) {
              	  std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
              	  newPreviousBoards[0] = this->board;
                } else if(newPreviousBoards.size() < 8) {
              	  newPreviousBoards.insert(newPreviousBoards.begin(), board);
                }
                int NewNmove = this->Nmove + 1;
                int newCounterToDraw = this->CounterToDraw + 1;
                newBoard[square] = empty;
                newBoard[square - (9 * n)] = white_bishop;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), BISHOP, square, (6+n)));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - (9 * n)]] == -1) {
                  ChessBoard newBoard = oldBoard;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
                  if(newPreviousBoards.size() == 8) {
              	    std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
              	    newPreviousBoards[0] = this->board;
                  } else if(newPreviousBoards.size() < 8) {
              	    newPreviousBoards.insert(newPreviousBoards.begin(), board);
                  }
                  int NewNmove = this->Nmove + 1;
                  int newCounterToDraw = 0;
                  newBoard[square] = empty;
                  newBoard[square - (9 * n)] = white_bishop;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), BISHOP, square, (6+n)));
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
                ChessBoard newBoard = oldBoard;
                std::array<int,2> newKingPositions = this->kingPositions;
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
                if(newPreviousBoards.size() == 8) {
              	  std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
              	  newPreviousBoards[0] = this->board;
                } else if(newPreviousBoards.size() < 8) {
              	  newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
                }
                int NewNmove = this->Nmove + 1;
                int newCounterToDraw = this->CounterToDraw + 1;
                newBoard[square] = empty;
                newBoard[square + (7 * n)] = white_bishop;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), BISHOP, square, (n-1)));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + (7 * n)]] == -1) {
                  ChessBoard newBoard = oldBoard;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
                  if(newPreviousBoards.size() == 8) {
              	    std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
              	    newPreviousBoards[0] = this->board;
                  } else if(newPreviousBoards.size() < 8) {
              	    newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
                  }
                  int NewNmove = this->Nmove + 1;
                  int newCounterToDraw = 0;
                  newBoard[square] = empty;
                  newBoard[square + (7 * n)] = white_bishop;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), BISHOP, square, (n-1)));
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
                ChessBoard newBoard = oldBoard;
                std::array<int,2> newKingPositions = this->kingPositions;
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
                if(newPreviousBoards.size() == 8) {
              	  std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
              	  newPreviousBoards[0] = this->board;
                } else if(newPreviousBoards.size() < 8) {
              	  newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
                }
                int NewNmove = this->Nmove + 1;
                int newCounterToDraw = this->CounterToDraw + 1;
                newBoard[square] = empty;
                newBoard[square - (7 * n)] = white_bishop;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), BISHOP, square, (13+n)));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - (7 * n)]] == -1) {
                  ChessBoard newBoard = oldBoard;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
                  if(newPreviousBoards.size() == 8) {
              	    std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
              	    newPreviousBoards[0] = this->board;
                  } else if(newPreviousBoards.size() < 8) {
              	    newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
                  }
                  int NewNmove = this->Nmove + 1;
                  int newCounterToDraw = 0;
                  newBoard[square] = empty;
                  newBoard[square - (7 * n)] = white_bishop;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), BISHOP, square, (13+n)));
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
                ChessBoard newBoard = oldBoard;
                std::array<int,2> newKingPositions = this->kingPositions;
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
                if(newPreviousBoards.size() == 8) {
              	  std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
              	  newPreviousBoards[0] = this->board;
                } else if(newPreviousBoards.size() < 8) {
              	  newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
                }
                int NewNmove = this->Nmove + 1;
                int newCounterToDraw = this->CounterToDraw + 1;
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
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), ROOK, square, (i-1)));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + (8 * i)]] == -1) {
                  ChessBoard newBoard = oldBoard;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
                  if(newPreviousBoards.size() == 8) {
              	    std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
              	    newPreviousBoards[0] = this->board;
                  } else if(newPreviousBoards.size() < 8) {
              	    newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
                  }
                  int NewNmove = this->Nmove + 1;
                  int newCounterToDraw = 0;
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
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), ROOK, square, (i-1)));
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
                ChessBoard newBoard = oldBoard;
                std::array<int,2> newKingPositions = this->kingPositions;
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
                if(newPreviousBoards.size() == 8) {
              	  std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
              	  newPreviousBoards[0] = this->board;
                } else if(newPreviousBoards.size() < 8) {
              	  newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
                }
                int NewNmove = this->Nmove + 1;
                int newCounterToDraw = this->CounterToDraw + 1;
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
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), ROOK, square, (13+i)));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - (8 * i)]] == -1) {
                  ChessBoard newBoard = oldBoard;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
                  if(newPreviousBoards.size() == 8) {
              	    std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
              	    newPreviousBoards[0] = this->board;
                  } else if(newPreviousBoards.size() < 8) {
              	    newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
                  }
                  int NewNmove = this->Nmove + 1;
                  int newCounterToDraw = 0;
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
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), ROOK, square, (13+i)));
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
                ChessBoard newBoard = oldBoard;
                std::array<int,2> newKingPositions = this->kingPositions;
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
                if(newPreviousBoards.size() == 8) {
              	  std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
              	  newPreviousBoards[0] = this->board;
                } else if(newPreviousBoards.size() < 8) {
              	  newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
                }
                int NewNmove = this->Nmove + 1;
                int newCounterToDraw = this->CounterToDraw + 1;
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
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), ROOK, square, (20+j)));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + j]] == -1) {
                  ChessBoard newBoard = oldBoard;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
                  if(newPreviousBoards.size() == 8) {
              	    std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
              	    newPreviousBoards[0] = this->board;
                  } else if(newPreviousBoards.size() < 8) {
              	    newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
                  }
                  int NewNmove = this->Nmove + 1;
                  int newCounterToDraw = 0;
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
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), ROOK, square, (20+j)));
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
                ChessBoard newBoard = oldBoard;
                std::array<int,2> newKingPositions = this->kingPositions;
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
                if(newPreviousBoards.size() == 8) {
              	  std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
              	  newPreviousBoards[0] = this->board;
                } else if(newPreviousBoards.size() < 8) {
              	  newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
                }
                int NewNmove = this->Nmove + 1;
                int newCounterToDraw = this->CounterToDraw + 1;
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
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), ROOK, square, (6+j)));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - j]] == -1) {
                  ChessBoard newBoard = oldBoard;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
                  if(newPreviousBoards.size() == 8) {
              	    std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
              	    newPreviousBoards[0] = this->board;
                  } else if(newPreviousBoards.size() < 8) {
              	    newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
                  }
                  int NewNmove = this->Nmove + 1;
                  int newCounterToDraw = 0;
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
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), ROOK, square, (6+j)));
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
                ChessBoard newBoard = oldBoard;
                std::array<int,2> newKingPositions = this->kingPositions;
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
                if(newPreviousBoards.size() == 8) {
              	  std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
              	  newPreviousBoards[0] = this->board;
                } else if(newPreviousBoards.size() < 8) {
              	  newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
                }
                int NewNmove = this->Nmove + 1;
                int newCounterToDraw = this->CounterToDraw + 1;
                newBoard[square] = empty;
                newBoard[square + (8 * i)] = white_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), QUEEN, square, (i-1)));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + (8 * i)]] == -1) {
                  ChessBoard newBoard = oldBoard;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
                  if(newPreviousBoards.size() == 8) {
              	    std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
              	    newPreviousBoards[0] = this->board;
                  } else if(newPreviousBoards.size() < 8) {
              	    newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
                  }
                  int NewNmove = this->Nmove + 1;
                  int newCounterToDraw = 0;
                  newBoard[square] = empty;
                  newBoard[square + (8 * i)] = white_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), QUEEN, square, (i-1)));
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
                ChessBoard newBoard = oldBoard;
                std::array<int,2> newKingPositions = this->kingPositions;
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
                if(newPreviousBoards.size() == 8) {
              	  std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
              	  newPreviousBoards[0] = this->board;
                } else if(newPreviousBoards.size() < 8) {
              	  newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
                }
                int NewNmove = this->Nmove + 1;
                int newCounterToDraw = this->CounterToDraw + 1;
                newBoard[square] = empty;
                newBoard[square - (8 * i)] = white_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), QUEEN, square, (27+i)));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - (8 * i)]] == -1) {
                  ChessBoard newBoard = oldBoard;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
                  if(newPreviousBoards.size() == 8) {
              	    std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
              	    newPreviousBoards[0] = this->board;
                  } else if(newPreviousBoards.size() < 8) {
              	    newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
                  } 
                  int NewNmove = this->Nmove + 1;
                  int newCounterToDraw = 0;
                  newBoard[square] = empty;
                  newBoard[square - (8 * i)] = white_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), QUEEN, square, (27+i)));
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
                ChessBoard newBoard = oldBoard;
                std::array<int,2> newKingPositions = this->kingPositions;
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
                if(newPreviousBoards.size() == 8) {
              	  std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
              	  newPreviousBoards[0] = this->board;
                } else if(newPreviousBoards.size() < 8) {
              	  newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
                }
                int NewNmove = this->Nmove + 1;
                int newCounterToDraw = this->CounterToDraw + 1;
                newBoard[square] = empty;
                newBoard[square + j] = white_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), QUEEN, square, (41+j)));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + j]] == -1) {
                  ChessBoard newBoard = oldBoard;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
                  if(newPreviousBoards.size() == 8) {
              	    std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
              	    newPreviousBoards[0] = this->board;
                  } else if(newPreviousBoards.size() < 8) {
              	    newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
                  }
                  int NewNmove = this->Nmove + 1;
                  int newCounterToDraw = 0;
                  newBoard[square] = empty;
                  newBoard[square + j] = white_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), QUEEN, square, (41+j)));
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
                ChessBoard newBoard = oldBoard;
                std::array<int,2> newKingPositions = this->kingPositions;
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
                if(newPreviousBoards.size() == 8) {
              	  std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
              	  newPreviousBoards[0] = this->board;
                } else if(newPreviousBoards.size() < 8) {
              	  newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
                }
                int NewNmove = this->Nmove + 1;
                int newCounterToDraw = this->CounterToDraw + 1;
                newBoard[square] = empty;
                newBoard[square - j] = white_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), QUEEN, square, (13+j)));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - j]] == -1) {
                  ChessBoard newBoard = oldBoard;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
                  if(newPreviousBoards.size() == 8) {
              	    std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
              	    newPreviousBoards[0] = this->board;
                  } else if(newPreviousBoards.size() < 8) {
              	    newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
                  }
                  int NewNmove = this->Nmove + 1;
                  int newCounterToDraw = this->CounterToDraw + 1;
                  newBoard[square] = empty;
                  newBoard[square - j] = white_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), QUEEN, square, (13+j)));
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
                ChessBoard newBoard = oldBoard;
                std::array<int,2> newKingPositions = this->kingPositions;
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
                if(newPreviousBoards.size() == 8) {
              	  std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
              	  newPreviousBoards[0] = this->board;
                } else if(newPreviousBoards.size() < 8) {
              	  newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
                }
                int NewNmove = this->Nmove + 1;
                int newCounterToDraw = this->CounterToDraw + 1;
                newBoard[square] = empty;
                newBoard[square + (9 * n)] = white_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), QUEEN, square, (48+n)));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + (9 * n)]] == -1) {
                  ChessBoard newBoard = oldBoard;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
                  if(newPreviousBoards.size() == 8) {
              	    std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
              	    newPreviousBoards[0] = this->board;
                  } else if(newPreviousBoards.size() < 8) {
              	    newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
                  }
                  int NewNmove = this->Nmove + 1;
                  int newCounterToDraw = 0;
                  newBoard[square] = empty;
                  newBoard[square + (9 * n)] = white_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), QUEEN, square, (48+n)));
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
                ChessBoard newBoard = oldBoard;
                std::array<int,2> newKingPositions = this->kingPositions;
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
                if(newPreviousBoards.size() == 8) {
              	  std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
              	  newPreviousBoards[0] = this->board;
                } else if(newPreviousBoards.size() < 8) {
              	  newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
                }
                int NewNmove = this->Nmove + 1;
                int newCounterToDraw = this->CounterToDraw + 1;
                newBoard[square] = empty;
                newBoard[square - (9 * n)] = white_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), QUEEN, square, (20+n)));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - (9 * n)]] == -1) {
                  ChessBoard newBoard = oldBoard;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
                  if(newPreviousBoards.size() == 8) {
              	    std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
              	    newPreviousBoards[0] = this->board;
                  } else if(newPreviousBoards.size() < 8) {
              	    newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
                  }
                  int NewNmove = this->Nmove + 1;
                  int newCounterToDraw = 0;
                  newBoard[square] = empty;
                  newBoard[square - (9 * n)] = white_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), QUEEN, square, (20+n)));
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
                ChessBoard newBoard = oldBoard;
                std::array<int,2> newKingPositions = this->kingPositions;
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
                if(newPreviousBoards.size() == 8) {
              	  std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
              	  newPreviousBoards[0] = this->board;
                } else if(newPreviousBoards.size() < 8) {
              	  newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
                }
                int NewNmove = this->Nmove + 1;
                int newCounterToDraw = this->CounterToDraw + 1;
                newBoard[square] = empty;
                newBoard[square + (7 * n)] = white_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), QUEEN, square, (6+n)));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + (7 * n)]] == -1) {
                  ChessBoard newBoard = oldBoard;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
                  if(newPreviousBoards.size() == 8) {
              	    std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
              	    newPreviousBoards[0] = this->board;
                  } else if(newPreviousBoards.size() < 8) {
              	    newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
                  }
                  int NewNmove = this->Nmove + 1;
                  int newCounterToDraw = 0;
                  newBoard[square] = empty;
                  newBoard[square + (7 * n)] = white_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), QUEEN, square, (6+n)));
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
                ChessBoard newBoard = oldBoard;
                std::array<int,2> newKingPositions = this->kingPositions;
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
                if(newPreviousBoards.size() == 8) {
              	  std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
              	  newPreviousBoards[0] = this->board;
                } else if(newPreviousBoards.size() < 8) {
              	  newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
                }
                int NewNmove = this->Nmove + 1;
                int newCounterToDraw = this->CounterToDraw + 1;
                newBoard[square] = empty;
                newBoard[square - (7 * n)] = white_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), QUEEN, square, (34+n)));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - (7 * n)]] == -1) {
                  ChessBoard newBoard = oldBoard;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
                  if(newPreviousBoards.size() == 8) {
              	    std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
              	    newPreviousBoards[0] = this->board;
                  } else if(newPreviousBoards.size() < 8) {
              	    newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
                  }
                  int NewNmove = this->Nmove + 1;
                  int newCounterToDraw = 0;
                  newBoard[square] = empty;
                  newBoard[square - (7 * n)] = white_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), QUEEN, square, (34+n)));
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
              i = (square / 8) + KNIGHT_JUMPS[n][1];
              j = (square % 8) + KNIGHT_JUMPS[n][0];
              if((i>=0) && (i<8) && (j>=0) && (j<8))
              {
                //And if it is empty or contains an enemy piece
                if(PIECES_COLORS[this->board[(8 * i) + j]] != 1) {
                  //It is possible to move there
                  ChessBoard newBoard = oldBoard;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
                  std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
                  if(newPreviousBoards.size() == 8) {
              	    std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
              	    newPreviousBoards[0] = this->board;
                  } else if(newPreviousBoards.size() < 8) {
              	    newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
                  }
                  int NewNmove = this->Nmove + 1;
                  int newCounterToDraw = this->CounterToDraw + 1;
                  if(PIECES_COLORS[this->board[(8 * i) + j]] == -1) {
                  	newCounterToDraw = 0;
                  }
                  newBoard[square] = empty;
                  newBoard[(8 * i) + j] = white_knight;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), KNIGHT, square, n));
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
                      ChessBoard newBoard = oldBoard;
                      std::array<int,2> newKingPositions = this->kingPositions;
                      std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
	                  std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
	                  if(newPreviousBoards.size() == 8) {
	              	    std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
	              	    newPreviousBoards[0] = this->board;
	                  } else if(newPreviousBoards.size() < 8) {
	              	    newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
	                  }
	                  int NewNmove = this->Nmove + 1;
	                  int newCounterToDraw = this->CounterToDraw + 1;
	                  if(PIECES_COLORS[this->board[square + (8 * i) + j]] == -1) {
	                  	newCounterToDraw = 0;
	                  }
                      newPossibleCastling[0][0] = 0;
                      newPossibleCastling[0][1] = 0;
                      newKingPositions[0] = square + (8 * i) + j;
                      newBoard[square] = empty;
                      newBoard[square + (8 * i) + j] = white_king;

                      //And if the move does not lead to check it a possible move
                      if(ChessState::isUnderAttack(newBoard, newKingPositions[0], -1 * this->player) == false)
                      {
                        possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), KING, square, ((3 * i) + j + 4)));
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
        switch(oldBoard[square]) {
          case black_pawn:
            //A pawn can either move ahead of one step not eating
            if((square >= 16) && (this->board[square - 8] == empty)) {
              ChessBoard newBoard = oldBoard;
              std::array<int,2> newKingPositions = this->kingPositions;
              std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
	          std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
	          if(newPreviousBoards.size() == 8) {
	          std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
	            newPreviousBoards[0] = this->board;
	          } else if(newPreviousBoards.size() < 8) {
	            newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
	          }
	          int NewNmove = this->Nmove + 1;
	          int newCounterToDraw = 0;
              newBoard[square] = empty;
              newBoard[square - 8] = black_pawn;

              //And if the move does not lead to check it a possible move
              if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
              {
                possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), PAWN, (63-square), 0));
              }
            } //Eventually getting promoted if it is moving to the last row
            else if((square >= 8) && (this->board[square - 8] == empty)) {
              ChessBoard newBoard = oldBoard;
              std::array<int,2> newKingPositions = this->kingPositions;
              std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
	          std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
	          if(newPreviousBoards.size() == 8) {
	          std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
	            newPreviousBoards[0] = this->board;
	          } else if(newPreviousBoards.size() < 8) {
	            newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
	          }
	          int NewNmove = this->Nmove + 1;
	          int newCounterToDraw = 0;
              newBoard[square] = empty;
              newBoard[square - 8] = black_knight;
              //And if the move does not lead to check it a possible move
              if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
              {
                possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), PAWN, (63-square), 9));
              }

              newBoard[square - 8] = black_queen;
              //And if the move does not lead to check it a possible move
              if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
              {
                possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), PAWN, (63-square), 6));
              }
            }

            //Or eat on the right and left
            if((square >= 16) && ((square % 8) < 7) && (PIECES_COLORS[this->board[square - 8 + 1]] == 1)) {
              ChessBoard newBoard = oldBoard;
              std::array<int,2> newKingPositions = this->kingPositions;
              std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
	          std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
	          if(newPreviousBoards.size() == 8) {
	          std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
	            newPreviousBoards[0] = this->board;
	          } else if(newPreviousBoards.size() < 8) {
	            newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
	          }
	          int NewNmove = this->Nmove + 1;
	          int newCounterToDraw = 0;
              newBoard[square] = empty;
              newBoard[square - 8 + 1] = black_pawn;

              //And if the move does not lead to check it a possible move
              if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
              {
                possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), PAWN, (63-square), 1));
              }
            } //Eventually getting promoted if it is moving to the last row
            else if((square >= 8) && ((square % 8) < 7) && (PIECES_COLORS[this->board[square - 8 + 1]] == 1)) {
              ChessBoard newBoard = oldBoard;
              std::array<int,2> newKingPositions = this->kingPositions;
              std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
	          std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
	          if(newPreviousBoards.size() == 8) {
	          std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
	            newPreviousBoards[0] = this->board;
	          } else if(newPreviousBoards.size() < 8) {
	            newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
	          }
	          int NewNmove = this->Nmove + 1;
	          int newCounterToDraw = 0;
              newBoard[square] = empty;
              newBoard[square - 8 + 1] = black_knight;
              //And if the move does not lead to check it a possible move
              if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
              {
                possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), PAWN, (63-square), 10));
              }

              newBoard[square - 8 + 1] = black_queen;
              //And if the move does not lead to check it a possible move
              if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
              {
                possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), PAWN, (63-square), 7));
              }
            }
            if((square >= 16) && ((square % 8) > 0) && (PIECES_COLORS[this->board[square - 8 - 1]] == 1)) {
              ChessBoard newBoard = oldBoard;
              std::array<int,2> newKingPositions = this->kingPositions;
              std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
	          std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
	          if(newPreviousBoards.size() == 8) {
	          std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
	            newPreviousBoards[0] = this->board;
	          } else if(newPreviousBoards.size() < 8) {
	            newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
	          }
	          int NewNmove = this->Nmove + 1;
	          int newCounterToDraw = 0;
              newBoard[square] = empty;
              newBoard[square - 8 - 1] = black_pawn;

              //And if the move does not lead to check it a possible move
              if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
              {
                possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), PAWN, (63-square), 2));
              }
            }  //Eventually getting promoted if it is moving to the last row
            else if((square >= 8) && ((square % 8) > 0) && (PIECES_COLORS[this->board[square - 8 - 1]] == 1)) {
              ChessBoard newBoard = oldBoard;
              std::array<int,2> newKingPositions = this->kingPositions;
              std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
	          std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
	          if(newPreviousBoards.size() == 8) {
	          std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
	            newPreviousBoards[0] = this->board;
	          } else if(newPreviousBoards.size() < 8) {
	            newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
	          }
	          int NewNmove = this->Nmove + 1;
	          int newCounterToDraw = 0;
              newBoard[square] = empty;
              newBoard[square - 8 - 1] = black_knight;
              //And if the move does not lead to check it a possible move
              if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
              {
                possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), PAWN, (63-square), 11));
              }

              newBoard[square - 8 - 1] = black_queen;
              //And if the move does not lead to check it a possible move
              if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
              {
                possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), PAWN, (63-square), 8));
              }
            }

            //If it is in his starting cell it can move ahead of two cells too
            if(((square / 8) == 6) && (this->board[square - 8] == empty) && (this->board[square - 16] == empty)) {
              ChessBoard newBoard = oldBoard;
              std::array<int,2> newKingPositions = this->kingPositions;
              std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
	          std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
	          if(newPreviousBoards.size() == 8) {
	          std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
	            newPreviousBoards[0] = this->board;
	          } else if(newPreviousBoards.size() < 8) {
	            newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
	          }
	          int NewNmove = this->Nmove + 1;
	          int newCounterToDraw = 0;
              newBoard[square] = empty;
              newBoard[square - 16] = black_pawn2;

              //And if the move does not lead to check it a possible move
              if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
              {
                possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), PAWN, (63-square), 3));
              }
            }

            //If it is in the 4th row, a capture en passant is possible
            if((square >= 24) && (square < 31)) {
              if((this->board[(square + 1)] == white_pawn2) && (this->board[square - 7] == empty)) {
                ChessBoard newBoard = oldBoard;
                std::array<int,2> newKingPositions = this->kingPositions;
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
	            std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
	            if(newPreviousBoards.size() == 8) {
	            std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
	              newPreviousBoards[0] = this->board;
	            } else if(newPreviousBoards.size() < 8) {
	              newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
	            }
	            int NewNmove = this->Nmove + 1;
	            int newCounterToDraw = 0;
                newBoard[square] = empty;
                newBoard[square + 1] = empty;
                newBoard[square - 7] = black_pawn;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), PAWN, (63-square), 4));
                }
              }
            }

            //If it is in the 4th row, a capture en passant is possible
            if((square > 24) && (square <= 31)) {
              if((this->board[(square - 1)] == white_pawn2) && (this->board[square - 9] == empty)) {
                ChessBoard newBoard = oldBoard;
                std::array<int,2> newKingPositions = this->kingPositions;
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
	            std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
	            if(newPreviousBoards.size() == 8) {
	            std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
	              newPreviousBoards[0] = this->board;
	            } else if(newPreviousBoards.size() < 8) {
	              newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
	            }
	            int NewNmove = this->Nmove + 1;
	            int newCounterToDraw = 0;
                newBoard[square] = empty;
                newBoard[square - 1] = empty;
                newBoard[square - 9] = black_pawn;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), PAWN, (63-square), 5));
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
                ChessBoard newBoard = oldBoard;
                std::array<int,2> newKingPositions = this->kingPositions;
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
	            std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
	            if(newPreviousBoards.size() == 8) {
	            std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
	              newPreviousBoards[0] = this->board;
	            } else if(newPreviousBoards.size() < 8) {
	              newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
	            }
	            int NewNmove = this->Nmove + 1;
	            int newCounterToDraw = this->CounterToDraw + 1;
                newBoard[square] = empty;
                newBoard[square + (9 * n)] = black_bishop;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), BISHOP, (63-square), (6+n)));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + (9 * n)]] == 1) {
                  ChessBoard newBoard = oldBoard;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
	              std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
	              if(newPreviousBoards.size() == 8) {
	              std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
	                newPreviousBoards[0] = this->board;
	              } else if(newPreviousBoards.size() < 8) {
	                newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
	              }
	              int NewNmove = this->Nmove + 1;
	              int newCounterToDraw = 0;
                  newBoard[square] = empty;
                  newBoard[square + (9 * n)] = black_bishop;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), BISHOP, (63-square), (6+n)));
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
                ChessBoard newBoard = oldBoard;
                std::array<int,2> newKingPositions = this->kingPositions;
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
	            std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
	            if(newPreviousBoards.size() == 8) {
	            std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
	              newPreviousBoards[0] = this->board;
	            } else if(newPreviousBoards.size() < 8) {
	              newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
	            }
	            int NewNmove = this->Nmove + 1;
	            int newCounterToDraw = this->CounterToDraw + 1;
                newBoard[square] = empty;
                newBoard[square - (9 * n)] = black_bishop;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), BISHOP, (63-square), (20+n)));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - (9 * n)]] == 1) {
                  ChessBoard newBoard = oldBoard;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
	              std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
	              if(newPreviousBoards.size() == 8) {
	              std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
	                newPreviousBoards[0] = this->board;
	              } else if(newPreviousBoards.size() < 8) {
	                newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
	              }
	              int NewNmove = this->Nmove + 1;
	              int newCounterToDraw = 0;
                  newBoard[square] = empty;
                  newBoard[square - (9 * n)] = black_bishop;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), BISHOP, (63-square), (20+n)));
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
                ChessBoard newBoard = oldBoard;
                std::array<int,2> newKingPositions = this->kingPositions;
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
	            std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
	            if(newPreviousBoards.size() == 8) {
	            std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
	              newPreviousBoards[0] = this->board;
	            } else if(newPreviousBoards.size() < 8) {
	              newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
	            }
	            int NewNmove = this->Nmove + 1;
	            int newCounterToDraw = this->CounterToDraw + 1;
                newBoard[square] = empty;
                newBoard[square + (7 * n)] = black_bishop;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), BISHOP, (63-square), (13+n)));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + (7 * n)]] == 1) {
                  ChessBoard newBoard = oldBoard;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
	              std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
	              if(newPreviousBoards.size() == 8) {
	              std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
	                newPreviousBoards[0] = this->board;
	              } else if(newPreviousBoards.size() < 8) {
	                newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
	              }
	              int NewNmove = this->Nmove + 1;
	              int newCounterToDraw = 0;
                  newBoard[square] = empty;
                  newBoard[square + (7 * n)] = black_bishop;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), BISHOP, (63-square), (13+n)));
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
                ChessBoard newBoard = oldBoard;
                std::array<int,2> newKingPositions = this->kingPositions;
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
	            std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
	            if(newPreviousBoards.size() == 8) {
	            std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
	              newPreviousBoards[0] = this->board;
	            } else if(newPreviousBoards.size() < 8) {
	              newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
	            }
	            int NewNmove = this->Nmove + 1;
	            int newCounterToDraw = this->CounterToDraw + 1;
                newBoard[square] = empty;
                newBoard[square - (7 * n)] = black_bishop;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), BISHOP, (63-square), (n-1)));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - (7 * n)]] == 1) {
                  ChessBoard newBoard = oldBoard;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
	              std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
	              if(newPreviousBoards.size() == 8) {
	              std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
	                newPreviousBoards[0] = this->board;
	              } else if(newPreviousBoards.size() < 8) {
	                newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
	              }
	              int NewNmove = this->Nmove + 1;
	              int newCounterToDraw = 0;
                  newBoard[square] = empty;
                  newBoard[square - (7 * n)] = black_bishop;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), BISHOP, (63-square), (n-1)));
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
                ChessBoard newBoard = oldBoard;
                std::array<int,2> newKingPositions = this->kingPositions;
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
	            std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
	            if(newPreviousBoards.size() == 8) {
	            std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
	              newPreviousBoards[0] = this->board;
	            } else if(newPreviousBoards.size() < 8) {
	              newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
	            }
	            int NewNmove = this->Nmove + 1;
	            int newCounterToDraw = this->CounterToDraw + 1;
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
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), ROOK, (63-square), (13+i)));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + (8 * i)]] == 1) {
                  ChessBoard newBoard = oldBoard;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
	              std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
	              if(newPreviousBoards.size() == 8) {
	              std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
	                newPreviousBoards[0] = this->board;
	              } else if(newPreviousBoards.size() < 8) {
	                newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
	              }
	              int NewNmove = this->Nmove + 1;
	              int newCounterToDraw = 0;
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
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), ROOK, (63-square), (13+i)));
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
                ChessBoard newBoard = oldBoard;
                std::array<int,2> newKingPositions = this->kingPositions;
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
	            std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
	            if(newPreviousBoards.size() == 8) {
	            std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
	              newPreviousBoards[0] = this->board;
	            } else if(newPreviousBoards.size() < 8) {
	              newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
	            }
	            int NewNmove = this->Nmove + 1;
	            int newCounterToDraw = this->CounterToDraw + 1;
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
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), ROOK, (63-square), (i-1)));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - (8 * i)]] == 1) {
                  ChessBoard newBoard = oldBoard;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
	              std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
	              if(newPreviousBoards.size() == 8) {
	              std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
	                newPreviousBoards[0] = this->board;
	              } else if(newPreviousBoards.size() < 8) {
	                newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
	              }
	              int NewNmove = this->Nmove + 1;
	              int newCounterToDraw = 0;
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
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), ROOK, (63-square), (i-1)));
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
                ChessBoard newBoard = oldBoard;
                std::array<int,2> newKingPositions = this->kingPositions;
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
	            std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
	            if(newPreviousBoards.size() == 8) {
	            std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
	              newPreviousBoards[0] = this->board;
	            } else if(newPreviousBoards.size() < 8) {
	              newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
	            }
	            int NewNmove = this->Nmove + 1;
	            int newCounterToDraw = this->CounterToDraw + 1;
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
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), ROOK, (63-square), (6+j)));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + j]] == 1) {
                  ChessBoard newBoard = oldBoard;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
	              std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
	              if(newPreviousBoards.size() == 8) {
	              std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
	                newPreviousBoards[0] = this->board;
	              } else if(newPreviousBoards.size() < 8) {
	                newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
	              }
	              int NewNmove = this->Nmove + 1;
	              int newCounterToDraw = 0;
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
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), ROOK, (63-square), (6+j)));
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
                ChessBoard newBoard = oldBoard;
                std::array<int,2> newKingPositions = this->kingPositions;
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
	            std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
	            if(newPreviousBoards.size() == 8) {
	            std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
	              newPreviousBoards[0] = this->board;
	            } else if(newPreviousBoards.size() < 8) {
	              newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
	            }
	            int NewNmove = this->Nmove + 1;
	            int newCounterToDraw = this->CounterToDraw + 1;
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
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), ROOK, (63-square), (20+j)));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - j]] == 1) {
                  ChessBoard newBoard = oldBoard;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
	              std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
	              if(newPreviousBoards.size() == 8) {
	              std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
	                newPreviousBoards[0] = this->board;
	              } else if(newPreviousBoards.size() < 8) {
	                newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
	              }
	              int NewNmove = this->Nmove + 1;
	              int newCounterToDraw = 0;
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
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), ROOK, (63-square), (20+j)));
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
                ChessBoard newBoard = oldBoard;
                std::array<int,2> newKingPositions = this->kingPositions;
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
	            std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
	            if(newPreviousBoards.size() == 8) {
	            std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
	              newPreviousBoards[0] = this->board;
	            } else if(newPreviousBoards.size() < 8) {
	              newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
	            }
	            int NewNmove = this->Nmove + 1;
	            int newCounterToDraw = this->CounterToDraw + 1;
                newBoard[square] = empty;
                newBoard[square + (8 * i)] = black_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), QUEEN, (63-square), (27+i)));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + (8 * i)]] == 1) {
                  ChessBoard newBoard = oldBoard;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
	              std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
	              if(newPreviousBoards.size() == 8) {
	              std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
	                newPreviousBoards[0] = this->board;
	              } else if(newPreviousBoards.size() < 8) {
	                newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
	              }
	              int NewNmove = this->Nmove + 1;
	              int newCounterToDraw = 0;
                  newBoard[square] = empty;
                  newBoard[square + (8 * i)] = black_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), QUEEN, (63-square), (27+i)));
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
                ChessBoard newBoard = oldBoard;
                std::array<int,2> newKingPositions = this->kingPositions;
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
	            std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
	            if(newPreviousBoards.size() == 8) {
	            std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
	              newPreviousBoards[0] = this->board;
	            } else if(newPreviousBoards.size() < 8) {
	              newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
	            }
	            int NewNmove = this->Nmove + 1;
	            int newCounterToDraw = this->CounterToDraw + 1;
                newBoard[square] = empty;
                newBoard[square - (8 * i)] = black_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), QUEEN, (63-square), (i-1)));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - (8 * i)]] == 1) {
                  ChessBoard newBoard = oldBoard;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
	              std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
	              if(newPreviousBoards.size() == 8) {
	              std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
	                newPreviousBoards[0] = this->board;
	              } else if(newPreviousBoards.size() < 8) {
	                newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
	              }
	              int NewNmove = this->Nmove + 1;
	              int newCounterToDraw = 0;
                  newBoard[square] = empty;
                  newBoard[square - (8 * i)] = black_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), QUEEN, (63-square), (i-1)));
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
                ChessBoard newBoard = oldBoard;
                std::array<int,2> newKingPositions = this->kingPositions;
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
	            std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
	            if(newPreviousBoards.size() == 8) {
	            std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
	              newPreviousBoards[0] = this->board;
	            } else if(newPreviousBoards.size() < 8) {
	              newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
	            }
	            int NewNmove = this->Nmove + 1;
	            int newCounterToDraw = this->CounterToDraw + 1;
                newBoard[square] = empty;
                newBoard[square + j] = black_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), QUEEN, (63-square), (13+j)));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + j]] == 1) {
                  ChessBoard newBoard = oldBoard;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
	              std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
	              if(newPreviousBoards.size() == 8) {
	              std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
	                newPreviousBoards[0] = this->board;
	              } else if(newPreviousBoards.size() < 8) {
	                newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
	              }
	              int NewNmove = this->Nmove + 1;
	              int newCounterToDraw = 0;
                  newBoard[square] = empty;
                  newBoard[square + j] = black_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), QUEEN, (63-square), (13+j)));
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
                ChessBoard newBoard = oldBoard;
                std::array<int,2> newKingPositions = this->kingPositions;
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
	            std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
	            if(newPreviousBoards.size() == 8) {
	            std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
	              newPreviousBoards[0] = this->board;
	            } else if(newPreviousBoards.size() < 8) {
	              newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
	            }
	            int NewNmove = this->Nmove + 1;
	            int newCounterToDraw = this->CounterToDraw + 1;
                newBoard[square] = empty;
                newBoard[square - j] = black_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), QUEEN, (63-square), (41+j)));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - j]] == 1) {
                  ChessBoard newBoard = oldBoard;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
	              std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
	              if(newPreviousBoards.size() == 8) {
	              std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
	                newPreviousBoards[0] = this->board;
	              } else if(newPreviousBoards.size() < 8) {
	                newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
	              }
	              int NewNmove = this->Nmove + 1;
	              int newCounterToDraw = 0;
                  newBoard[square] = empty;
                  newBoard[square - j] = black_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), QUEEN, (63-square), (41+j)));
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
                ChessBoard newBoard = oldBoard;
                std::array<int,2> newKingPositions = this->kingPositions;
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
	            std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
	            if(newPreviousBoards.size() == 8) {
	            std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
	              newPreviousBoards[0] = this->board;
	            } else if(newPreviousBoards.size() < 8) {
	              newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
	            }
	            int NewNmove = this->Nmove + 1;
	            int newCounterToDraw = this->CounterToDraw + 1;
                newBoard[square] = empty;
                newBoard[square + (9 * n)] = black_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), QUEEN, (63-square), (20+n)));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + (9 * n)]] == 1) {
                  ChessBoard newBoard = oldBoard;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
	              std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
	              if(newPreviousBoards.size() == 8) {
	              std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
	                newPreviousBoards[0] = this->board;
	              } else if(newPreviousBoards.size() < 8) {
	                newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
	              }
	              int NewNmove = this->Nmove + 1;
	              int newCounterToDraw = 0;
                  newBoard[square] = empty;
                  newBoard[square + (9 * n)] = black_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), QUEEN, (63-square), (20+n)));
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
                ChessBoard newBoard = oldBoard;
                std::array<int,2> newKingPositions = this->kingPositions;
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
	            std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
	            if(newPreviousBoards.size() == 8) {
	            std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
	              newPreviousBoards[0] = this->board;
	            } else if(newPreviousBoards.size() < 8) {
	              newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
	            }
	            int NewNmove = this->Nmove + 1;
	            int newCounterToDraw = this->CounterToDraw + 1;
                newBoard[square] = empty;
                newBoard[square - (9 * n)] = black_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), QUEEN, (63-square), (48+n)));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - (9 * n)]] == 1) {
                  ChessBoard newBoard = oldBoard;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
	              std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
	              if(newPreviousBoards.size() == 8) {
	              std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
	                newPreviousBoards[0] = this->board;
	              } else if(newPreviousBoards.size() < 8) {
	                newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
	              }
	              int NewNmove = this->Nmove + 1;
	              int newCounterToDraw = 0;
                  newBoard[square] = empty;
                  newBoard[square - (9 * n)] = black_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), QUEEN, (63-square), (48+n)));
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
                ChessBoard newBoard = oldBoard;
                std::array<int,2> newKingPositions = this->kingPositions;
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
	            std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
	            if(newPreviousBoards.size() == 8) {
	            std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
	              newPreviousBoards[0] = this->board;
	            } else if(newPreviousBoards.size() < 8) {
	              newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
	            }
	            int NewNmove = this->Nmove + 1;
	            int newCounterToDraw = this->CounterToDraw + 1;
                newBoard[square] = empty;
                newBoard[square + (7 * n)] = black_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), QUEEN, (63-square), (34+n)));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square + (7 * n)]] == 1) {
                  ChessBoard newBoard = oldBoard;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
	              std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
	              if(newPreviousBoards.size() == 8) {
	              std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
	                newPreviousBoards[0] = this->board;
	              } else if(newPreviousBoards.size() < 8) {
	                newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
	              }
	              int NewNmove = this->Nmove + 1;
	              int newCounterToDraw = 0;
                  newBoard[square] = empty;
                  newBoard[square + (7 * n)] = black_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), QUEEN, (63-square), (34+n)));
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
                ChessBoard newBoard = oldBoard;
                std::array<int,2> newKingPositions = this->kingPositions;
                std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
	            std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
	            if(newPreviousBoards.size() == 8) {
	            std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
	              newPreviousBoards[0] = this->board;
	            } else if(newPreviousBoards.size() < 8) {
	              newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
	            }
	            int NewNmove = this->Nmove + 1;
	            int newCounterToDraw = this->CounterToDraw + 1;
                newBoard[square] = empty;
                newBoard[square - (7 * n)] = black_queen;

                //And if the move does not lead to check it a possible move
                if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                {
                  possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), QUEEN, (63-square), (6+n)));
                }
              }
              else 
              {
                check = 1;
                if(PIECES_COLORS[this->board[square - (7 * n)]] == 1) {
                  ChessBoard newBoard = oldBoard;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
	              std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
	              if(newPreviousBoards.size() == 8) {
	              std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
	                newPreviousBoards[0] = this->board;
	              } else if(newPreviousBoards.size() < 8) {
	                newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
	              }
	              int NewNmove = this->Nmove + 1;
	              int newCounterToDraw = this->CounterToDraw + 1;
                  newBoard[square] = empty;
                  newBoard[square - (7 * n)] = black_queen;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), QUEEN, (63-square), (6+n)));
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
              i = (square / 8) - KNIGHT_JUMPS[n][1];
              j = (square % 8) - KNIGHT_JUMPS[n][0];
              if((i>=0) && (i<8) && (j>=0) && (j<8))
              {
                //And if it is empty or contains an enemy piece
                if(PIECES_COLORS[this->board[(8 * i) + j]] != -1) {
                  //It is possible to move there
                  ChessBoard newBoard = oldBoard;
                  std::array<int,2> newKingPositions = this->kingPositions;
                  std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
	              std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
	              if(newPreviousBoards.size() == 8) {
	              std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
	                newPreviousBoards[0] = this->board;
	              } else if(newPreviousBoards.size() < 8) {
	                newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
	              }
	              int NewNmove = this->Nmove + 1;
	              int newCounterToDraw = this->CounterToDraw + 1;
	              if(PIECES_COLORS[this->board[(8 * i) + j]] == 1) {
	              	newCounterToDraw = 0;
	              }
                  newBoard[square] = empty;
                  newBoard[(8 * i) + j] = black_knight;

                  //And if the move does not lead to check it a possible move
                  if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                  {
                    possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), KNIGHT, (63-square), n));
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
                  if((((square / 8) - i)>=0) && (((square / 8) - i)<8) && (((square % 8) - j)>=0) && (((square % 8) - j)<8))
                  {
                    //And if it is empty or contains an enemy piece
                    if(PIECES_COLORS[this->board[square - (8 * i) - j]] != -1) {
                      //The move is possible
                      ChessBoard newBoard = oldBoard;
                      std::array<int,2> newKingPositions = this->kingPositions;
                      std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
		              std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
		              if(newPreviousBoards.size() == 8) {
		              std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
		                newPreviousBoards[0] = this->board;
		              } else if(newPreviousBoards.size() < 8) {
		                newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
		              }
		              int NewNmove = this->Nmove + 1;
		              int newCounterToDraw = this->CounterToDraw + 1;
		              if(PIECES_COLORS[this->board[square - (8 * i) - j]] == 1) {
		              	newCounterToDraw = 0;
		              }
                      newPossibleCastling[1][0] = 0;
                      newPossibleCastling[1][1] = 0;
                      newKingPositions[1] = square - (8 * i) - j;
                      newBoard[square] = empty;
                      newBoard[square - (8 * i) - j] = black_king;

                      //And if the move does not lead to check it a possible move
                      if(ChessState::isUnderAttack(newBoard, newKingPositions[1], -1 * this->player) == false)
                      {
                        possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), KING, (63-square), ((3*i)+j+4)));
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
        if((this->board[0] == white_rook) && (this->board[1] == empty) && (this->board[2] == empty) && (this->board[3] == empty) && (this->board[4] == white_king)) {
          if((ChessState::isUnderAttack(this->board, 2, -1) == false) && (ChessState::isUnderAttack(this->board, 3, -1) == false) && (ChessState::isUnderAttack(this->board, 4, -1) == false)) {
            //The castling is possible
            ChessBoard newBoard = oldBoard;
            std::array<int,2> newKingPositions = this->kingPositions;
            std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
		    std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
		    if(newPreviousBoards.size() == 8) {
		      std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
		      newPreviousBoards[0] = this->board;
		    } else if(newPreviousBoards.size() < 8) {
		      newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
		    }
		    int NewNmove = this->Nmove + 1;
		    int newCounterToDraw = this->CounterToDraw + 1;
            newPossibleCastling[0][0] = 0;
            newPossibleCastling[0][1] = 0;
            newKingPositions[0] = 2;
            newBoard[0] = empty;
            newBoard[4] = empty;
            newBoard[2] = white_king;
            newBoard[3] = white_rook;
            possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), KING, 4, 4));
          }
        }
      }
      if(possibleCastling[0][1]) {
        if((this->board[4] == white_king) && (this->board[5] == empty) && (this->board[6] == empty) && (this->board[7] == white_rook)) {
          if((ChessState::isUnderAttack(this->board, 4, -1) == false) && (ChessState::isUnderAttack(this->board, 5, -1) == false) && (ChessState::isUnderAttack(this->board, 6, -1) == false)) {
            //The castling is possible
            ChessBoard newBoard = oldBoard;
            std::array<int,2> newKingPositions = this->kingPositions;
            std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
		    std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
		    if(newPreviousBoards.size() == 8) {
		      std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
		      newPreviousBoards[0] = this->board;
		    } else if(newPreviousBoards.size() < 8) {
		      newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
		    }
		    int NewNmove = this->Nmove + 1;
		    int newCounterToDraw = this->CounterToDraw + 1;
            newPossibleCastling[0][0] = 0;
            newPossibleCastling[0][1] = 0;
            newKingPositions[0] = 6;
            newBoard[7] = empty;
            newBoard[4] = empty;
            newBoard[6] = white_king;
            newBoard[5] = white_rook;
            possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), KING, 4, 9));
          }
        }
      }
    }
    else {
      if(possibleCastling[1][0]) {
        if((this->board[56] == black_rook) && (this->board[57] == empty) && (this->board[58] == empty) && (this->board[59] == empty) && (this->board[60] == black_king)) {
          if((ChessState::isUnderAttack(this->board, 58, 1) == false) && (ChessState::isUnderAttack(this->board, 59, 1) == false) && (ChessState::isUnderAttack(this->board, 60, 1) == false)) {
            //The castling is possible
            ChessBoard newBoard = oldBoard;
            std::array<int,2> newKingPositions = this->kingPositions;
            std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
		    std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
		    if(newPreviousBoards.size() == 8) {
		      std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
		      newPreviousBoards[0] = this->board;
		    } else if(newPreviousBoards.size() < 8) {
		      newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
		    }
		    int NewNmove = this->Nmove + 1;
		    int newCounterToDraw = this->CounterToDraw + 1;
            newPossibleCastling[1][0] = 0;
            newPossibleCastling[1][1] = 0;
            newKingPositions[1] = 58;
            newBoard[56] = empty;
            newBoard[60] = empty;
            newBoard[58] = black_king;
            newBoard[59] = black_rook;
            possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), KING, 3, 4));
          }
        }
      }
      if(possibleCastling[1][1]) {
        if((this->board[60] == black_king) && (this->board[61] == empty) && (this->board[62] == empty) && (this->board[63] == black_rook)) {
          if((ChessState::isUnderAttack(this->board, 60, 1) == false) && (ChessState::isUnderAttack(this->board, 61, 1) == false) && (ChessState::isUnderAttack(this->board, 62, 1) == false)) {
            //The castling is possible
            ChessBoard newBoard = oldBoard;
            std::array<int,2> newKingPositions = this->kingPositions;
            std::array<std::array<int,2>,2> newPossibleCastling = this->possibleCastling;
		    std::vector<ChessBoard> newPreviousBoards = this->previousBoards;
		    if(newPreviousBoards.size() == 8) {
		      std::rotate(newPreviousBoards.begin(),newPreviousBoards.end()-1,newPreviousBoards.end());
		      newPreviousBoards[0] = this->board;
		    } else if(newPreviousBoards.size() < 8) {
		      newPreviousBoards.insert(newPreviousBoards.begin(), this->board);
		    }
		    int NewNmove = this->Nmove + 1;
		    int newCounterToDraw = this->CounterToDraw + 1;
            newPossibleCastling[1][0] = 0;
            newPossibleCastling[1][1] = 0;
            newKingPositions[1] = 62;
            newBoard[63] = empty;
            newBoard[60] = empty;
            newBoard[62] = white_king;
            newBoard[61] = white_rook;
            possibleMoves.push_back(ChessMove(new ChessState(newBoard, newPreviousBoards, newKingPositions, newPossibleCastling, NewNmove, newCounterToDraw, (-1 * this->player)), KING, 3, 9));
          }
        }
      }
    }

    return possibleMoves;
}




std::vector<double> ChessState::getFirstNetworkInput(void) {
  std::vector<double> netInput;

  if(this->player == 1) {
      for(int square=0;square<64;square++) {
          if((this->board[square] == white_pawn) || (this->board[square] == white_pawn2)) {
              netInput.push_back(1);
          }
          else if ((this->board[square] == black_pawn) || (this->board[square] == black_pawn2)){
              netInput.push_back(-1);
          }
          else {
              netInput.push_back(0);
          }
      }
      for(int square=0;square<64;square++) {
          if(this->board[square] == white_rook) {
              netInput.push_back(1);
          }
          else if(this->board[square] == black_rook) {
              netInput.push_back(-1);
          }
          else{
              netInput.push_back(0);
          }
      }
      for(int square=0;square<64;square++) {
          if(this->board[square] == white_knight) {
              netInput.push_back(1);
          }
          else if(this->board[square] == black_knight) {
              netInput.push_back(-1);
          }
          else{
              netInput.push_back(0);
          }
      }
      for(int square=0;square<64;square++) {
          if(this->board[square] == white_bishop) {
              netInput.push_back(1);
          }
          else if(this->board[square] == black_bishop) {
              netInput.push_back(-1);
          }
          else{
              netInput.push_back(0);
          }
      }
      for(int square=0;square<64;square++) {
          if(this->board[square] == white_queen) {
              netInput.push_back(1);
          }
          else if(this->board[square] == black_queen) {
              netInput.push_back(-1);
          }
          else{
              netInput.push_back(0);
          }
      }
      for(int square=0;square<64;square++) {
          if(this->board[square] == white_king) {
              netInput.push_back(1);
          }
          else if(this->board[square] == black_king) {
              netInput.push_back(-1);
          }
          else{
              netInput.push_back(0);
          }
      }

      for(int col=0;col<8;col++) {
        if(this->board[(32+col)] == black_pawn2) {
          netInput.push_back(1);
        }
        else {
          netInput.push_back(0);
        }
      }
      
      netInput.push_back(this->possibleCastling[0][0]);
      netInput.push_back(this->possibleCastling[0][1]);
      
      netInput.push_back(this->possibleCastling[1][0]);
      netInput.push_back(this->possibleCastling[1][1]);
      
      netInput.push_back(0);
      
      netInput.push_back(this->Repetition);
      
      netInput.push_back(this->Nmove);
  } else {
      for(int square=0;square<64;square++) {
          if((this->board[(63 - square)] == black_pawn) || (this->board[(63 - square)] == black_pawn2)) {
              netInput.push_back(1);
          }
          else if((this->board[(63 - square)] == white_pawn) || (this->board[(63 - square)] == white_pawn2)) {
              netInput.push_back(-1);
          }
          else{
              netInput.push_back(0);
          }
      }
      for(int square=0;square<64;square++) {
          if(this->board[(63 - square)] == black_rook) {
              netInput.push_back(1);
          }
          else if(this->board[(63 - square)] == white_rook) {
              netInput.push_back(-1);
          }
          else{
              netInput.push_back(0);
          }
      }
      for(int square=0;square<64;square++) {
          if(this->board[(63 - square)] == black_knight) {
              netInput.push_back(1);
          }
          else if(this->board[(63 - square)] == white_knight) {
              netInput.push_back(-1);
          }
          else{
              netInput.push_back(0);
          }
      }
      for(int square=0;square<64;square++) {
          if(this->board[(63 - square)] == black_bishop) {
              netInput.push_back(1);
          }
          else if(this->board[(63 - square)] == white_bishop) {
              netInput.push_back(-1);
          }
          else{
              netInput.push_back(0);
          }
      }
      for(int square=0;square<64;square++) {
          if(this->board[(63 - square)] == black_queen) {
              netInput.push_back(1);
          }
          else if(this->board[(63 - square)] == white_queen) {
              netInput.push_back(-1);
          }
          else{
              netInput.push_back(0);
          }
      }
      for(int square=0;square<64;square++) {
          if(this->board[(63 - square)] == black_king) {
              netInput.push_back(1);
          }
          else if(this->board[(63 - square)] == white_king) {
              netInput.push_back(-1);
          }
          else{
              netInput.push_back(0);
          }
      }

      for(int col=0;col<8;col++) {
        if(this->board[(31-col)] == white_pawn2) {
          netInput.push_back(1);
        }
        else {
          netInput.push_back(0);
        }
      }
      
      netInput.push_back(this->possibleCastling[1][0]);
      netInput.push_back(this->possibleCastling[1][1]);
      
      netInput.push_back(this->possibleCastling[0][0]);
      netInput.push_back(this->possibleCastling[0][1]);
      
      netInput.push_back(1);
      
      netInput.push_back(this->Repetition);
      
      netInput.push_back(this->Nmove);
  }

  return netInput;
}




std::vector<double> ChessState::getSecondNetworkInput(int square0) {
  std::vector<double> netInput;

  if(this->player == 1) {
      for(int square=0;square<64;square++) {
          if((this->board[square] == white_pawn) || (this->board[square] == white_pawn2)) {
              netInput.push_back(1);
          }
          else if ((this->board[square] == black_pawn) || (this->board[square] == black_pawn2)){
              netInput.push_back(-1);
          }
          else {
              netInput.push_back(0);
          }
      }
      for(int square=0;square<64;square++) {
          if(this->board[square] == white_rook) {
              netInput.push_back(1);
          }
          else if(this->board[square] == black_rook) {
              netInput.push_back(-1);
          }
          else{
              netInput.push_back(0);
          }
      }
      for(int square=0;square<64;square++) {
          if(this->board[square] == white_knight) {
              netInput.push_back(1);
          }
          else if(this->board[square] == black_knight) {
              netInput.push_back(-1);
          }
          else{
              netInput.push_back(0);
          }
      }
      for(int square=0;square<64;square++) {
          if(this->board[square] == white_bishop) {
              netInput.push_back(1);
          }
          else if(this->board[square] == black_bishop) {
              netInput.push_back(-1);
          }
          else{
              netInput.push_back(0);
          }
      }
      for(int square=0;square<64;square++) {
          if(this->board[square] == white_queen) {
              netInput.push_back(1);
          }
          else if(this->board[square] == black_queen) {
              netInput.push_back(-1);
          }
          else{
              netInput.push_back(0);
          }
      }
      for(int square=0;square<64;square++) {
          if(this->board[square] == white_king) {
              netInput.push_back(1);
          }
          else if(this->board[square] == black_king) {
              netInput.push_back(-1);
          }
          else{
              netInput.push_back(0);
          }
      }

      for(int col=0;col<8;col++) {
        if(this->board[(32+col)] == black_pawn2) {
          netInput.push_back(1);
        }
        else {
          netInput.push_back(0);
        }
      }
      
      netInput.push_back(this->possibleCastling[0][0]);
      netInput.push_back(this->possibleCastling[0][1]);
      
      netInput.push_back(this->possibleCastling[1][0]);
      netInput.push_back(this->possibleCastling[1][1]);
      
      netInput.push_back(0);
      
      netInput.push_back(0);

      for(int square=0;square<64;square++) {
          if(square == square0) {
              netInput.push_back(1);
          }
          else{
              netInput.push_back(0);
          }
      }
  } else {
      for(int square=0;square<64;square++) {
          if((this->board[(63 - square)] == black_pawn) || (this->board[(63 - square)] == black_pawn2)) {
              netInput.push_back(1);
          }
          else if((this->board[(63 - square)] == white_pawn) || (this->board[(63 - square)] == white_pawn2)) {
              netInput.push_back(-1);
          }
          else{
              netInput.push_back(0);
          }
      }
      for(int square=0;square<64;square++) {
          if(this->board[(63 - square)] == black_rook) {
              netInput.push_back(1);
          }
          else if(this->board[(63 - square)] == white_rook) {
              netInput.push_back(-1);
          }
          else{
              netInput.push_back(0);
          }
      }
      for(int square=0;square<64;square++) {
          if(this->board[(63 - square)] == black_knight) {
              netInput.push_back(1);
          }
          else if(this->board[(63 - square)] == white_knight) {
              netInput.push_back(-1);
          }
          else{
              netInput.push_back(0);
          }
      }
      for(int square=0;square<64;square++) {
          if(this->board[(63 - square)] == black_bishop) {
              netInput.push_back(1);
          }
          else if(this->board[(63 - square)] == white_bishop) {
              netInput.push_back(-1);
          }
          else{
              netInput.push_back(0);
          }
      }
      for(int square=0;square<64;square++) {
          if(this->board[(63 - square)] == black_queen) {
              netInput.push_back(1);
          }
          else if(this->board[(63 - square)] == white_queen) {
              netInput.push_back(-1);
          }
          else{
              netInput.push_back(0);
          }
      }
      for(int square=0;square<64;square++) {
          if(this->board[(63 - square)] == black_king) {
              netInput.push_back(1);
          }
          else if(this->board[(63 - square)] == white_king) {
              netInput.push_back(-1);
          }
          else{
              netInput.push_back(0);
          }
      }

      for(int col=0;col<8;col++) {
        if(this->board[(31-col)] == white_pawn2) {
          netInput.push_back(1);
        }
        else {
          netInput.push_back(0);
        }
      }
      
      netInput.push_back(this->possibleCastling[1][0]);
      netInput.push_back(this->possibleCastling[1][1]);
      
      netInput.push_back(this->possibleCastling[0][0]);
      netInput.push_back(this->possibleCastling[0][1]);
      
      netInput.push_back(1);
      
      netInput.push_back(0);

      for(int square=0;square<64;square++) {
          if(square == square0) {
              netInput.push_back(1);
          }
          else{
              netInput.push_back(0);
          }
      }
  }

  return netInput;
}



void ChessState::printState(void) {
    for(int i=7;i>=0;i--) {
        for(int j=0;j<8;j++) {
            std::cout << "|" << PIECES_SYMBOLS[this->board[(8*i)+j]] << "|";
        }
        std::cout << "\n";
    }
    std::cout << "\n\n";

    std::cout << "White king position:" << this->kingPositions[0] << "\n";
    std::cout << "Black king position:" << this->kingPositions[1] << "\n";
    std::cout << "White possible castlings: left " << this->possibleCastling[0][0] << ", right " << this->possibleCastling[0][1] << "\n";
    std::cout << "Black possible castlings: left " << this->possibleCastling[1][0] << ", right " << this->possibleCastling[1][1] << "\n";
    std::cout << "Size of previous boards: " << this->previousBoards.size() << "\n\n\n";
    std::cout << "Repetitions: " << this->Repetition << "\n\n\n";
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
        if((board[square + (9 * n)] == white_king) || (board[square + (9 * n)] == black_king) || (board[square + (9 * n)] == black_pawn) || (board[square + (9 * n)] == black_pawn2)) {
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
        if((board[square - (9 * n)] == white_king) || (board[square - (9 * n)] == black_king) || (board[square - (9 * n)] == white_pawn) || (board[square - (9 * n)] == white_pawn2)) {
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
        if((board[square + (7 * n)] == white_king) || (board[square + (7 * n)] == black_king) || (board[square + (7 * n)] == black_pawn) || (board[square + (7 * n)] == black_pawn2)) {
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
        if((board[square - (7 * n)] == white_king) || (board[square - (7 * n)] == black_king) || (board[square - (7 * n)] == white_pawn) || (board[square - (7 * n)] == white_pawn2)) {
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
    int i = (square / 8) + KNIGHT_JUMPS[n][1];
    int j = (square % 8) + KNIGHT_JUMPS[n][0];
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
    for(square1=0;square1<64;square1++) {
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
    for(square1=0;square1<64;square1++) {
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
    for(square1=0;square1<64;square1++) {
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
    for(square1=0;square1<64;square1++) {
    	//For every of the possible jumping squares
    	for(n=0;n<8;n++)
        {
            //If the jumping cell is a real cell
            i = (square1 / 8) + KNIGHT_JUMPS[n][0];
            j = (square1 % 8) + KNIGHT_JUMPS[n][1];

            if((i>=0) && (i<8) && (j>=0) && (j<8))
            {
                //Add the move
                square2 = (8 * i) + j;
	    	    moveNames.push_back("N" + std::to_string(square1) + "N" + std::to_string(square2));
            }
	   	}
    }

    //And, in the end, consider the king
    //From every starting square
    for(square1=0;square1<64;square1++) {
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
    {"B0B9", 228},
    {"B0B18", 229},
    {"B0B27", 230},
    {"B0B36", 231},
    {"B0B45", 232},
    {"B0B54", 233},
    {"B0B63", 234},
    {"B1B10", 235},
    {"B1B19", 236},
    {"B1B28", 237},
    {"B1B37", 238},
    {"B1B46", 239},
    {"B1B55", 240},
    {"B1B8", 241},
    {"B2B11", 242},
    {"B2B20", 243},
    {"B2B29", 244},
    {"B2B38", 245},
    {"B2B47", 246},
    {"B2B9", 247},
    {"B2B16", 248},
    {"B3B12", 249},
    {"B3B21", 250},
    {"B3B30", 251},
    {"B3B39", 252},
    {"B3B10", 253},
    {"B3B17", 254},
    {"B3B24", 255},
    {"B4B13", 256},
    {"B4B22", 257},
    {"B4B31", 258},
    {"B4B11", 259},
    {"B4B18", 260},
    {"B4B25", 261},
    {"B4B32", 262},
    {"B5B14", 263},
    {"B5B23", 264},
    {"B5B12", 265},
    {"B5B19", 266},
    {"B5B26", 267},
    {"B5B33", 268},
    {"B5B40", 269},
    {"B6B15", 270},
    {"B6B13", 271},
    {"B6B20", 272},
    {"B6B27", 273},
    {"B6B34", 274},
    {"B6B41", 275},
    {"B6B48", 276},
    {"B7B14", 277},
    {"B7B21", 278},
    {"B7B28", 279},
    {"B7B35", 280},
    {"B7B42", 281},
    {"B7B49", 282},
    {"B7B56", 283},
    {"B8B17", 284},
    {"B8B26", 285},
    {"B8B35", 286},
    {"B8B44", 287},
    {"B8B53", 288},
    {"B8B62", 289},
    {"B8B1", 290},
    {"B9B18", 291},
    {"B9B27", 292},
    {"B9B36", 293},
    {"B9B45", 294},
    {"B9B54", 295},
    {"B9B63", 296},
    {"B9B0", 297},
    {"B9B16", 298},
    {"B9B2", 299},
    {"B10B19", 300},
    {"B10B28", 301},
    {"B10B37", 302},
    {"B10B46", 303},
    {"B10B55", 304},
    {"B10B1", 305},
    {"B10B17", 306},
    {"B10B24", 307},
    {"B10B3", 308},
    {"B11B20", 309},
    {"B11B29", 310},
    {"B11B38", 311},
    {"B11B47", 312},
    {"B11B2", 313},
    {"B11B18", 314},
    {"B11B25", 315},
    {"B11B32", 316},
    {"B11B4", 317},
    {"B12B21", 318},
    {"B12B30", 319},
    {"B12B39", 320},
    {"B12B3", 321},
    {"B12B19", 322},
    {"B12B26", 323},
    {"B12B33", 324},
    {"B12B40", 325},
    {"B12B5", 326},
    {"B13B22", 327},
    {"B13B31", 328},
    {"B13B4", 329},
    {"B13B20", 330},
    {"B13B27", 331},
    {"B13B34", 332},
    {"B13B41", 333},
    {"B13B48", 334},
    {"B13B6", 335},
    {"B14B23", 336},
    {"B14B5", 337},
    {"B14B21", 338},
    {"B14B28", 339},
    {"B14B35", 340},
    {"B14B42", 341},
    {"B14B49", 342},
    {"B14B56", 343},
    {"B14B7", 344},
    {"B15B6", 345},
    {"B15B22", 346},
    {"B15B29", 347},
    {"B15B36", 348},
    {"B15B43", 349},
    {"B15B50", 350},
    {"B15B57", 351},
    {"B16B25", 352},
    {"B16B34", 353},
    {"B16B43", 354},
    {"B16B52", 355},
    {"B16B61", 356},
    {"B16B9", 357},
    {"B16B2", 358},
    {"B17B26", 359},
    {"B17B35", 360},
    {"B17B44", 361},
    {"B17B53", 362},
    {"B17B62", 363},
    {"B17B8", 364},
    {"B17B24", 365},
    {"B17B10", 366},
    {"B17B3", 367},
    {"B18B27", 368},
    {"B18B36", 369},
    {"B18B45", 370},
    {"B18B54", 371},
    {"B18B63", 372},
    {"B18B9", 373},
    {"B18B0", 374},
    {"B18B25", 375},
    {"B18B32", 376},
    {"B18B11", 377},
    {"B18B4", 378},
    {"B19B28", 379},
    {"B19B37", 380},
    {"B19B46", 381},
    {"B19B55", 382},
    {"B19B10", 383},
    {"B19B1", 384},
    {"B19B26", 385},
    {"B19B33", 386},
    {"B19B40", 387},
    {"B19B12", 388},
    {"B19B5", 389},
    {"B20B29", 390},
    {"B20B38", 391},
    {"B20B47", 392},
    {"B20B11", 393},
    {"B20B2", 394},
    {"B20B27", 395},
    {"B20B34", 396},
    {"B20B41", 397},
    {"B20B48", 398},
    {"B20B13", 399},
    {"B20B6", 400},
    {"B21B30", 401},
    {"B21B39", 402},
    {"B21B12", 403},
    {"B21B3", 404},
    {"B21B28", 405},
    {"B21B35", 406},
    {"B21B42", 407},
    {"B21B49", 408},
    {"B21B56", 409},
    {"B21B14", 410},
    {"B21B7", 411},
    {"B22B31", 412},
    {"B22B13", 413},
    {"B22B4", 414},
    {"B22B29", 415},
    {"B22B36", 416},
    {"B22B43", 417},
    {"B22B50", 418},
    {"B22B57", 419},
    {"B22B15", 420},
    {"B23B14", 421},
    {"B23B5", 422},
    {"B23B30", 423},
    {"B23B37", 424},
    {"B23B44", 425},
    {"B23B51", 426},
    {"B23B58", 427},
    {"B24B33", 428},
    {"B24B42", 429},
    {"B24B51", 430},
    {"B24B60", 431},
    {"B24B17", 432},
    {"B24B10", 433},
    {"B24B3", 434},
    {"B25B34", 435},
    {"B25B43", 436},
    {"B25B52", 437},
    {"B25B61", 438},
    {"B25B16", 439},
    {"B25B32", 440},
    {"B25B18", 441},
    {"B25B11", 442},
    {"B25B4", 443},
    {"B26B35", 444},
    {"B26B44", 445},
    {"B26B53", 446},
    {"B26B62", 447},
    {"B26B17", 448},
    {"B26B8", 449},
    {"B26B33", 450},
    {"B26B40", 451},
    {"B26B19", 452},
    {"B26B12", 453},
    {"B26B5", 454},
    {"B27B36", 455},
    {"B27B45", 456},
    {"B27B54", 457},
    {"B27B63", 458},
    {"B27B18", 459},
    {"B27B9", 460},
    {"B27B0", 461},
    {"B27B34", 462},
    {"B27B41", 463},
    {"B27B48", 464},
    {"B27B20", 465},
    {"B27B13", 466},
    {"B27B6", 467},
    {"B28B37", 468},
    {"B28B46", 469},
    {"B28B55", 470},
    {"B28B19", 471},
    {"B28B10", 472},
    {"B28B1", 473},
    {"B28B35", 474},
    {"B28B42", 475},
    {"B28B49", 476},
    {"B28B56", 477},
    {"B28B21", 478},
    {"B28B14", 479},
    {"B28B7", 480},
    {"B29B38", 481},
    {"B29B47", 482},
    {"B29B20", 483},
    {"B29B11", 484},
    {"B29B2", 485},
    {"B29B36", 486},
    {"B29B43", 487},
    {"B29B50", 488},
    {"B29B57", 489},
    {"B29B22", 490},
    {"B29B15", 491},
    {"B30B39", 492},
    {"B30B21", 493},
    {"B30B12", 494},
    {"B30B3", 495},
    {"B30B37", 496},
    {"B30B44", 497},
    {"B30B51", 498},
    {"B30B58", 499},
    {"B30B23", 500},
    {"B31B22", 501},
    {"B31B13", 502},
    {"B31B4", 503},
    {"B31B38", 504},
    {"B31B45", 505},
    {"B31B52", 506},
    {"B31B59", 507},
    {"B32B41", 508},
    {"B32B50", 509},
    {"B32B59", 510},
    {"B32B25", 511},
    {"B32B18", 512},
    {"B32B11", 513},
    {"B32B4", 514},
    {"B33B42", 515},
    {"B33B51", 516},
    {"B33B60", 517},
    {"B33B24", 518},
    {"B33B40", 519},
    {"B33B26", 520},
    {"B33B19", 521},
    {"B33B12", 522},
    {"B33B5", 523},
    {"B34B43", 524},
    {"B34B52", 525},
    {"B34B61", 526},
    {"B34B25", 527},
    {"B34B16", 528},
    {"B34B41", 529},
    {"B34B48", 530},
    {"B34B27", 531},
    {"B34B20", 532},
    {"B34B13", 533},
    {"B34B6", 534},
    {"B35B44", 535},
    {"B35B53", 536},
    {"B35B62", 537},
    {"B35B26", 538},
    {"B35B17", 539},
    {"B35B8", 540},
    {"B35B42", 541},
    {"B35B49", 542},
    {"B35B56", 543},
    {"B35B28", 544},
    {"B35B21", 545},
    {"B35B14", 546},
    {"B35B7", 547},
    {"B36B45", 548},
    {"B36B54", 549},
    {"B36B63", 550},
    {"B36B27", 551},
    {"B36B18", 552},
    {"B36B9", 553},
    {"B36B0", 554},
    {"B36B43", 555},
    {"B36B50", 556},
    {"B36B57", 557},
    {"B36B29", 558},
    {"B36B22", 559},
    {"B36B15", 560},
    {"B37B46", 561},
    {"B37B55", 562},
    {"B37B28", 563},
    {"B37B19", 564},
    {"B37B10", 565},
    {"B37B1", 566},
    {"B37B44", 567},
    {"B37B51", 568},
    {"B37B58", 569},
    {"B37B30", 570},
    {"B37B23", 571},
    {"B38B47", 572},
    {"B38B29", 573},
    {"B38B20", 574},
    {"B38B11", 575},
    {"B38B2", 576},
    {"B38B45", 577},
    {"B38B52", 578},
    {"B38B59", 579},
    {"B38B31", 580},
    {"B39B30", 581},
    {"B39B21", 582},
    {"B39B12", 583},
    {"B39B3", 584},
    {"B39B46", 585},
    {"B39B53", 586},
    {"B39B60", 587},
    {"B40B49", 588},
    {"B40B58", 589},
    {"B40B33", 590},
    {"B40B26", 591},
    {"B40B19", 592},
    {"B40B12", 593},
    {"B40B5", 594},
    {"B41B50", 595},
    {"B41B59", 596},
    {"B41B32", 597},
    {"B41B48", 598},
    {"B41B34", 599},
    {"B41B27", 600},
    {"B41B20", 601},
    {"B41B13", 602},
    {"B41B6", 603},
    {"B42B51", 604},
    {"B42B60", 605},
    {"B42B33", 606},
    {"B42B24", 607},
    {"B42B49", 608},
    {"B42B56", 609},
    {"B42B35", 610},
    {"B42B28", 611},
    {"B42B21", 612},
    {"B42B14", 613},
    {"B42B7", 614},
    {"B43B52", 615},
    {"B43B61", 616},
    {"B43B34", 617},
    {"B43B25", 618},
    {"B43B16", 619},
    {"B43B50", 620},
    {"B43B57", 621},
    {"B43B36", 622},
    {"B43B29", 623},
    {"B43B22", 624},
    {"B43B15", 625},
    {"B44B53", 626},
    {"B44B62", 627},
    {"B44B35", 628},
    {"B44B26", 629},
    {"B44B17", 630},
    {"B44B8", 631},
    {"B44B51", 632},
    {"B44B58", 633},
    {"B44B37", 634},
    {"B44B30", 635},
    {"B44B23", 636},
    {"B45B54", 637},
    {"B45B63", 638},
    {"B45B36", 639},
    {"B45B27", 640},
    {"B45B18", 641},
    {"B45B9", 642},
    {"B45B0", 643},
    {"B45B52", 644},
    {"B45B59", 645},
    {"B45B38", 646},
    {"B45B31", 647},
    {"B46B55", 648},
    {"B46B37", 649},
    {"B46B28", 650},
    {"B46B19", 651},
    {"B46B10", 652},
    {"B46B1", 653},
    {"B46B53", 654},
    {"B46B60", 655},
    {"B46B39", 656},
    {"B47B38", 657},
    {"B47B29", 658},
    {"B47B20", 659},
    {"B47B11", 660},
    {"B47B2", 661},
    {"B47B54", 662},
    {"B47B61", 663},
    {"B48B57", 664},
    {"B48B41", 665},
    {"B48B34", 666},
    {"B48B27", 667},
    {"B48B20", 668},
    {"B48B13", 669},
    {"B48B6", 670},
    {"B49B58", 671},
    {"B49B40", 672},
    {"B49B56", 673},
    {"B49B42", 674},
    {"B49B35", 675},
    {"B49B28", 676},
    {"B49B21", 677},
    {"B49B14", 678},
    {"B49B7", 679},
    {"B50B59", 680},
    {"B50B41", 681},
    {"B50B32", 682},
    {"B50B57", 683},
    {"B50B43", 684},
    {"B50B36", 685},
    {"B50B29", 686},
    {"B50B22", 687},
    {"B50B15", 688},
    {"B51B60", 689},
    {"B51B42", 690},
    {"B51B33", 691},
    {"B51B24", 692},
    {"B51B58", 693},
    {"B51B44", 694},
    {"B51B37", 695},
    {"B51B30", 696},
    {"B51B23", 697},
    {"B52B61", 698},
    {"B52B43", 699},
    {"B52B34", 700},
    {"B52B25", 701},
    {"B52B16", 702},
    {"B52B59", 703},
    {"B52B45", 704},
    {"B52B38", 705},
    {"B52B31", 706},
    {"B53B62", 707},
    {"B53B44", 708},
    {"B53B35", 709},
    {"B53B26", 710},
    {"B53B17", 711},
    {"B53B8", 712},
    {"B53B60", 713},
    {"B53B46", 714},
    {"B53B39", 715},
    {"B54B63", 716},
    {"B54B45", 717},
    {"B54B36", 718},
    {"B54B27", 719},
    {"B54B18", 720},
    {"B54B9", 721},
    {"B54B0", 722},
    {"B54B61", 723},
    {"B54B47", 724},
    {"B55B46", 725},
    {"B55B37", 726},
    {"B55B28", 727},
    {"B55B19", 728},
    {"B55B10", 729},
    {"B55B1", 730},
    {"B55B62", 731},
    {"B56B49", 732},
    {"B56B42", 733},
    {"B56B35", 734},
    {"B56B28", 735},
    {"B56B21", 736},
    {"B56B14", 737},
    {"B56B7", 738},
    {"B57B48", 739},
    {"B57B50", 740},
    {"B57B43", 741},
    {"B57B36", 742},
    {"B57B29", 743},
    {"B57B22", 744},
    {"B57B15", 745},
    {"B58B49", 746},
    {"B58B40", 747},
    {"B58B51", 748},
    {"B58B44", 749},
    {"B58B37", 750},
    {"B58B30", 751},
    {"B58B23", 752},
    {"B59B50", 753},
    {"B59B41", 754},
    {"B59B32", 755},
    {"B59B52", 756},
    {"B59B45", 757},
    {"B59B38", 758},
    {"B59B31", 759},
    {"B60B51", 760},
    {"B60B42", 761},
    {"B60B33", 762},
    {"B60B24", 763},
    {"B60B53", 764},
    {"B60B46", 765},
    {"B60B39", 766},
    {"B61B52", 767},
    {"B61B43", 768},
    {"B61B34", 769},
    {"B61B25", 770},
    {"B61B16", 771},
    {"B61B54", 772},
    {"B61B47", 773},
    {"B62B53", 774},
    {"B62B44", 775},
    {"B62B35", 776},
    {"B62B26", 777},
    {"B62B17", 778},
    {"B62B8", 779},
    {"B62B55", 780},
    {"B63B54", 781},
    {"B63B45", 782},
    {"B63B36", 783},
    {"B63B27", 784},
    {"B63B18", 785},
    {"B63B9", 786},
    {"B63B0", 787},
    {"R0R8", 788},
    {"R0R16", 789},
    {"R0R24", 790},
    {"R0R32", 791},
    {"R0R40", 792},
    {"R0R48", 793},
    {"R0R56", 794},
    {"R0R1", 795},
    {"R0R2", 796},
    {"R0R3", 797},
    {"R0R4", 798},
    {"R0R5", 799},
    {"R0R6", 800},
    {"R0R7", 801},
    {"R1R9", 802},
    {"R1R17", 803},
    {"R1R25", 804},
    {"R1R33", 805},
    {"R1R41", 806},
    {"R1R49", 807},
    {"R1R57", 808},
    {"R1R2", 809},
    {"R1R3", 810},
    {"R1R4", 811},
    {"R1R5", 812},
    {"R1R6", 813},
    {"R1R7", 814},
    {"R1R0", 815},
    {"R2R10", 816},
    {"R2R18", 817},
    {"R2R26", 818},
    {"R2R34", 819},
    {"R2R42", 820},
    {"R2R50", 821},
    {"R2R58", 822},
    {"R2R3", 823},
    {"R2R4", 824},
    {"R2R5", 825},
    {"R2R6", 826},
    {"R2R7", 827},
    {"R2R1", 828},
    {"R2R0", 829},
    {"R3R11", 830},
    {"R3R19", 831},
    {"R3R27", 832},
    {"R3R35", 833},
    {"R3R43", 834},
    {"R3R51", 835},
    {"R3R59", 836},
    {"R3R4", 837},
    {"R3R5", 838},
    {"R3R6", 839},
    {"R3R7", 840},
    {"R3R2", 841},
    {"R3R1", 842},
    {"R3R0", 843},
    {"R4R12", 844},
    {"R4R20", 845},
    {"R4R28", 846},
    {"R4R36", 847},
    {"R4R44", 848},
    {"R4R52", 849},
    {"R4R60", 850},
    {"R4R5", 851},
    {"R4R6", 852},
    {"R4R7", 853},
    {"R4R3", 854},
    {"R4R2", 855},
    {"R4R1", 856},
    {"R4R0", 857},
    {"R5R13", 858},
    {"R5R21", 859},
    {"R5R29", 860},
    {"R5R37", 861},
    {"R5R45", 862},
    {"R5R53", 863},
    {"R5R61", 864},
    {"R5R6", 865},
    {"R5R7", 866},
    {"R5R4", 867},
    {"R5R3", 868},
    {"R5R2", 869},
    {"R5R1", 870},
    {"R5R0", 871},
    {"R6R14", 872},
    {"R6R22", 873},
    {"R6R30", 874},
    {"R6R38", 875},
    {"R6R46", 876},
    {"R6R54", 877},
    {"R6R62", 878},
    {"R6R7", 879},
    {"R6R5", 880},
    {"R6R4", 881},
    {"R6R3", 882},
    {"R6R2", 883},
    {"R6R1", 884},
    {"R6R0", 885},
    {"R7R15", 886},
    {"R7R23", 887},
    {"R7R31", 888},
    {"R7R39", 889},
    {"R7R47", 890},
    {"R7R55", 891},
    {"R7R63", 892},
    {"R7R6", 893},
    {"R7R5", 894},
    {"R7R4", 895},
    {"R7R3", 896},
    {"R7R2", 897},
    {"R7R1", 898},
    {"R7R0", 899},
    {"R8R16", 900},
    {"R8R24", 901},
    {"R8R32", 902},
    {"R8R40", 903},
    {"R8R48", 904},
    {"R8R56", 905},
    {"R8R0", 906},
    {"R8R9", 907},
    {"R8R10", 908},
    {"R8R11", 909},
    {"R8R12", 910},
    {"R8R13", 911},
    {"R8R14", 912},
    {"R8R15", 913},
    {"R9R17", 914},
    {"R9R25", 915},
    {"R9R33", 916},
    {"R9R41", 917},
    {"R9R49", 918},
    {"R9R57", 919},
    {"R9R1", 920},
    {"R9R10", 921},
    {"R9R11", 922},
    {"R9R12", 923},
    {"R9R13", 924},
    {"R9R14", 925},
    {"R9R15", 926},
    {"R9R8", 927},
    {"R10R18", 928},
    {"R10R26", 929},
    {"R10R34", 930},
    {"R10R42", 931},
    {"R10R50", 932},
    {"R10R58", 933},
    {"R10R2", 934},
    {"R10R11", 935},
    {"R10R12", 936},
    {"R10R13", 937},
    {"R10R14", 938},
    {"R10R15", 939},
    {"R10R9", 940},
    {"R10R8", 941},
    {"R11R19", 942},
    {"R11R27", 943},
    {"R11R35", 944},
    {"R11R43", 945},
    {"R11R51", 946},
    {"R11R59", 947},
    {"R11R3", 948},
    {"R11R12", 949},
    {"R11R13", 950},
    {"R11R14", 951},
    {"R11R15", 952},
    {"R11R10", 953},
    {"R11R9", 954},
    {"R11R8", 955},
    {"R12R20", 956},
    {"R12R28", 957},
    {"R12R36", 958},
    {"R12R44", 959},
    {"R12R52", 960},
    {"R12R60", 961},
    {"R12R4", 962},
    {"R12R13", 963},
    {"R12R14", 964},
    {"R12R15", 965},
    {"R12R11", 966},
    {"R12R10", 967},
    {"R12R9", 968},
    {"R12R8", 969},
    {"R13R21", 970},
    {"R13R29", 971},
    {"R13R37", 972},
    {"R13R45", 973},
    {"R13R53", 974},
    {"R13R61", 975},
    {"R13R5", 976},
    {"R13R14", 977},
    {"R13R15", 978},
    {"R13R12", 979},
    {"R13R11", 980},
    {"R13R10", 981},
    {"R13R9", 982},
    {"R13R8", 983},
    {"R14R22", 984},
    {"R14R30", 985},
    {"R14R38", 986},
    {"R14R46", 987},
    {"R14R54", 988},
    {"R14R62", 989},
    {"R14R6", 990},
    {"R14R15", 991},
    {"R14R13", 992},
    {"R14R12", 993},
    {"R14R11", 994},
    {"R14R10", 995},
    {"R14R9", 996},
    {"R14R8", 997},
    {"R15R23", 998},
    {"R15R31", 999},
    {"R15R39", 1000},
    {"R15R47", 1001},
    {"R15R55", 1002},
    {"R15R63", 1003},
    {"R15R7", 1004},
    {"R15R14", 1005},
    {"R15R13", 1006},
    {"R15R12", 1007},
    {"R15R11", 1008},
    {"R15R10", 1009},
    {"R15R9", 1010},
    {"R15R8", 1011},
    {"R16R24", 1012},
    {"R16R32", 1013},
    {"R16R40", 1014},
    {"R16R48", 1015},
    {"R16R56", 1016},
    {"R16R8", 1017},
    {"R16R0", 1018},
    {"R16R17", 1019},
    {"R16R18", 1020},
    {"R16R19", 1021},
    {"R16R20", 1022},
    {"R16R21", 1023},
    {"R16R22", 1024},
    {"R16R23", 1025},
    {"R17R25", 1026},
    {"R17R33", 1027},
    {"R17R41", 1028},
    {"R17R49", 1029},
    {"R17R57", 1030},
    {"R17R9", 1031},
    {"R17R1", 1032},
    {"R17R18", 1033},
    {"R17R19", 1034},
    {"R17R20", 1035},
    {"R17R21", 1036},
    {"R17R22", 1037},
    {"R17R23", 1038},
    {"R17R16", 1039},
    {"R18R26", 1040},
    {"R18R34", 1041},
    {"R18R42", 1042},
    {"R18R50", 1043},
    {"R18R58", 1044},
    {"R18R10", 1045},
    {"R18R2", 1046},
    {"R18R19", 1047},
    {"R18R20", 1048},
    {"R18R21", 1049},
    {"R18R22", 1050},
    {"R18R23", 1051},
    {"R18R17", 1052},
    {"R18R16", 1053},
    {"R19R27", 1054},
    {"R19R35", 1055},
    {"R19R43", 1056},
    {"R19R51", 1057},
    {"R19R59", 1058},
    {"R19R11", 1059},
    {"R19R3", 1060},
    {"R19R20", 1061},
    {"R19R21", 1062},
    {"R19R22", 1063},
    {"R19R23", 1064},
    {"R19R18", 1065},
    {"R19R17", 1066},
    {"R19R16", 1067},
    {"R20R28", 1068},
    {"R20R36", 1069},
    {"R20R44", 1070},
    {"R20R52", 1071},
    {"R20R60", 1072},
    {"R20R12", 1073},
    {"R20R4", 1074},
    {"R20R21", 1075},
    {"R20R22", 1076},
    {"R20R23", 1077},
    {"R20R19", 1078},
    {"R20R18", 1079},
    {"R20R17", 1080},
    {"R20R16", 1081},
    {"R21R29", 1082},
    {"R21R37", 1083},
    {"R21R45", 1084},
    {"R21R53", 1085},
    {"R21R61", 1086},
    {"R21R13", 1087},
    {"R21R5", 1088},
    {"R21R22", 1089},
    {"R21R23", 1090},
    {"R21R20", 1091},
    {"R21R19", 1092},
    {"R21R18", 1093},
    {"R21R17", 1094},
    {"R21R16", 1095},
    {"R22R30", 1096},
    {"R22R38", 1097},
    {"R22R46", 1098},
    {"R22R54", 1099},
    {"R22R62", 1100},
    {"R22R14", 1101},
    {"R22R6", 1102},
    {"R22R23", 1103},
    {"R22R21", 1104},
    {"R22R20", 1105},
    {"R22R19", 1106},
    {"R22R18", 1107},
    {"R22R17", 1108},
    {"R22R16", 1109},
    {"R23R31", 1110},
    {"R23R39", 1111},
    {"R23R47", 1112},
    {"R23R55", 1113},
    {"R23R63", 1114},
    {"R23R15", 1115},
    {"R23R7", 1116},
    {"R23R22", 1117},
    {"R23R21", 1118},
    {"R23R20", 1119},
    {"R23R19", 1120},
    {"R23R18", 1121},
    {"R23R17", 1122},
    {"R23R16", 1123},
    {"R24R32", 1124},
    {"R24R40", 1125},
    {"R24R48", 1126},
    {"R24R56", 1127},
    {"R24R16", 1128},
    {"R24R8", 1129},
    {"R24R0", 1130},
    {"R24R25", 1131},
    {"R24R26", 1132},
    {"R24R27", 1133},
    {"R24R28", 1134},
    {"R24R29", 1135},
    {"R24R30", 1136},
    {"R24R31", 1137},
    {"R25R33", 1138},
    {"R25R41", 1139},
    {"R25R49", 1140},
    {"R25R57", 1141},
    {"R25R17", 1142},
    {"R25R9", 1143},
    {"R25R1", 1144},
    {"R25R26", 1145},
    {"R25R27", 1146},
    {"R25R28", 1147},
    {"R25R29", 1148},
    {"R25R30", 1149},
    {"R25R31", 1150},
    {"R25R24", 1151},
    {"R26R34", 1152},
    {"R26R42", 1153},
    {"R26R50", 1154},
    {"R26R58", 1155},
    {"R26R18", 1156},
    {"R26R10", 1157},
    {"R26R2", 1158},
    {"R26R27", 1159},
    {"R26R28", 1160},
    {"R26R29", 1161},
    {"R26R30", 1162},
    {"R26R31", 1163},
    {"R26R25", 1164},
    {"R26R24", 1165},
    {"R27R35", 1166},
    {"R27R43", 1167},
    {"R27R51", 1168},
    {"R27R59", 1169},
    {"R27R19", 1170},
    {"R27R11", 1171},
    {"R27R3", 1172},
    {"R27R28", 1173},
    {"R27R29", 1174},
    {"R27R30", 1175},
    {"R27R31", 1176},
    {"R27R26", 1177},
    {"R27R25", 1178},
    {"R27R24", 1179},
    {"R28R36", 1180},
    {"R28R44", 1181},
    {"R28R52", 1182},
    {"R28R60", 1183},
    {"R28R20", 1184},
    {"R28R12", 1185},
    {"R28R4", 1186},
    {"R28R29", 1187},
    {"R28R30", 1188},
    {"R28R31", 1189},
    {"R28R27", 1190},
    {"R28R26", 1191},
    {"R28R25", 1192},
    {"R28R24", 1193},
    {"R29R37", 1194},
    {"R29R45", 1195},
    {"R29R53", 1196},
    {"R29R61", 1197},
    {"R29R21", 1198},
    {"R29R13", 1199},
    {"R29R5", 1200},
    {"R29R30", 1201},
    {"R29R31", 1202},
    {"R29R28", 1203},
    {"R29R27", 1204},
    {"R29R26", 1205},
    {"R29R25", 1206},
    {"R29R24", 1207},
    {"R30R38", 1208},
    {"R30R46", 1209},
    {"R30R54", 1210},
    {"R30R62", 1211},
    {"R30R22", 1212},
    {"R30R14", 1213},
    {"R30R6", 1214},
    {"R30R31", 1215},
    {"R30R29", 1216},
    {"R30R28", 1217},
    {"R30R27", 1218},
    {"R30R26", 1219},
    {"R30R25", 1220},
    {"R30R24", 1221},
    {"R31R39", 1222},
    {"R31R47", 1223},
    {"R31R55", 1224},
    {"R31R63", 1225},
    {"R31R23", 1226},
    {"R31R15", 1227},
    {"R31R7", 1228},
    {"R31R30", 1229},
    {"R31R29", 1230},
    {"R31R28", 1231},
    {"R31R27", 1232},
    {"R31R26", 1233},
    {"R31R25", 1234},
    {"R31R24", 1235},
    {"R32R40", 1236},
    {"R32R48", 1237},
    {"R32R56", 1238},
    {"R32R24", 1239},
    {"R32R16", 1240},
    {"R32R8", 1241},
    {"R32R0", 1242},
    {"R32R33", 1243},
    {"R32R34", 1244},
    {"R32R35", 1245},
    {"R32R36", 1246},
    {"R32R37", 1247},
    {"R32R38", 1248},
    {"R32R39", 1249},
    {"R33R41", 1250},
    {"R33R49", 1251},
    {"R33R57", 1252},
    {"R33R25", 1253},
    {"R33R17", 1254},
    {"R33R9", 1255},
    {"R33R1", 1256},
    {"R33R34", 1257},
    {"R33R35", 1258},
    {"R33R36", 1259},
    {"R33R37", 1260},
    {"R33R38", 1261},
    {"R33R39", 1262},
    {"R33R32", 1263},
    {"R34R42", 1264},
    {"R34R50", 1265},
    {"R34R58", 1266},
    {"R34R26", 1267},
    {"R34R18", 1268},
    {"R34R10", 1269},
    {"R34R2", 1270},
    {"R34R35", 1271},
    {"R34R36", 1272},
    {"R34R37", 1273},
    {"R34R38", 1274},
    {"R34R39", 1275},
    {"R34R33", 1276},
    {"R34R32", 1277},
    {"R35R43", 1278},
    {"R35R51", 1279},
    {"R35R59", 1280},
    {"R35R27", 1281},
    {"R35R19", 1282},
    {"R35R11", 1283},
    {"R35R3", 1284},
    {"R35R36", 1285},
    {"R35R37", 1286},
    {"R35R38", 1287},
    {"R35R39", 1288},
    {"R35R34", 1289},
    {"R35R33", 1290},
    {"R35R32", 1291},
    {"R36R44", 1292},
    {"R36R52", 1293},
    {"R36R60", 1294},
    {"R36R28", 1295},
    {"R36R20", 1296},
    {"R36R12", 1297},
    {"R36R4", 1298},
    {"R36R37", 1299},
    {"R36R38", 1300},
    {"R36R39", 1301},
    {"R36R35", 1302},
    {"R36R34", 1303},
    {"R36R33", 1304},
    {"R36R32", 1305},
    {"R37R45", 1306},
    {"R37R53", 1307},
    {"R37R61", 1308},
    {"R37R29", 1309},
    {"R37R21", 1310},
    {"R37R13", 1311},
    {"R37R5", 1312},
    {"R37R38", 1313},
    {"R37R39", 1314},
    {"R37R36", 1315},
    {"R37R35", 1316},
    {"R37R34", 1317},
    {"R37R33", 1318},
    {"R37R32", 1319},
    {"R38R46", 1320},
    {"R38R54", 1321},
    {"R38R62", 1322},
    {"R38R30", 1323},
    {"R38R22", 1324},
    {"R38R14", 1325},
    {"R38R6", 1326},
    {"R38R39", 1327},
    {"R38R37", 1328},
    {"R38R36", 1329},
    {"R38R35", 1330},
    {"R38R34", 1331},
    {"R38R33", 1332},
    {"R38R32", 1333},
    {"R39R47", 1334},
    {"R39R55", 1335},
    {"R39R63", 1336},
    {"R39R31", 1337},
    {"R39R23", 1338},
    {"R39R15", 1339},
    {"R39R7", 1340},
    {"R39R38", 1341},
    {"R39R37", 1342},
    {"R39R36", 1343},
    {"R39R35", 1344},
    {"R39R34", 1345},
    {"R39R33", 1346},
    {"R39R32", 1347},
    {"R40R48", 1348},
    {"R40R56", 1349},
    {"R40R32", 1350},
    {"R40R24", 1351},
    {"R40R16", 1352},
    {"R40R8", 1353},
    {"R40R0", 1354},
    {"R40R41", 1355},
    {"R40R42", 1356},
    {"R40R43", 1357},
    {"R40R44", 1358},
    {"R40R45", 1359},
    {"R40R46", 1360},
    {"R40R47", 1361},
    {"R41R49", 1362},
    {"R41R57", 1363},
    {"R41R33", 1364},
    {"R41R25", 1365},
    {"R41R17", 1366},
    {"R41R9", 1367},
    {"R41R1", 1368},
    {"R41R42", 1369},
    {"R41R43", 1370},
    {"R41R44", 1371},
    {"R41R45", 1372},
    {"R41R46", 1373},
    {"R41R47", 1374},
    {"R41R40", 1375},
    {"R42R50", 1376},
    {"R42R58", 1377},
    {"R42R34", 1378},
    {"R42R26", 1379},
    {"R42R18", 1380},
    {"R42R10", 1381},
    {"R42R2", 1382},
    {"R42R43", 1383},
    {"R42R44", 1384},
    {"R42R45", 1385},
    {"R42R46", 1386},
    {"R42R47", 1387},
    {"R42R41", 1388},
    {"R42R40", 1389},
    {"R43R51", 1390},
    {"R43R59", 1391},
    {"R43R35", 1392},
    {"R43R27", 1393},
    {"R43R19", 1394},
    {"R43R11", 1395},
    {"R43R3", 1396},
    {"R43R44", 1397},
    {"R43R45", 1398},
    {"R43R46", 1399},
    {"R43R47", 1400},
    {"R43R42", 1401},
    {"R43R41", 1402},
    {"R43R40", 1403},
    {"R44R52", 1404},
    {"R44R60", 1405},
    {"R44R36", 1406},
    {"R44R28", 1407},
    {"R44R20", 1408},
    {"R44R12", 1409},
    {"R44R4", 1410},
    {"R44R45", 1411},
    {"R44R46", 1412},
    {"R44R47", 1413},
    {"R44R43", 1414},
    {"R44R42", 1415},
    {"R44R41", 1416},
    {"R44R40", 1417},
    {"R45R53", 1418},
    {"R45R61", 1419},
    {"R45R37", 1420},
    {"R45R29", 1421},
    {"R45R21", 1422},
    {"R45R13", 1423},
    {"R45R5", 1424},
    {"R45R46", 1425},
    {"R45R47", 1426},
    {"R45R44", 1427},
    {"R45R43", 1428},
    {"R45R42", 1429},
    {"R45R41", 1430},
    {"R45R40", 1431},
    {"R46R54", 1432},
    {"R46R62", 1433},
    {"R46R38", 1434},
    {"R46R30", 1435},
    {"R46R22", 1436},
    {"R46R14", 1437},
    {"R46R6", 1438},
    {"R46R47", 1439},
    {"R46R45", 1440},
    {"R46R44", 1441},
    {"R46R43", 1442},
    {"R46R42", 1443},
    {"R46R41", 1444},
    {"R46R40", 1445},
    {"R47R55", 1446},
    {"R47R63", 1447},
    {"R47R39", 1448},
    {"R47R31", 1449},
    {"R47R23", 1450},
    {"R47R15", 1451},
    {"R47R7", 1452},
    {"R47R46", 1453},
    {"R47R45", 1454},
    {"R47R44", 1455},
    {"R47R43", 1456},
    {"R47R42", 1457},
    {"R47R41", 1458},
    {"R47R40", 1459},
    {"R48R56", 1460},
    {"R48R40", 1461},
    {"R48R32", 1462},
    {"R48R24", 1463},
    {"R48R16", 1464},
    {"R48R8", 1465},
    {"R48R0", 1466},
    {"R48R49", 1467},
    {"R48R50", 1468},
    {"R48R51", 1469},
    {"R48R52", 1470},
    {"R48R53", 1471},
    {"R48R54", 1472},
    {"R48R55", 1473},
    {"R49R57", 1474},
    {"R49R41", 1475},
    {"R49R33", 1476},
    {"R49R25", 1477},
    {"R49R17", 1478},
    {"R49R9", 1479},
    {"R49R1", 1480},
    {"R49R50", 1481},
    {"R49R51", 1482},
    {"R49R52", 1483},
    {"R49R53", 1484},
    {"R49R54", 1485},
    {"R49R55", 1486},
    {"R49R48", 1487},
    {"R50R58", 1488},
    {"R50R42", 1489},
    {"R50R34", 1490},
    {"R50R26", 1491},
    {"R50R18", 1492},
    {"R50R10", 1493},
    {"R50R2", 1494},
    {"R50R51", 1495},
    {"R50R52", 1496},
    {"R50R53", 1497},
    {"R50R54", 1498},
    {"R50R55", 1499},
    {"R50R49", 1500},
    {"R50R48", 1501},
    {"R51R59", 1502},
    {"R51R43", 1503},
    {"R51R35", 1504},
    {"R51R27", 1505},
    {"R51R19", 1506},
    {"R51R11", 1507},
    {"R51R3", 1508},
    {"R51R52", 1509},
    {"R51R53", 1510},
    {"R51R54", 1511},
    {"R51R55", 1512},
    {"R51R50", 1513},
    {"R51R49", 1514},
    {"R51R48", 1515},
    {"R52R60", 1516},
    {"R52R44", 1517},
    {"R52R36", 1518},
    {"R52R28", 1519},
    {"R52R20", 1520},
    {"R52R12", 1521},
    {"R52R4", 1522},
    {"R52R53", 1523},
    {"R52R54", 1524},
    {"R52R55", 1525},
    {"R52R51", 1526},
    {"R52R50", 1527},
    {"R52R49", 1528},
    {"R52R48", 1529},
    {"R53R61", 1530},
    {"R53R45", 1531},
    {"R53R37", 1532},
    {"R53R29", 1533},
    {"R53R21", 1534},
    {"R53R13", 1535},
    {"R53R5", 1536},
    {"R53R54", 1537},
    {"R53R55", 1538},
    {"R53R52", 1539},
    {"R53R51", 1540},
    {"R53R50", 1541},
    {"R53R49", 1542},
    {"R53R48", 1543},
    {"R54R62", 1544},
    {"R54R46", 1545},
    {"R54R38", 1546},
    {"R54R30", 1547},
    {"R54R22", 1548},
    {"R54R14", 1549},
    {"R54R6", 1550},
    {"R54R55", 1551},
    {"R54R53", 1552},
    {"R54R52", 1553},
    {"R54R51", 1554},
    {"R54R50", 1555},
    {"R54R49", 1556},
    {"R54R48", 1557},
    {"R55R63", 1558},
    {"R55R47", 1559},
    {"R55R39", 1560},
    {"R55R31", 1561},
    {"R55R23", 1562},
    {"R55R15", 1563},
    {"R55R7", 1564},
    {"R55R54", 1565},
    {"R55R53", 1566},
    {"R55R52", 1567},
    {"R55R51", 1568},
    {"R55R50", 1569},
    {"R55R49", 1570},
    {"R55R48", 1571},
    {"R56R48", 1572},
    {"R56R40", 1573},
    {"R56R32", 1574},
    {"R56R24", 1575},
    {"R56R16", 1576},
    {"R56R8", 1577},
    {"R56R0", 1578},
    {"R56R57", 1579},
    {"R56R58", 1580},
    {"R56R59", 1581},
    {"R56R60", 1582},
    {"R56R61", 1583},
    {"R56R62", 1584},
    {"R56R63", 1585},
    {"R57R49", 1586},
    {"R57R41", 1587},
    {"R57R33", 1588},
    {"R57R25", 1589},
    {"R57R17", 1590},
    {"R57R9", 1591},
    {"R57R1", 1592},
    {"R57R58", 1593},
    {"R57R59", 1594},
    {"R57R60", 1595},
    {"R57R61", 1596},
    {"R57R62", 1597},
    {"R57R63", 1598},
    {"R57R56", 1599},
    {"R58R50", 1600},
    {"R58R42", 1601},
    {"R58R34", 1602},
    {"R58R26", 1603},
    {"R58R18", 1604},
    {"R58R10", 1605},
    {"R58R2", 1606},
    {"R58R59", 1607},
    {"R58R60", 1608},
    {"R58R61", 1609},
    {"R58R62", 1610},
    {"R58R63", 1611},
    {"R58R57", 1612},
    {"R58R56", 1613},
    {"R59R51", 1614},
    {"R59R43", 1615},
    {"R59R35", 1616},
    {"R59R27", 1617},
    {"R59R19", 1618},
    {"R59R11", 1619},
    {"R59R3", 1620},
    {"R59R60", 1621},
    {"R59R61", 1622},
    {"R59R62", 1623},
    {"R59R63", 1624},
    {"R59R58", 1625},
    {"R59R57", 1626},
    {"R59R56", 1627},
    {"R60R52", 1628},
    {"R60R44", 1629},
    {"R60R36", 1630},
    {"R60R28", 1631},
    {"R60R20", 1632},
    {"R60R12", 1633},
    {"R60R4", 1634},
    {"R60R61", 1635},
    {"R60R62", 1636},
    {"R60R63", 1637},
    {"R60R59", 1638},
    {"R60R58", 1639},
    {"R60R57", 1640},
    {"R60R56", 1641},
    {"R61R53", 1642},
    {"R61R45", 1643},
    {"R61R37", 1644},
    {"R61R29", 1645},
    {"R61R21", 1646},
    {"R61R13", 1647},
    {"R61R5", 1648},
    {"R61R62", 1649},
    {"R61R63", 1650},
    {"R61R60", 1651},
    {"R61R59", 1652},
    {"R61R58", 1653},
    {"R61R57", 1654},
    {"R61R56", 1655},
    {"R62R54", 1656},
    {"R62R46", 1657},
    {"R62R38", 1658},
    {"R62R30", 1659},
    {"R62R22", 1660},
    {"R62R14", 1661},
    {"R62R6", 1662},
    {"R62R63", 1663},
    {"R62R61", 1664},
    {"R62R60", 1665},
    {"R62R59", 1666},
    {"R62R58", 1667},
    {"R62R57", 1668},
    {"R62R56", 1669},
    {"R63R55", 1670},
    {"R63R47", 1671},
    {"R63R39", 1672},
    {"R63R31", 1673},
    {"R63R23", 1674},
    {"R63R15", 1675},
    {"R63R7", 1676},
    {"R63R62", 1677},
    {"R63R61", 1678},
    {"R63R60", 1679},
    {"R63R59", 1680},
    {"R63R58", 1681},
    {"R63R57", 1682},
    {"R63R56", 1683},
    {"Q0Q9", 1684},
    {"Q0Q18", 1685},
    {"Q0Q27", 1686},
    {"Q0Q36", 1687},
    {"Q0Q45", 1688},
    {"Q0Q54", 1689},
    {"Q0Q63", 1690},
    {"Q0Q8", 1691},
    {"Q0Q16", 1692},
    {"Q0Q24", 1693},
    {"Q0Q32", 1694},
    {"Q0Q40", 1695},
    {"Q0Q48", 1696},
    {"Q0Q56", 1697},
    {"Q0Q1", 1698},
    {"Q0Q2", 1699},
    {"Q0Q3", 1700},
    {"Q0Q4", 1701},
    {"Q0Q5", 1702},
    {"Q0Q6", 1703},
    {"Q0Q7", 1704},
    {"Q1Q10", 1705},
    {"Q1Q19", 1706},
    {"Q1Q28", 1707},
    {"Q1Q37", 1708},
    {"Q1Q46", 1709},
    {"Q1Q55", 1710},
    {"Q1Q8", 1711},
    {"Q1Q9", 1712},
    {"Q1Q17", 1713},
    {"Q1Q25", 1714},
    {"Q1Q33", 1715},
    {"Q1Q41", 1716},
    {"Q1Q49", 1717},
    {"Q1Q57", 1718},
    {"Q1Q2", 1719},
    {"Q1Q3", 1720},
    {"Q1Q4", 1721},
    {"Q1Q5", 1722},
    {"Q1Q6", 1723},
    {"Q1Q7", 1724},
    {"Q1Q0", 1725},
    {"Q2Q11", 1726},
    {"Q2Q20", 1727},
    {"Q2Q29", 1728},
    {"Q2Q38", 1729},
    {"Q2Q47", 1730},
    {"Q2Q9", 1731},
    {"Q2Q16", 1732},
    {"Q2Q10", 1733},
    {"Q2Q18", 1734},
    {"Q2Q26", 1735},
    {"Q2Q34", 1736},
    {"Q2Q42", 1737},
    {"Q2Q50", 1738},
    {"Q2Q58", 1739},
    {"Q2Q3", 1740},
    {"Q2Q4", 1741},
    {"Q2Q5", 1742},
    {"Q2Q6", 1743},
    {"Q2Q7", 1744},
    {"Q2Q1", 1745},
    {"Q2Q0", 1746},
    {"Q3Q12", 1747},
    {"Q3Q21", 1748},
    {"Q3Q30", 1749},
    {"Q3Q39", 1750},
    {"Q3Q10", 1751},
    {"Q3Q17", 1752},
    {"Q3Q24", 1753},
    {"Q3Q11", 1754},
    {"Q3Q19", 1755},
    {"Q3Q27", 1756},
    {"Q3Q35", 1757},
    {"Q3Q43", 1758},
    {"Q3Q51", 1759},
    {"Q3Q59", 1760},
    {"Q3Q4", 1761},
    {"Q3Q5", 1762},
    {"Q3Q6", 1763},
    {"Q3Q7", 1764},
    {"Q3Q2", 1765},
    {"Q3Q1", 1766},
    {"Q3Q0", 1767},
    {"Q4Q13", 1768},
    {"Q4Q22", 1769},
    {"Q4Q31", 1770},
    {"Q4Q11", 1771},
    {"Q4Q18", 1772},
    {"Q4Q25", 1773},
    {"Q4Q32", 1774},
    {"Q4Q12", 1775},
    {"Q4Q20", 1776},
    {"Q4Q28", 1777},
    {"Q4Q36", 1778},
    {"Q4Q44", 1779},
    {"Q4Q52", 1780},
    {"Q4Q60", 1781},
    {"Q4Q5", 1782},
    {"Q4Q6", 1783},
    {"Q4Q7", 1784},
    {"Q4Q3", 1785},
    {"Q4Q2", 1786},
    {"Q4Q1", 1787},
    {"Q4Q0", 1788},
    {"Q5Q14", 1789},
    {"Q5Q23", 1790},
    {"Q5Q12", 1791},
    {"Q5Q19", 1792},
    {"Q5Q26", 1793},
    {"Q5Q33", 1794},
    {"Q5Q40", 1795},
    {"Q5Q13", 1796},
    {"Q5Q21", 1797},
    {"Q5Q29", 1798},
    {"Q5Q37", 1799},
    {"Q5Q45", 1800},
    {"Q5Q53", 1801},
    {"Q5Q61", 1802},
    {"Q5Q6", 1803},
    {"Q5Q7", 1804},
    {"Q5Q4", 1805},
    {"Q5Q3", 1806},
    {"Q5Q2", 1807},
    {"Q5Q1", 1808},
    {"Q5Q0", 1809},
    {"Q6Q15", 1810},
    {"Q6Q13", 1811},
    {"Q6Q20", 1812},
    {"Q6Q27", 1813},
    {"Q6Q34", 1814},
    {"Q6Q41", 1815},
    {"Q6Q48", 1816},
    {"Q6Q14", 1817},
    {"Q6Q22", 1818},
    {"Q6Q30", 1819},
    {"Q6Q38", 1820},
    {"Q6Q46", 1821},
    {"Q6Q54", 1822},
    {"Q6Q62", 1823},
    {"Q6Q7", 1824},
    {"Q6Q5", 1825},
    {"Q6Q4", 1826},
    {"Q6Q3", 1827},
    {"Q6Q2", 1828},
    {"Q6Q1", 1829},
    {"Q6Q0", 1830},
    {"Q7Q14", 1831},
    {"Q7Q21", 1832},
    {"Q7Q28", 1833},
    {"Q7Q35", 1834},
    {"Q7Q42", 1835},
    {"Q7Q49", 1836},
    {"Q7Q56", 1837},
    {"Q7Q15", 1838},
    {"Q7Q23", 1839},
    {"Q7Q31", 1840},
    {"Q7Q39", 1841},
    {"Q7Q47", 1842},
    {"Q7Q55", 1843},
    {"Q7Q63", 1844},
    {"Q7Q6", 1845},
    {"Q7Q5", 1846},
    {"Q7Q4", 1847},
    {"Q7Q3", 1848},
    {"Q7Q2", 1849},
    {"Q7Q1", 1850},
    {"Q7Q0", 1851},
    {"Q8Q17", 1852},
    {"Q8Q26", 1853},
    {"Q8Q35", 1854},
    {"Q8Q44", 1855},
    {"Q8Q53", 1856},
    {"Q8Q62", 1857},
    {"Q8Q1", 1858},
    {"Q8Q16", 1859},
    {"Q8Q24", 1860},
    {"Q8Q32", 1861},
    {"Q8Q40", 1862},
    {"Q8Q48", 1863},
    {"Q8Q56", 1864},
    {"Q8Q0", 1865},
    {"Q8Q9", 1866},
    {"Q8Q10", 1867},
    {"Q8Q11", 1868},
    {"Q8Q12", 1869},
    {"Q8Q13", 1870},
    {"Q8Q14", 1871},
    {"Q8Q15", 1872},
    {"Q9Q18", 1873},
    {"Q9Q27", 1874},
    {"Q9Q36", 1875},
    {"Q9Q45", 1876},
    {"Q9Q54", 1877},
    {"Q9Q63", 1878},
    {"Q9Q0", 1879},
    {"Q9Q16", 1880},
    {"Q9Q2", 1881},
    {"Q9Q17", 1882},
    {"Q9Q25", 1883},
    {"Q9Q33", 1884},
    {"Q9Q41", 1885},
    {"Q9Q49", 1886},
    {"Q9Q57", 1887},
    {"Q9Q1", 1888},
    {"Q9Q10", 1889},
    {"Q9Q11", 1890},
    {"Q9Q12", 1891},
    {"Q9Q13", 1892},
    {"Q9Q14", 1893},
    {"Q9Q15", 1894},
    {"Q9Q8", 1895},
    {"Q10Q19", 1896},
    {"Q10Q28", 1897},
    {"Q10Q37", 1898},
    {"Q10Q46", 1899},
    {"Q10Q55", 1900},
    {"Q10Q1", 1901},
    {"Q10Q17", 1902},
    {"Q10Q24", 1903},
    {"Q10Q3", 1904},
    {"Q10Q18", 1905},
    {"Q10Q26", 1906},
    {"Q10Q34", 1907},
    {"Q10Q42", 1908},
    {"Q10Q50", 1909},
    {"Q10Q58", 1910},
    {"Q10Q2", 1911},
    {"Q10Q11", 1912},
    {"Q10Q12", 1913},
    {"Q10Q13", 1914},
    {"Q10Q14", 1915},
    {"Q10Q15", 1916},
    {"Q10Q9", 1917},
    {"Q10Q8", 1918},
    {"Q11Q20", 1919},
    {"Q11Q29", 1920},
    {"Q11Q38", 1921},
    {"Q11Q47", 1922},
    {"Q11Q2", 1923},
    {"Q11Q18", 1924},
    {"Q11Q25", 1925},
    {"Q11Q32", 1926},
    {"Q11Q4", 1927},
    {"Q11Q19", 1928},
    {"Q11Q27", 1929},
    {"Q11Q35", 1930},
    {"Q11Q43", 1931},
    {"Q11Q51", 1932},
    {"Q11Q59", 1933},
    {"Q11Q3", 1934},
    {"Q11Q12", 1935},
    {"Q11Q13", 1936},
    {"Q11Q14", 1937},
    {"Q11Q15", 1938},
    {"Q11Q10", 1939},
    {"Q11Q9", 1940},
    {"Q11Q8", 1941},
    {"Q12Q21", 1942},
    {"Q12Q30", 1943},
    {"Q12Q39", 1944},
    {"Q12Q3", 1945},
    {"Q12Q19", 1946},
    {"Q12Q26", 1947},
    {"Q12Q33", 1948},
    {"Q12Q40", 1949},
    {"Q12Q5", 1950},
    {"Q12Q20", 1951},
    {"Q12Q28", 1952},
    {"Q12Q36", 1953},
    {"Q12Q44", 1954},
    {"Q12Q52", 1955},
    {"Q12Q60", 1956},
    {"Q12Q4", 1957},
    {"Q12Q13", 1958},
    {"Q12Q14", 1959},
    {"Q12Q15", 1960},
    {"Q12Q11", 1961},
    {"Q12Q10", 1962},
    {"Q12Q9", 1963},
    {"Q12Q8", 1964},
    {"Q13Q22", 1965},
    {"Q13Q31", 1966},
    {"Q13Q4", 1967},
    {"Q13Q20", 1968},
    {"Q13Q27", 1969},
    {"Q13Q34", 1970},
    {"Q13Q41", 1971},
    {"Q13Q48", 1972},
    {"Q13Q6", 1973},
    {"Q13Q21", 1974},
    {"Q13Q29", 1975},
    {"Q13Q37", 1976},
    {"Q13Q45", 1977},
    {"Q13Q53", 1978},
    {"Q13Q61", 1979},
    {"Q13Q5", 1980},
    {"Q13Q14", 1981},
    {"Q13Q15", 1982},
    {"Q13Q12", 1983},
    {"Q13Q11", 1984},
    {"Q13Q10", 1985},
    {"Q13Q9", 1986},
    {"Q13Q8", 1987},
    {"Q14Q23", 1988},
    {"Q14Q5", 1989},
    {"Q14Q21", 1990},
    {"Q14Q28", 1991},
    {"Q14Q35", 1992},
    {"Q14Q42", 1993},
    {"Q14Q49", 1994},
    {"Q14Q56", 1995},
    {"Q14Q7", 1996},
    {"Q14Q22", 1997},
    {"Q14Q30", 1998},
    {"Q14Q38", 1999},
    {"Q14Q46", 2000},
    {"Q14Q54", 2001},
    {"Q14Q62", 2002},
    {"Q14Q6", 2003},
    {"Q14Q15", 2004},
    {"Q14Q13", 2005},
    {"Q14Q12", 2006},
    {"Q14Q11", 2007},
    {"Q14Q10", 2008},
    {"Q14Q9", 2009},
    {"Q14Q8", 2010},
    {"Q15Q6", 2011},
    {"Q15Q22", 2012},
    {"Q15Q29", 2013},
    {"Q15Q36", 2014},
    {"Q15Q43", 2015},
    {"Q15Q50", 2016},
    {"Q15Q57", 2017},
    {"Q15Q23", 2018},
    {"Q15Q31", 2019},
    {"Q15Q39", 2020},
    {"Q15Q47", 2021},
    {"Q15Q55", 2022},
    {"Q15Q63", 2023},
    {"Q15Q7", 2024},
    {"Q15Q14", 2025},
    {"Q15Q13", 2026},
    {"Q15Q12", 2027},
    {"Q15Q11", 2028},
    {"Q15Q10", 2029},
    {"Q15Q9", 2030},
    {"Q15Q8", 2031},
    {"Q16Q25", 2032},
    {"Q16Q34", 2033},
    {"Q16Q43", 2034},
    {"Q16Q52", 2035},
    {"Q16Q61", 2036},
    {"Q16Q9", 2037},
    {"Q16Q2", 2038},
    {"Q16Q24", 2039},
    {"Q16Q32", 2040},
    {"Q16Q40", 2041},
    {"Q16Q48", 2042},
    {"Q16Q56", 2043},
    {"Q16Q8", 2044},
    {"Q16Q0", 2045},
    {"Q16Q17", 2046},
    {"Q16Q18", 2047},
    {"Q16Q19", 2048},
    {"Q16Q20", 2049},
    {"Q16Q21", 2050},
    {"Q16Q22", 2051},
    {"Q16Q23", 2052},
    {"Q17Q26", 2053},
    {"Q17Q35", 2054},
    {"Q17Q44", 2055},
    {"Q17Q53", 2056},
    {"Q17Q62", 2057},
    {"Q17Q8", 2058},
    {"Q17Q24", 2059},
    {"Q17Q10", 2060},
    {"Q17Q3", 2061},
    {"Q17Q25", 2062},
    {"Q17Q33", 2063},
    {"Q17Q41", 2064},
    {"Q17Q49", 2065},
    {"Q17Q57", 2066},
    {"Q17Q9", 2067},
    {"Q17Q1", 2068},
    {"Q17Q18", 2069},
    {"Q17Q19", 2070},
    {"Q17Q20", 2071},
    {"Q17Q21", 2072},
    {"Q17Q22", 2073},
    {"Q17Q23", 2074},
    {"Q17Q16", 2075},
    {"Q18Q27", 2076},
    {"Q18Q36", 2077},
    {"Q18Q45", 2078},
    {"Q18Q54", 2079},
    {"Q18Q63", 2080},
    {"Q18Q9", 2081},
    {"Q18Q0", 2082},
    {"Q18Q25", 2083},
    {"Q18Q32", 2084},
    {"Q18Q11", 2085},
    {"Q18Q4", 2086},
    {"Q18Q26", 2087},
    {"Q18Q34", 2088},
    {"Q18Q42", 2089},
    {"Q18Q50", 2090},
    {"Q18Q58", 2091},
    {"Q18Q10", 2092},
    {"Q18Q2", 2093},
    {"Q18Q19", 2094},
    {"Q18Q20", 2095},
    {"Q18Q21", 2096},
    {"Q18Q22", 2097},
    {"Q18Q23", 2098},
    {"Q18Q17", 2099},
    {"Q18Q16", 2100},
    {"Q19Q28", 2101},
    {"Q19Q37", 2102},
    {"Q19Q46", 2103},
    {"Q19Q55", 2104},
    {"Q19Q10", 2105},
    {"Q19Q1", 2106},
    {"Q19Q26", 2107},
    {"Q19Q33", 2108},
    {"Q19Q40", 2109},
    {"Q19Q12", 2110},
    {"Q19Q5", 2111},
    {"Q19Q27", 2112},
    {"Q19Q35", 2113},
    {"Q19Q43", 2114},
    {"Q19Q51", 2115},
    {"Q19Q59", 2116},
    {"Q19Q11", 2117},
    {"Q19Q3", 2118},
    {"Q19Q20", 2119},
    {"Q19Q21", 2120},
    {"Q19Q22", 2121},
    {"Q19Q23", 2122},
    {"Q19Q18", 2123},
    {"Q19Q17", 2124},
    {"Q19Q16", 2125},
    {"Q20Q29", 2126},
    {"Q20Q38", 2127},
    {"Q20Q47", 2128},
    {"Q20Q11", 2129},
    {"Q20Q2", 2130},
    {"Q20Q27", 2131},
    {"Q20Q34", 2132},
    {"Q20Q41", 2133},
    {"Q20Q48", 2134},
    {"Q20Q13", 2135},
    {"Q20Q6", 2136},
    {"Q20Q28", 2137},
    {"Q20Q36", 2138},
    {"Q20Q44", 2139},
    {"Q20Q52", 2140},
    {"Q20Q60", 2141},
    {"Q20Q12", 2142},
    {"Q20Q4", 2143},
    {"Q20Q21", 2144},
    {"Q20Q22", 2145},
    {"Q20Q23", 2146},
    {"Q20Q19", 2147},
    {"Q20Q18", 2148},
    {"Q20Q17", 2149},
    {"Q20Q16", 2150},
    {"Q21Q30", 2151},
    {"Q21Q39", 2152},
    {"Q21Q12", 2153},
    {"Q21Q3", 2154},
    {"Q21Q28", 2155},
    {"Q21Q35", 2156},
    {"Q21Q42", 2157},
    {"Q21Q49", 2158},
    {"Q21Q56", 2159},
    {"Q21Q14", 2160},
    {"Q21Q7", 2161},
    {"Q21Q29", 2162},
    {"Q21Q37", 2163},
    {"Q21Q45", 2164},
    {"Q21Q53", 2165},
    {"Q21Q61", 2166},
    {"Q21Q13", 2167},
    {"Q21Q5", 2168},
    {"Q21Q22", 2169},
    {"Q21Q23", 2170},
    {"Q21Q20", 2171},
    {"Q21Q19", 2172},
    {"Q21Q18", 2173},
    {"Q21Q17", 2174},
    {"Q21Q16", 2175},
    {"Q22Q31", 2176},
    {"Q22Q13", 2177},
    {"Q22Q4", 2178},
    {"Q22Q29", 2179},
    {"Q22Q36", 2180},
    {"Q22Q43", 2181},
    {"Q22Q50", 2182},
    {"Q22Q57", 2183},
    {"Q22Q15", 2184},
    {"Q22Q30", 2185},
    {"Q22Q38", 2186},
    {"Q22Q46", 2187},
    {"Q22Q54", 2188},
    {"Q22Q62", 2189},
    {"Q22Q14", 2190},
    {"Q22Q6", 2191},
    {"Q22Q23", 2192},
    {"Q22Q21", 2193},
    {"Q22Q20", 2194},
    {"Q22Q19", 2195},
    {"Q22Q18", 2196},
    {"Q22Q17", 2197},
    {"Q22Q16", 2198},
    {"Q23Q14", 2199},
    {"Q23Q5", 2200},
    {"Q23Q30", 2201},
    {"Q23Q37", 2202},
    {"Q23Q44", 2203},
    {"Q23Q51", 2204},
    {"Q23Q58", 2205},
    {"Q23Q31", 2206},
    {"Q23Q39", 2207},
    {"Q23Q47", 2208},
    {"Q23Q55", 2209},
    {"Q23Q63", 2210},
    {"Q23Q15", 2211},
    {"Q23Q7", 2212},
    {"Q23Q22", 2213},
    {"Q23Q21", 2214},
    {"Q23Q20", 2215},
    {"Q23Q19", 2216},
    {"Q23Q18", 2217},
    {"Q23Q17", 2218},
    {"Q23Q16", 2219},
    {"Q24Q33", 2220},
    {"Q24Q42", 2221},
    {"Q24Q51", 2222},
    {"Q24Q60", 2223},
    {"Q24Q17", 2224},
    {"Q24Q10", 2225},
    {"Q24Q3", 2226},
    {"Q24Q32", 2227},
    {"Q24Q40", 2228},
    {"Q24Q48", 2229},
    {"Q24Q56", 2230},
    {"Q24Q16", 2231},
    {"Q24Q8", 2232},
    {"Q24Q0", 2233},
    {"Q24Q25", 2234},
    {"Q24Q26", 2235},
    {"Q24Q27", 2236},
    {"Q24Q28", 2237},
    {"Q24Q29", 2238},
    {"Q24Q30", 2239},
    {"Q24Q31", 2240},
    {"Q25Q34", 2241},
    {"Q25Q43", 2242},
    {"Q25Q52", 2243},
    {"Q25Q61", 2244},
    {"Q25Q16", 2245},
    {"Q25Q32", 2246},
    {"Q25Q18", 2247},
    {"Q25Q11", 2248},
    {"Q25Q4", 2249},
    {"Q25Q33", 2250},
    {"Q25Q41", 2251},
    {"Q25Q49", 2252},
    {"Q25Q57", 2253},
    {"Q25Q17", 2254},
    {"Q25Q9", 2255},
    {"Q25Q1", 2256},
    {"Q25Q26", 2257},
    {"Q25Q27", 2258},
    {"Q25Q28", 2259},
    {"Q25Q29", 2260},
    {"Q25Q30", 2261},
    {"Q25Q31", 2262},
    {"Q25Q24", 2263},
    {"Q26Q35", 2264},
    {"Q26Q44", 2265},
    {"Q26Q53", 2266},
    {"Q26Q62", 2267},
    {"Q26Q17", 2268},
    {"Q26Q8", 2269},
    {"Q26Q33", 2270},
    {"Q26Q40", 2271},
    {"Q26Q19", 2272},
    {"Q26Q12", 2273},
    {"Q26Q5", 2274},
    {"Q26Q34", 2275},
    {"Q26Q42", 2276},
    {"Q26Q50", 2277},
    {"Q26Q58", 2278},
    {"Q26Q18", 2279},
    {"Q26Q10", 2280},
    {"Q26Q2", 2281},
    {"Q26Q27", 2282},
    {"Q26Q28", 2283},
    {"Q26Q29", 2284},
    {"Q26Q30", 2285},
    {"Q26Q31", 2286},
    {"Q26Q25", 2287},
    {"Q26Q24", 2288},
    {"Q27Q36", 2289},
    {"Q27Q45", 2290},
    {"Q27Q54", 2291},
    {"Q27Q63", 2292},
    {"Q27Q18", 2293},
    {"Q27Q9", 2294},
    {"Q27Q0", 2295},
    {"Q27Q34", 2296},
    {"Q27Q41", 2297},
    {"Q27Q48", 2298},
    {"Q27Q20", 2299},
    {"Q27Q13", 2300},
    {"Q27Q6", 2301},
    {"Q27Q35", 2302},
    {"Q27Q43", 2303},
    {"Q27Q51", 2304},
    {"Q27Q59", 2305},
    {"Q27Q19", 2306},
    {"Q27Q11", 2307},
    {"Q27Q3", 2308},
    {"Q27Q28", 2309},
    {"Q27Q29", 2310},
    {"Q27Q30", 2311},
    {"Q27Q31", 2312},
    {"Q27Q26", 2313},
    {"Q27Q25", 2314},
    {"Q27Q24", 2315},
    {"Q28Q37", 2316},
    {"Q28Q46", 2317},
    {"Q28Q55", 2318},
    {"Q28Q19", 2319},
    {"Q28Q10", 2320},
    {"Q28Q1", 2321},
    {"Q28Q35", 2322},
    {"Q28Q42", 2323},
    {"Q28Q49", 2324},
    {"Q28Q56", 2325},
    {"Q28Q21", 2326},
    {"Q28Q14", 2327},
    {"Q28Q7", 2328},
    {"Q28Q36", 2329},
    {"Q28Q44", 2330},
    {"Q28Q52", 2331},
    {"Q28Q60", 2332},
    {"Q28Q20", 2333},
    {"Q28Q12", 2334},
    {"Q28Q4", 2335},
    {"Q28Q29", 2336},
    {"Q28Q30", 2337},
    {"Q28Q31", 2338},
    {"Q28Q27", 2339},
    {"Q28Q26", 2340},
    {"Q28Q25", 2341},
    {"Q28Q24", 2342},
    {"Q29Q38", 2343},
    {"Q29Q47", 2344},
    {"Q29Q20", 2345},
    {"Q29Q11", 2346},
    {"Q29Q2", 2347},
    {"Q29Q36", 2348},
    {"Q29Q43", 2349},
    {"Q29Q50", 2350},
    {"Q29Q57", 2351},
    {"Q29Q22", 2352},
    {"Q29Q15", 2353},
    {"Q29Q37", 2354},
    {"Q29Q45", 2355},
    {"Q29Q53", 2356},
    {"Q29Q61", 2357},
    {"Q29Q21", 2358},
    {"Q29Q13", 2359},
    {"Q29Q5", 2360},
    {"Q29Q30", 2361},
    {"Q29Q31", 2362},
    {"Q29Q28", 2363},
    {"Q29Q27", 2364},
    {"Q29Q26", 2365},
    {"Q29Q25", 2366},
    {"Q29Q24", 2367},
    {"Q30Q39", 2368},
    {"Q30Q21", 2369},
    {"Q30Q12", 2370},
    {"Q30Q3", 2371},
    {"Q30Q37", 2372},
    {"Q30Q44", 2373},
    {"Q30Q51", 2374},
    {"Q30Q58", 2375},
    {"Q30Q23", 2376},
    {"Q30Q38", 2377},
    {"Q30Q46", 2378},
    {"Q30Q54", 2379},
    {"Q30Q62", 2380},
    {"Q30Q22", 2381},
    {"Q30Q14", 2382},
    {"Q30Q6", 2383},
    {"Q30Q31", 2384},
    {"Q30Q29", 2385},
    {"Q30Q28", 2386},
    {"Q30Q27", 2387},
    {"Q30Q26", 2388},
    {"Q30Q25", 2389},
    {"Q30Q24", 2390},
    {"Q31Q22", 2391},
    {"Q31Q13", 2392},
    {"Q31Q4", 2393},
    {"Q31Q38", 2394},
    {"Q31Q45", 2395},
    {"Q31Q52", 2396},
    {"Q31Q59", 2397},
    {"Q31Q39", 2398},
    {"Q31Q47", 2399},
    {"Q31Q55", 2400},
    {"Q31Q63", 2401},
    {"Q31Q23", 2402},
    {"Q31Q15", 2403},
    {"Q31Q7", 2404},
    {"Q31Q30", 2405},
    {"Q31Q29", 2406},
    {"Q31Q28", 2407},
    {"Q31Q27", 2408},
    {"Q31Q26", 2409},
    {"Q31Q25", 2410},
    {"Q31Q24", 2411},
    {"Q32Q41", 2412},
    {"Q32Q50", 2413},
    {"Q32Q59", 2414},
    {"Q32Q25", 2415},
    {"Q32Q18", 2416},
    {"Q32Q11", 2417},
    {"Q32Q4", 2418},
    {"Q32Q40", 2419},
    {"Q32Q48", 2420},
    {"Q32Q56", 2421},
    {"Q32Q24", 2422},
    {"Q32Q16", 2423},
    {"Q32Q8", 2424},
    {"Q32Q0", 2425},
    {"Q32Q33", 2426},
    {"Q32Q34", 2427},
    {"Q32Q35", 2428},
    {"Q32Q36", 2429},
    {"Q32Q37", 2430},
    {"Q32Q38", 2431},
    {"Q32Q39", 2432},
    {"Q33Q42", 2433},
    {"Q33Q51", 2434},
    {"Q33Q60", 2435},
    {"Q33Q24", 2436},
    {"Q33Q40", 2437},
    {"Q33Q26", 2438},
    {"Q33Q19", 2439},
    {"Q33Q12", 2440},
    {"Q33Q5", 2441},
    {"Q33Q41", 2442},
    {"Q33Q49", 2443},
    {"Q33Q57", 2444},
    {"Q33Q25", 2445},
    {"Q33Q17", 2446},
    {"Q33Q9", 2447},
    {"Q33Q1", 2448},
    {"Q33Q34", 2449},
    {"Q33Q35", 2450},
    {"Q33Q36", 2451},
    {"Q33Q37", 2452},
    {"Q33Q38", 2453},
    {"Q33Q39", 2454},
    {"Q33Q32", 2455},
    {"Q34Q43", 2456},
    {"Q34Q52", 2457},
    {"Q34Q61", 2458},
    {"Q34Q25", 2459},
    {"Q34Q16", 2460},
    {"Q34Q41", 2461},
    {"Q34Q48", 2462},
    {"Q34Q27", 2463},
    {"Q34Q20", 2464},
    {"Q34Q13", 2465},
    {"Q34Q6", 2466},
    {"Q34Q42", 2467},
    {"Q34Q50", 2468},
    {"Q34Q58", 2469},
    {"Q34Q26", 2470},
    {"Q34Q18", 2471},
    {"Q34Q10", 2472},
    {"Q34Q2", 2473},
    {"Q34Q35", 2474},
    {"Q34Q36", 2475},
    {"Q34Q37", 2476},
    {"Q34Q38", 2477},
    {"Q34Q39", 2478},
    {"Q34Q33", 2479},
    {"Q34Q32", 2480},
    {"Q35Q44", 2481},
    {"Q35Q53", 2482},
    {"Q35Q62", 2483},
    {"Q35Q26", 2484},
    {"Q35Q17", 2485},
    {"Q35Q8", 2486},
    {"Q35Q42", 2487},
    {"Q35Q49", 2488},
    {"Q35Q56", 2489},
    {"Q35Q28", 2490},
    {"Q35Q21", 2491},
    {"Q35Q14", 2492},
    {"Q35Q7", 2493},
    {"Q35Q43", 2494},
    {"Q35Q51", 2495},
    {"Q35Q59", 2496},
    {"Q35Q27", 2497},
    {"Q35Q19", 2498},
    {"Q35Q11", 2499},
    {"Q35Q3", 2500},
    {"Q35Q36", 2501},
    {"Q35Q37", 2502},
    {"Q35Q38", 2503},
    {"Q35Q39", 2504},
    {"Q35Q34", 2505},
    {"Q35Q33", 2506},
    {"Q35Q32", 2507},
    {"Q36Q45", 2508},
    {"Q36Q54", 2509},
    {"Q36Q63", 2510},
    {"Q36Q27", 2511},
    {"Q36Q18", 2512},
    {"Q36Q9", 2513},
    {"Q36Q0", 2514},
    {"Q36Q43", 2515},
    {"Q36Q50", 2516},
    {"Q36Q57", 2517},
    {"Q36Q29", 2518},
    {"Q36Q22", 2519},
    {"Q36Q15", 2520},
    {"Q36Q44", 2521},
    {"Q36Q52", 2522},
    {"Q36Q60", 2523},
    {"Q36Q28", 2524},
    {"Q36Q20", 2525},
    {"Q36Q12", 2526},
    {"Q36Q4", 2527},
    {"Q36Q37", 2528},
    {"Q36Q38", 2529},
    {"Q36Q39", 2530},
    {"Q36Q35", 2531},
    {"Q36Q34", 2532},
    {"Q36Q33", 2533},
    {"Q36Q32", 2534},
    {"Q37Q46", 2535},
    {"Q37Q55", 2536},
    {"Q37Q28", 2537},
    {"Q37Q19", 2538},
    {"Q37Q10", 2539},
    {"Q37Q1", 2540},
    {"Q37Q44", 2541},
    {"Q37Q51", 2542},
    {"Q37Q58", 2543},
    {"Q37Q30", 2544},
    {"Q37Q23", 2545},
    {"Q37Q45", 2546},
    {"Q37Q53", 2547},
    {"Q37Q61", 2548},
    {"Q37Q29", 2549},
    {"Q37Q21", 2550},
    {"Q37Q13", 2551},
    {"Q37Q5", 2552},
    {"Q37Q38", 2553},
    {"Q37Q39", 2554},
    {"Q37Q36", 2555},
    {"Q37Q35", 2556},
    {"Q37Q34", 2557},
    {"Q37Q33", 2558},
    {"Q37Q32", 2559},
    {"Q38Q47", 2560},
    {"Q38Q29", 2561},
    {"Q38Q20", 2562},
    {"Q38Q11", 2563},
    {"Q38Q2", 2564},
    {"Q38Q45", 2565},
    {"Q38Q52", 2566},
    {"Q38Q59", 2567},
    {"Q38Q31", 2568},
    {"Q38Q46", 2569},
    {"Q38Q54", 2570},
    {"Q38Q62", 2571},
    {"Q38Q30", 2572},
    {"Q38Q22", 2573},
    {"Q38Q14", 2574},
    {"Q38Q6", 2575},
    {"Q38Q39", 2576},
    {"Q38Q37", 2577},
    {"Q38Q36", 2578},
    {"Q38Q35", 2579},
    {"Q38Q34", 2580},
    {"Q38Q33", 2581},
    {"Q38Q32", 2582},
    {"Q39Q30", 2583},
    {"Q39Q21", 2584},
    {"Q39Q12", 2585},
    {"Q39Q3", 2586},
    {"Q39Q46", 2587},
    {"Q39Q53", 2588},
    {"Q39Q60", 2589},
    {"Q39Q47", 2590},
    {"Q39Q55", 2591},
    {"Q39Q63", 2592},
    {"Q39Q31", 2593},
    {"Q39Q23", 2594},
    {"Q39Q15", 2595},
    {"Q39Q7", 2596},
    {"Q39Q38", 2597},
    {"Q39Q37", 2598},
    {"Q39Q36", 2599},
    {"Q39Q35", 2600},
    {"Q39Q34", 2601},
    {"Q39Q33", 2602},
    {"Q39Q32", 2603},
    {"Q40Q49", 2604},
    {"Q40Q58", 2605},
    {"Q40Q33", 2606},
    {"Q40Q26", 2607},
    {"Q40Q19", 2608},
    {"Q40Q12", 2609},
    {"Q40Q5", 2610},
    {"Q40Q48", 2611},
    {"Q40Q56", 2612},
    {"Q40Q32", 2613},
    {"Q40Q24", 2614},
    {"Q40Q16", 2615},
    {"Q40Q8", 2616},
    {"Q40Q0", 2617},
    {"Q40Q41", 2618},
    {"Q40Q42", 2619},
    {"Q40Q43", 2620},
    {"Q40Q44", 2621},
    {"Q40Q45", 2622},
    {"Q40Q46", 2623},
    {"Q40Q47", 2624},
    {"Q41Q50", 2625},
    {"Q41Q59", 2626},
    {"Q41Q32", 2627},
    {"Q41Q48", 2628},
    {"Q41Q34", 2629},
    {"Q41Q27", 2630},
    {"Q41Q20", 2631},
    {"Q41Q13", 2632},
    {"Q41Q6", 2633},
    {"Q41Q49", 2634},
    {"Q41Q57", 2635},
    {"Q41Q33", 2636},
    {"Q41Q25", 2637},
    {"Q41Q17", 2638},
    {"Q41Q9", 2639},
    {"Q41Q1", 2640},
    {"Q41Q42", 2641},
    {"Q41Q43", 2642},
    {"Q41Q44", 2643},
    {"Q41Q45", 2644},
    {"Q41Q46", 2645},
    {"Q41Q47", 2646},
    {"Q41Q40", 2647},
    {"Q42Q51", 2648},
    {"Q42Q60", 2649},
    {"Q42Q33", 2650},
    {"Q42Q24", 2651},
    {"Q42Q49", 2652},
    {"Q42Q56", 2653},
    {"Q42Q35", 2654},
    {"Q42Q28", 2655},
    {"Q42Q21", 2656},
    {"Q42Q14", 2657},
    {"Q42Q7", 2658},
    {"Q42Q50", 2659},
    {"Q42Q58", 2660},
    {"Q42Q34", 2661},
    {"Q42Q26", 2662},
    {"Q42Q18", 2663},
    {"Q42Q10", 2664},
    {"Q42Q2", 2665},
    {"Q42Q43", 2666},
    {"Q42Q44", 2667},
    {"Q42Q45", 2668},
    {"Q42Q46", 2669},
    {"Q42Q47", 2670},
    {"Q42Q41", 2671},
    {"Q42Q40", 2672},
    {"Q43Q52", 2673},
    {"Q43Q61", 2674},
    {"Q43Q34", 2675},
    {"Q43Q25", 2676},
    {"Q43Q16", 2677},
    {"Q43Q50", 2678},
    {"Q43Q57", 2679},
    {"Q43Q36", 2680},
    {"Q43Q29", 2681},
    {"Q43Q22", 2682},
    {"Q43Q15", 2683},
    {"Q43Q51", 2684},
    {"Q43Q59", 2685},
    {"Q43Q35", 2686},
    {"Q43Q27", 2687},
    {"Q43Q19", 2688},
    {"Q43Q11", 2689},
    {"Q43Q3", 2690},
    {"Q43Q44", 2691},
    {"Q43Q45", 2692},
    {"Q43Q46", 2693},
    {"Q43Q47", 2694},
    {"Q43Q42", 2695},
    {"Q43Q41", 2696},
    {"Q43Q40", 2697},
    {"Q44Q53", 2698},
    {"Q44Q62", 2699},
    {"Q44Q35", 2700},
    {"Q44Q26", 2701},
    {"Q44Q17", 2702},
    {"Q44Q8", 2703},
    {"Q44Q51", 2704},
    {"Q44Q58", 2705},
    {"Q44Q37", 2706},
    {"Q44Q30", 2707},
    {"Q44Q23", 2708},
    {"Q44Q52", 2709},
    {"Q44Q60", 2710},
    {"Q44Q36", 2711},
    {"Q44Q28", 2712},
    {"Q44Q20", 2713},
    {"Q44Q12", 2714},
    {"Q44Q4", 2715},
    {"Q44Q45", 2716},
    {"Q44Q46", 2717},
    {"Q44Q47", 2718},
    {"Q44Q43", 2719},
    {"Q44Q42", 2720},
    {"Q44Q41", 2721},
    {"Q44Q40", 2722},
    {"Q45Q54", 2723},
    {"Q45Q63", 2724},
    {"Q45Q36", 2725},
    {"Q45Q27", 2726},
    {"Q45Q18", 2727},
    {"Q45Q9", 2728},
    {"Q45Q0", 2729},
    {"Q45Q52", 2730},
    {"Q45Q59", 2731},
    {"Q45Q38", 2732},
    {"Q45Q31", 2733},
    {"Q45Q53", 2734},
    {"Q45Q61", 2735},
    {"Q45Q37", 2736},
    {"Q45Q29", 2737},
    {"Q45Q21", 2738},
    {"Q45Q13", 2739},
    {"Q45Q5", 2740},
    {"Q45Q46", 2741},
    {"Q45Q47", 2742},
    {"Q45Q44", 2743},
    {"Q45Q43", 2744},
    {"Q45Q42", 2745},
    {"Q45Q41", 2746},
    {"Q45Q40", 2747},
    {"Q46Q55", 2748},
    {"Q46Q37", 2749},
    {"Q46Q28", 2750},
    {"Q46Q19", 2751},
    {"Q46Q10", 2752},
    {"Q46Q1", 2753},
    {"Q46Q53", 2754},
    {"Q46Q60", 2755},
    {"Q46Q39", 2756},
    {"Q46Q54", 2757},
    {"Q46Q62", 2758},
    {"Q46Q38", 2759},
    {"Q46Q30", 2760},
    {"Q46Q22", 2761},
    {"Q46Q14", 2762},
    {"Q46Q6", 2763},
    {"Q46Q47", 2764},
    {"Q46Q45", 2765},
    {"Q46Q44", 2766},
    {"Q46Q43", 2767},
    {"Q46Q42", 2768},
    {"Q46Q41", 2769},
    {"Q46Q40", 2770},
    {"Q47Q38", 2771},
    {"Q47Q29", 2772},
    {"Q47Q20", 2773},
    {"Q47Q11", 2774},
    {"Q47Q2", 2775},
    {"Q47Q54", 2776},
    {"Q47Q61", 2777},
    {"Q47Q55", 2778},
    {"Q47Q63", 2779},
    {"Q47Q39", 2780},
    {"Q47Q31", 2781},
    {"Q47Q23", 2782},
    {"Q47Q15", 2783},
    {"Q47Q7", 2784},
    {"Q47Q46", 2785},
    {"Q47Q45", 2786},
    {"Q47Q44", 2787},
    {"Q47Q43", 2788},
    {"Q47Q42", 2789},
    {"Q47Q41", 2790},
    {"Q47Q40", 2791},
    {"Q48Q57", 2792},
    {"Q48Q41", 2793},
    {"Q48Q34", 2794},
    {"Q48Q27", 2795},
    {"Q48Q20", 2796},
    {"Q48Q13", 2797},
    {"Q48Q6", 2798},
    {"Q48Q56", 2799},
    {"Q48Q40", 2800},
    {"Q48Q32", 2801},
    {"Q48Q24", 2802},
    {"Q48Q16", 2803},
    {"Q48Q8", 2804},
    {"Q48Q0", 2805},
    {"Q48Q49", 2806},
    {"Q48Q50", 2807},
    {"Q48Q51", 2808},
    {"Q48Q52", 2809},
    {"Q48Q53", 2810},
    {"Q48Q54", 2811},
    {"Q48Q55", 2812},
    {"Q49Q58", 2813},
    {"Q49Q40", 2814},
    {"Q49Q56", 2815},
    {"Q49Q42", 2816},
    {"Q49Q35", 2817},
    {"Q49Q28", 2818},
    {"Q49Q21", 2819},
    {"Q49Q14", 2820},
    {"Q49Q7", 2821},
    {"Q49Q57", 2822},
    {"Q49Q41", 2823},
    {"Q49Q33", 2824},
    {"Q49Q25", 2825},
    {"Q49Q17", 2826},
    {"Q49Q9", 2827},
    {"Q49Q1", 2828},
    {"Q49Q50", 2829},
    {"Q49Q51", 2830},
    {"Q49Q52", 2831},
    {"Q49Q53", 2832},
    {"Q49Q54", 2833},
    {"Q49Q55", 2834},
    {"Q49Q48", 2835},
    {"Q50Q59", 2836},
    {"Q50Q41", 2837},
    {"Q50Q32", 2838},
    {"Q50Q57", 2839},
    {"Q50Q43", 2840},
    {"Q50Q36", 2841},
    {"Q50Q29", 2842},
    {"Q50Q22", 2843},
    {"Q50Q15", 2844},
    {"Q50Q58", 2845},
    {"Q50Q42", 2846},
    {"Q50Q34", 2847},
    {"Q50Q26", 2848},
    {"Q50Q18", 2849},
    {"Q50Q10", 2850},
    {"Q50Q2", 2851},
    {"Q50Q51", 2852},
    {"Q50Q52", 2853},
    {"Q50Q53", 2854},
    {"Q50Q54", 2855},
    {"Q50Q55", 2856},
    {"Q50Q49", 2857},
    {"Q50Q48", 2858},
    {"Q51Q60", 2859},
    {"Q51Q42", 2860},
    {"Q51Q33", 2861},
    {"Q51Q24", 2862},
    {"Q51Q58", 2863},
    {"Q51Q44", 2864},
    {"Q51Q37", 2865},
    {"Q51Q30", 2866},
    {"Q51Q23", 2867},
    {"Q51Q59", 2868},
    {"Q51Q43", 2869},
    {"Q51Q35", 2870},
    {"Q51Q27", 2871},
    {"Q51Q19", 2872},
    {"Q51Q11", 2873},
    {"Q51Q3", 2874},
    {"Q51Q52", 2875},
    {"Q51Q53", 2876},
    {"Q51Q54", 2877},
    {"Q51Q55", 2878},
    {"Q51Q50", 2879},
    {"Q51Q49", 2880},
    {"Q51Q48", 2881},
    {"Q52Q61", 2882},
    {"Q52Q43", 2883},
    {"Q52Q34", 2884},
    {"Q52Q25", 2885},
    {"Q52Q16", 2886},
    {"Q52Q59", 2887},
    {"Q52Q45", 2888},
    {"Q52Q38", 2889},
    {"Q52Q31", 2890},
    {"Q52Q60", 2891},
    {"Q52Q44", 2892},
    {"Q52Q36", 2893},
    {"Q52Q28", 2894},
    {"Q52Q20", 2895},
    {"Q52Q12", 2896},
    {"Q52Q4", 2897},
    {"Q52Q53", 2898},
    {"Q52Q54", 2899},
    {"Q52Q55", 2900},
    {"Q52Q51", 2901},
    {"Q52Q50", 2902},
    {"Q52Q49", 2903},
    {"Q52Q48", 2904},
    {"Q53Q62", 2905},
    {"Q53Q44", 2906},
    {"Q53Q35", 2907},
    {"Q53Q26", 2908},
    {"Q53Q17", 2909},
    {"Q53Q8", 2910},
    {"Q53Q60", 2911},
    {"Q53Q46", 2912},
    {"Q53Q39", 2913},
    {"Q53Q61", 2914},
    {"Q53Q45", 2915},
    {"Q53Q37", 2916},
    {"Q53Q29", 2917},
    {"Q53Q21", 2918},
    {"Q53Q13", 2919},
    {"Q53Q5", 2920},
    {"Q53Q54", 2921},
    {"Q53Q55", 2922},
    {"Q53Q52", 2923},
    {"Q53Q51", 2924},
    {"Q53Q50", 2925},
    {"Q53Q49", 2926},
    {"Q53Q48", 2927},
    {"Q54Q63", 2928},
    {"Q54Q45", 2929},
    {"Q54Q36", 2930},
    {"Q54Q27", 2931},
    {"Q54Q18", 2932},
    {"Q54Q9", 2933},
    {"Q54Q0", 2934},
    {"Q54Q61", 2935},
    {"Q54Q47", 2936},
    {"Q54Q62", 2937},
    {"Q54Q46", 2938},
    {"Q54Q38", 2939},
    {"Q54Q30", 2940},
    {"Q54Q22", 2941},
    {"Q54Q14", 2942},
    {"Q54Q6", 2943},
    {"Q54Q55", 2944},
    {"Q54Q53", 2945},
    {"Q54Q52", 2946},
    {"Q54Q51", 2947},
    {"Q54Q50", 2948},
    {"Q54Q49", 2949},
    {"Q54Q48", 2950},
    {"Q55Q46", 2951},
    {"Q55Q37", 2952},
    {"Q55Q28", 2953},
    {"Q55Q19", 2954},
    {"Q55Q10", 2955},
    {"Q55Q1", 2956},
    {"Q55Q62", 2957},
    {"Q55Q63", 2958},
    {"Q55Q47", 2959},
    {"Q55Q39", 2960},
    {"Q55Q31", 2961},
    {"Q55Q23", 2962},
    {"Q55Q15", 2963},
    {"Q55Q7", 2964},
    {"Q55Q54", 2965},
    {"Q55Q53", 2966},
    {"Q55Q52", 2967},
    {"Q55Q51", 2968},
    {"Q55Q50", 2969},
    {"Q55Q49", 2970},
    {"Q55Q48", 2971},
    {"Q56Q49", 2972},
    {"Q56Q42", 2973},
    {"Q56Q35", 2974},
    {"Q56Q28", 2975},
    {"Q56Q21", 2976},
    {"Q56Q14", 2977},
    {"Q56Q7", 2978},
    {"Q56Q48", 2979},
    {"Q56Q40", 2980},
    {"Q56Q32", 2981},
    {"Q56Q24", 2982},
    {"Q56Q16", 2983},
    {"Q56Q8", 2984},
    {"Q56Q0", 2985},
    {"Q56Q57", 2986},
    {"Q56Q58", 2987},
    {"Q56Q59", 2988},
    {"Q56Q60", 2989},
    {"Q56Q61", 2990},
    {"Q56Q62", 2991},
    {"Q56Q63", 2992},
    {"Q57Q48", 2993},
    {"Q57Q50", 2994},
    {"Q57Q43", 2995},
    {"Q57Q36", 2996},
    {"Q57Q29", 2997},
    {"Q57Q22", 2998},
    {"Q57Q15", 2999},
    {"Q57Q49", 3000},
    {"Q57Q41", 3001},
    {"Q57Q33", 3002},
    {"Q57Q25", 3003},
    {"Q57Q17", 3004},
    {"Q57Q9", 3005},
    {"Q57Q1", 3006},
    {"Q57Q58", 3007},
    {"Q57Q59", 3008},
    {"Q57Q60", 3009},
    {"Q57Q61", 3010},
    {"Q57Q62", 3011},
    {"Q57Q63", 3012},
    {"Q57Q56", 3013},
    {"Q58Q49", 3014},
    {"Q58Q40", 3015},
    {"Q58Q51", 3016},
    {"Q58Q44", 3017},
    {"Q58Q37", 3018},
    {"Q58Q30", 3019},
    {"Q58Q23", 3020},
    {"Q58Q50", 3021},
    {"Q58Q42", 3022},
    {"Q58Q34", 3023},
    {"Q58Q26", 3024},
    {"Q58Q18", 3025},
    {"Q58Q10", 3026},
    {"Q58Q2", 3027},
    {"Q58Q59", 3028},
    {"Q58Q60", 3029},
    {"Q58Q61", 3030},
    {"Q58Q62", 3031},
    {"Q58Q63", 3032},
    {"Q58Q57", 3033},
    {"Q58Q56", 3034},
    {"Q59Q50", 3035},
    {"Q59Q41", 3036},
    {"Q59Q32", 3037},
    {"Q59Q52", 3038},
    {"Q59Q45", 3039},
    {"Q59Q38", 3040},
    {"Q59Q31", 3041},
    {"Q59Q51", 3042},
    {"Q59Q43", 3043},
    {"Q59Q35", 3044},
    {"Q59Q27", 3045},
    {"Q59Q19", 3046},
    {"Q59Q11", 3047},
    {"Q59Q3", 3048},
    {"Q59Q60", 3049},
    {"Q59Q61", 3050},
    {"Q59Q62", 3051},
    {"Q59Q63", 3052},
    {"Q59Q58", 3053},
    {"Q59Q57", 3054},
    {"Q59Q56", 3055},
    {"Q60Q51", 3056},
    {"Q60Q42", 3057},
    {"Q60Q33", 3058},
    {"Q60Q24", 3059},
    {"Q60Q53", 3060},
    {"Q60Q46", 3061},
    {"Q60Q39", 3062},
    {"Q60Q52", 3063},
    {"Q60Q44", 3064},
    {"Q60Q36", 3065},
    {"Q60Q28", 3066},
    {"Q60Q20", 3067},
    {"Q60Q12", 3068},
    {"Q60Q4", 3069},
    {"Q60Q61", 3070},
    {"Q60Q62", 3071},
    {"Q60Q63", 3072},
    {"Q60Q59", 3073},
    {"Q60Q58", 3074},
    {"Q60Q57", 3075},
    {"Q60Q56", 3076},
    {"Q61Q52", 3077},
    {"Q61Q43", 3078},
    {"Q61Q34", 3079},
    {"Q61Q25", 3080},
    {"Q61Q16", 3081},
    {"Q61Q54", 3082},
    {"Q61Q47", 3083},
    {"Q61Q53", 3084},
    {"Q61Q45", 3085},
    {"Q61Q37", 3086},
    {"Q61Q29", 3087},
    {"Q61Q21", 3088},
    {"Q61Q13", 3089},
    {"Q61Q5", 3090},
    {"Q61Q62", 3091},
    {"Q61Q63", 3092},
    {"Q61Q60", 3093},
    {"Q61Q59", 3094},
    {"Q61Q58", 3095},
    {"Q61Q57", 3096},
    {"Q61Q56", 3097},
    {"Q62Q53", 3098},
    {"Q62Q44", 3099},
    {"Q62Q35", 3100},
    {"Q62Q26", 3101},
    {"Q62Q17", 3102},
    {"Q62Q8", 3103},
    {"Q62Q55", 3104},
    {"Q62Q54", 3105},
    {"Q62Q46", 3106},
    {"Q62Q38", 3107},
    {"Q62Q30", 3108},
    {"Q62Q22", 3109},
    {"Q62Q14", 3110},
    {"Q62Q6", 3111},
    {"Q62Q63", 3112},
    {"Q62Q61", 3113},
    {"Q62Q60", 3114},
    {"Q62Q59", 3115},
    {"Q62Q58", 3116},
    {"Q62Q57", 3117},
    {"Q62Q56", 3118},
    {"Q63Q54", 3119},
    {"Q63Q45", 3120},
    {"Q63Q36", 3121},
    {"Q63Q27", 3122},
    {"Q63Q18", 3123},
    {"Q63Q9", 3124},
    {"Q63Q0", 3125},
    {"Q63Q55", 3126},
    {"Q63Q47", 3127},
    {"Q63Q39", 3128},
    {"Q63Q31", 3129},
    {"Q63Q23", 3130},
    {"Q63Q15", 3131},
    {"Q63Q7", 3132},
    {"Q63Q62", 3133},
    {"Q63Q61", 3134},
    {"Q63Q60", 3135},
    {"Q63Q59", 3136},
    {"Q63Q58", 3137},
    {"Q63Q57", 3138},
    {"Q63Q56", 3139},
    {"N0N10", 3140},
    {"N0N17", 3141},
    {"N1N11", 3142},
    {"N1N16", 3143},
    {"N1N18", 3144},
    {"N2N8", 3145},
    {"N2N12", 3146},
    {"N2N17", 3147},
    {"N2N19", 3148},
    {"N3N9", 3149},
    {"N3N13", 3150},
    {"N3N18", 3151},
    {"N3N20", 3152},
    {"N4N10", 3153},
    {"N4N14", 3154},
    {"N4N19", 3155},
    {"N4N21", 3156},
    {"N5N11", 3157},
    {"N5N15", 3158},
    {"N5N20", 3159},
    {"N5N22", 3160},
    {"N6N12", 3161},
    {"N6N21", 3162},
    {"N6N23", 3163},
    {"N7N13", 3164},
    {"N7N22", 3165},
    {"N8N2", 3166},
    {"N8N18", 3167},
    {"N8N25", 3168},
    {"N9N3", 3169},
    {"N9N19", 3170},
    {"N9N24", 3171},
    {"N9N26", 3172},
    {"N10N0", 3173},
    {"N10N4", 3174},
    {"N10N16", 3175},
    {"N10N20", 3176},
    {"N10N25", 3177},
    {"N10N27", 3178},
    {"N11N1", 3179},
    {"N11N5", 3180},
    {"N11N17", 3181},
    {"N11N21", 3182},
    {"N11N26", 3183},
    {"N11N28", 3184},
    {"N12N2", 3185},
    {"N12N6", 3186},
    {"N12N18", 3187},
    {"N12N22", 3188},
    {"N12N27", 3189},
    {"N12N29", 3190},
    {"N13N3", 3191},
    {"N13N7", 3192},
    {"N13N19", 3193},
    {"N13N23", 3194},
    {"N13N28", 3195},
    {"N13N30", 3196},
    {"N14N4", 3197},
    {"N14N20", 3198},
    {"N14N29", 3199},
    {"N14N31", 3200},
    {"N15N5", 3201},
    {"N15N21", 3202},
    {"N15N30", 3203},
    {"N16N1", 3204},
    {"N16N10", 3205},
    {"N16N26", 3206},
    {"N16N33", 3207},
    {"N17N0", 3208},
    {"N17N2", 3209},
    {"N17N11", 3210},
    {"N17N27", 3211},
    {"N17N32", 3212},
    {"N17N34", 3213},
    {"N18N1", 3214},
    {"N18N3", 3215},
    {"N18N8", 3216},
    {"N18N12", 3217},
    {"N18N24", 3218},
    {"N18N28", 3219},
    {"N18N33", 3220},
    {"N18N35", 3221},
    {"N19N2", 3222},
    {"N19N4", 3223},
    {"N19N9", 3224},
    {"N19N13", 3225},
    {"N19N25", 3226},
    {"N19N29", 3227},
    {"N19N34", 3228},
    {"N19N36", 3229},
    {"N20N3", 3230},
    {"N20N5", 3231},
    {"N20N10", 3232},
    {"N20N14", 3233},
    {"N20N26", 3234},
    {"N20N30", 3235},
    {"N20N35", 3236},
    {"N20N37", 3237},
    {"N21N4", 3238},
    {"N21N6", 3239},
    {"N21N11", 3240},
    {"N21N15", 3241},
    {"N21N27", 3242},
    {"N21N31", 3243},
    {"N21N36", 3244},
    {"N21N38", 3245},
    {"N22N5", 3246},
    {"N22N7", 3247},
    {"N22N12", 3248},
    {"N22N28", 3249},
    {"N22N37", 3250},
    {"N22N39", 3251},
    {"N23N6", 3252},
    {"N23N13", 3253},
    {"N23N29", 3254},
    {"N23N38", 3255},
    {"N24N9", 3256},
    {"N24N18", 3257},
    {"N24N34", 3258},
    {"N24N41", 3259},
    {"N25N8", 3260},
    {"N25N10", 3261},
    {"N25N19", 3262},
    {"N25N35", 3263},
    {"N25N40", 3264},
    {"N25N42", 3265},
    {"N26N9", 3266},
    {"N26N11", 3267},
    {"N26N16", 3268},
    {"N26N20", 3269},
    {"N26N32", 3270},
    {"N26N36", 3271},
    {"N26N41", 3272},
    {"N26N43", 3273},
    {"N27N10", 3274},
    {"N27N12", 3275},
    {"N27N17", 3276},
    {"N27N21", 3277},
    {"N27N33", 3278},
    {"N27N37", 3279},
    {"N27N42", 3280},
    {"N27N44", 3281},
    {"N28N11", 3282},
    {"N28N13", 3283},
    {"N28N18", 3284},
    {"N28N22", 3285},
    {"N28N34", 3286},
    {"N28N38", 3287},
    {"N28N43", 3288},
    {"N28N45", 3289},
    {"N29N12", 3290},
    {"N29N14", 3291},
    {"N29N19", 3292},
    {"N29N23", 3293},
    {"N29N35", 3294},
    {"N29N39", 3295},
    {"N29N44", 3296},
    {"N29N46", 3297},
    {"N30N13", 3298},
    {"N30N15", 3299},
    {"N30N20", 3300},
    {"N30N36", 3301},
    {"N30N45", 3302},
    {"N30N47", 3303},
    {"N31N14", 3304},
    {"N31N21", 3305},
    {"N31N37", 3306},
    {"N31N46", 3307},
    {"N32N17", 3308},
    {"N32N26", 3309},
    {"N32N42", 3310},
    {"N32N49", 3311},
    {"N33N16", 3312},
    {"N33N18", 3313},
    {"N33N27", 3314},
    {"N33N43", 3315},
    {"N33N48", 3316},
    {"N33N50", 3317},
    {"N34N17", 3318},
    {"N34N19", 3319},
    {"N34N24", 3320},
    {"N34N28", 3321},
    {"N34N40", 3322},
    {"N34N44", 3323},
    {"N34N49", 3324},
    {"N34N51", 3325},
    {"N35N18", 3326},
    {"N35N20", 3327},
    {"N35N25", 3328},
    {"N35N29", 3329},
    {"N35N41", 3330},
    {"N35N45", 3331},
    {"N35N50", 3332},
    {"N35N52", 3333},
    {"N36N19", 3334},
    {"N36N21", 3335},
    {"N36N26", 3336},
    {"N36N30", 3337},
    {"N36N42", 3338},
    {"N36N46", 3339},
    {"N36N51", 3340},
    {"N36N53", 3341},
    {"N37N20", 3342},
    {"N37N22", 3343},
    {"N37N27", 3344},
    {"N37N31", 3345},
    {"N37N43", 3346},
    {"N37N47", 3347},
    {"N37N52", 3348},
    {"N37N54", 3349},
    {"N38N21", 3350},
    {"N38N23", 3351},
    {"N38N28", 3352},
    {"N38N44", 3353},
    {"N38N53", 3354},
    {"N38N55", 3355},
    {"N39N22", 3356},
    {"N39N29", 3357},
    {"N39N45", 3358},
    {"N39N54", 3359},
    {"N40N25", 3360},
    {"N40N34", 3361},
    {"N40N50", 3362},
    {"N40N57", 3363},
    {"N41N24", 3364},
    {"N41N26", 3365},
    {"N41N35", 3366},
    {"N41N51", 3367},
    {"N41N56", 3368},
    {"N41N58", 3369},
    {"N42N25", 3370},
    {"N42N27", 3371},
    {"N42N32", 3372},
    {"N42N36", 3373},
    {"N42N48", 3374},
    {"N42N52", 3375},
    {"N42N57", 3376},
    {"N42N59", 3377},
    {"N43N26", 3378},
    {"N43N28", 3379},
    {"N43N33", 3380},
    {"N43N37", 3381},
    {"N43N49", 3382},
    {"N43N53", 3383},
    {"N43N58", 3384},
    {"N43N60", 3385},
    {"N44N27", 3386},
    {"N44N29", 3387},
    {"N44N34", 3388},
    {"N44N38", 3389},
    {"N44N50", 3390},
    {"N44N54", 3391},
    {"N44N59", 3392},
    {"N44N61", 3393},
    {"N45N28", 3394},
    {"N45N30", 3395},
    {"N45N35", 3396},
    {"N45N39", 3397},
    {"N45N51", 3398},
    {"N45N55", 3399},
    {"N45N60", 3400},
    {"N45N62", 3401},
    {"N46N29", 3402},
    {"N46N31", 3403},
    {"N46N36", 3404},
    {"N46N52", 3405},
    {"N46N61", 3406},
    {"N46N63", 3407},
    {"N47N30", 3408},
    {"N47N37", 3409},
    {"N47N53", 3410},
    {"N47N62", 3411},
    {"N48N33", 3412},
    {"N48N42", 3413},
    {"N48N58", 3414},
    {"N49N32", 3415},
    {"N49N34", 3416},
    {"N49N43", 3417},
    {"N49N59", 3418},
    {"N50N33", 3419},
    {"N50N35", 3420},
    {"N50N40", 3421},
    {"N50N44", 3422},
    {"N50N56", 3423},
    {"N50N60", 3424},
    {"N51N34", 3425},
    {"N51N36", 3426},
    {"N51N41", 3427},
    {"N51N45", 3428},
    {"N51N57", 3429},
    {"N51N61", 3430},
    {"N52N35", 3431},
    {"N52N37", 3432},
    {"N52N42", 3433},
    {"N52N46", 3434},
    {"N52N58", 3435},
    {"N52N62", 3436},
    {"N53N36", 3437},
    {"N53N38", 3438},
    {"N53N43", 3439},
    {"N53N47", 3440},
    {"N53N59", 3441},
    {"N53N63", 3442},
    {"N54N37", 3443},
    {"N54N39", 3444},
    {"N54N44", 3445},
    {"N54N60", 3446},
    {"N55N38", 3447},
    {"N55N45", 3448},
    {"N55N61", 3449},
    {"N56N41", 3450},
    {"N56N50", 3451},
    {"N57N40", 3452},
    {"N57N42", 3453},
    {"N57N51", 3454},
    {"N58N41", 3455},
    {"N58N43", 3456},
    {"N58N48", 3457},
    {"N58N52", 3458},
    {"N59N42", 3459},
    {"N59N44", 3460},
    {"N59N49", 3461},
    {"N59N53", 3462},
    {"N60N43", 3463},
    {"N60N45", 3464},
    {"N60N50", 3465},
    {"N60N54", 3466},
    {"N61N44", 3467},
    {"N61N46", 3468},
    {"N61N51", 3469},
    {"N61N55", 3470},
    {"N62N45", 3471},
    {"N62N47", 3472},
    {"N62N52", 3473},
    {"N63N46", 3474},
    {"N63N53", 3475},
    {"K0K1", 3476},
    {"K0K8", 3477},
    {"K0K9", 3478},
    {"K1K0", 3479},
    {"K1K2", 3480},
    {"K1K8", 3481},
    {"K1K9", 3482},
    {"K1K10", 3483},
    {"K2K1", 3484},
    {"K2K3", 3485},
    {"K2K9", 3486},
    {"K2K10", 3487},
    {"K2K11", 3488},
    {"K3K2", 3489},
    {"K3K4", 3490},
    {"K3K10", 3491},
    {"K3K11", 3492},
    {"K3K12", 3493},
    {"K4K3", 3494},
    {"K4K5", 3495},
    {"K4K11", 3496},
    {"K4K12", 3497},
    {"K4K13", 3498},
    {"K5K4", 3499},
    {"K5K6", 3500},
    {"K5K12", 3501},
    {"K5K13", 3502},
    {"K5K14", 3503},
    {"K6K5", 3504},
    {"K6K7", 3505},
    {"K6K13", 3506},
    {"K6K14", 3507},
    {"K6K15", 3508},
    {"K7K6", 3509},
    {"K7K14", 3510},
    {"K7K15", 3511},
    {"K8K0", 3512},
    {"K8K1", 3513},
    {"K8K9", 3514},
    {"K8K16", 3515},
    {"K8K17", 3516},
    {"K9K0", 3517},
    {"K9K1", 3518},
    {"K9K2", 3519},
    {"K9K8", 3520},
    {"K9K10", 3521},
    {"K9K16", 3522},
    {"K9K17", 3523},
    {"K9K18", 3524},
    {"K10K1", 3525},
    {"K10K2", 3526},
    {"K10K3", 3527},
    {"K10K9", 3528},
    {"K10K11", 3529},
    {"K10K17", 3530},
    {"K10K18", 3531},
    {"K10K19", 3532},
    {"K11K2", 3533},
    {"K11K3", 3534},
    {"K11K4", 3535},
    {"K11K10", 3536},
    {"K11K12", 3537},
    {"K11K18", 3538},
    {"K11K19", 3539},
    {"K11K20", 3540},
    {"K12K3", 3541},
    {"K12K4", 3542},
    {"K12K5", 3543},
    {"K12K11", 3544},
    {"K12K13", 3545},
    {"K12K19", 3546},
    {"K12K20", 3547},
    {"K12K21", 3548},
    {"K13K4", 3549},
    {"K13K5", 3550},
    {"K13K6", 3551},
    {"K13K12", 3552},
    {"K13K14", 3553},
    {"K13K20", 3554},
    {"K13K21", 3555},
    {"K13K22", 3556},
    {"K14K5", 3557},
    {"K14K6", 3558},
    {"K14K7", 3559},
    {"K14K13", 3560},
    {"K14K15", 3561},
    {"K14K21", 3562},
    {"K14K22", 3563},
    {"K14K23", 3564},
    {"K15K6", 3565},
    {"K15K7", 3566},
    {"K15K14", 3567},
    {"K15K22", 3568},
    {"K15K23", 3569},
    {"K16K8", 3570},
    {"K16K9", 3571},
    {"K16K17", 3572},
    {"K16K24", 3573},
    {"K16K25", 3574},
    {"K17K8", 3575},
    {"K17K9", 3576},
    {"K17K10", 3577},
    {"K17K16", 3578},
    {"K17K18", 3579},
    {"K17K24", 3580},
    {"K17K25", 3581},
    {"K17K26", 3582},
    {"K18K9", 3583},
    {"K18K10", 3584},
    {"K18K11", 3585},
    {"K18K17", 3586},
    {"K18K19", 3587},
    {"K18K25", 3588},
    {"K18K26", 3589},
    {"K18K27", 3590},
    {"K19K10", 3591},
    {"K19K11", 3592},
    {"K19K12", 3593},
    {"K19K18", 3594},
    {"K19K20", 3595},
    {"K19K26", 3596},
    {"K19K27", 3597},
    {"K19K28", 3598},
    {"K20K11", 3599},
    {"K20K12", 3600},
    {"K20K13", 3601},
    {"K20K19", 3602},
    {"K20K21", 3603},
    {"K20K27", 3604},
    {"K20K28", 3605},
    {"K20K29", 3606},
    {"K21K12", 3607},
    {"K21K13", 3608},
    {"K21K14", 3609},
    {"K21K20", 3610},
    {"K21K22", 3611},
    {"K21K28", 3612},
    {"K21K29", 3613},
    {"K21K30", 3614},
    {"K22K13", 3615},
    {"K22K14", 3616},
    {"K22K15", 3617},
    {"K22K21", 3618},
    {"K22K23", 3619},
    {"K22K29", 3620},
    {"K22K30", 3621},
    {"K22K31", 3622},
    {"K23K14", 3623},
    {"K23K15", 3624},
    {"K23K22", 3625},
    {"K23K30", 3626},
    {"K23K31", 3627},
    {"K24K16", 3628},
    {"K24K17", 3629},
    {"K24K25", 3630},
    {"K24K32", 3631},
    {"K24K33", 3632},
    {"K25K16", 3633},
    {"K25K17", 3634},
    {"K25K18", 3635},
    {"K25K24", 3636},
    {"K25K26", 3637},
    {"K25K32", 3638},
    {"K25K33", 3639},
    {"K25K34", 3640},
    {"K26K17", 3641},
    {"K26K18", 3642},
    {"K26K19", 3643},
    {"K26K25", 3644},
    {"K26K27", 3645},
    {"K26K33", 3646},
    {"K26K34", 3647},
    {"K26K35", 3648},
    {"K27K18", 3649},
    {"K27K19", 3650},
    {"K27K20", 3651},
    {"K27K26", 3652},
    {"K27K28", 3653},
    {"K27K34", 3654},
    {"K27K35", 3655},
    {"K27K36", 3656},
    {"K28K19", 3657},
    {"K28K20", 3658},
    {"K28K21", 3659},
    {"K28K27", 3660},
    {"K28K29", 3661},
    {"K28K35", 3662},
    {"K28K36", 3663},
    {"K28K37", 3664},
    {"K29K20", 3665},
    {"K29K21", 3666},
    {"K29K22", 3667},
    {"K29K28", 3668},
    {"K29K30", 3669},
    {"K29K36", 3670},
    {"K29K37", 3671},
    {"K29K38", 3672},
    {"K30K21", 3673},
    {"K30K22", 3674},
    {"K30K23", 3675},
    {"K30K29", 3676},
    {"K30K31", 3677},
    {"K30K37", 3678},
    {"K30K38", 3679},
    {"K30K39", 3680},
    {"K31K22", 3681},
    {"K31K23", 3682},
    {"K31K30", 3683},
    {"K31K38", 3684},
    {"K31K39", 3685},
    {"K32K24", 3686},
    {"K32K25", 3687},
    {"K32K33", 3688},
    {"K32K40", 3689},
    {"K32K41", 3690},
    {"K33K24", 3691},
    {"K33K25", 3692},
    {"K33K26", 3693},
    {"K33K32", 3694},
    {"K33K34", 3695},
    {"K33K40", 3696},
    {"K33K41", 3697},
    {"K33K42", 3698},
    {"K34K25", 3699},
    {"K34K26", 3700},
    {"K34K27", 3701},
    {"K34K33", 3702},
    {"K34K35", 3703},
    {"K34K41", 3704},
    {"K34K42", 3705},
    {"K34K43", 3706},
    {"K35K26", 3707},
    {"K35K27", 3708},
    {"K35K28", 3709},
    {"K35K34", 3710},
    {"K35K36", 3711},
    {"K35K42", 3712},
    {"K35K43", 3713},
    {"K35K44", 3714},
    {"K36K27", 3715},
    {"K36K28", 3716},
    {"K36K29", 3717},
    {"K36K35", 3718},
    {"K36K37", 3719},
    {"K36K43", 3720},
    {"K36K44", 3721},
    {"K36K45", 3722},
    {"K37K28", 3723},
    {"K37K29", 3724},
    {"K37K30", 3725},
    {"K37K36", 3726},
    {"K37K38", 3727},
    {"K37K44", 3728},
    {"K37K45", 3729},
    {"K37K46", 3730},
    {"K38K29", 3731},
    {"K38K30", 3732},
    {"K38K31", 3733},
    {"K38K37", 3734},
    {"K38K39", 3735},
    {"K38K45", 3736},
    {"K38K46", 3737},
    {"K38K47", 3738},
    {"K39K30", 3739},
    {"K39K31", 3740},
    {"K39K38", 3741},
    {"K39K46", 3742},
    {"K39K47", 3743},
    {"K40K32", 3744},
    {"K40K33", 3745},
    {"K40K41", 3746},
    {"K40K48", 3747},
    {"K40K49", 3748},
    {"K41K32", 3749},
    {"K41K33", 3750},
    {"K41K34", 3751},
    {"K41K40", 3752},
    {"K41K42", 3753},
    {"K41K48", 3754},
    {"K41K49", 3755},
    {"K41K50", 3756},
    {"K42K33", 3757},
    {"K42K34", 3758},
    {"K42K35", 3759},
    {"K42K41", 3760},
    {"K42K43", 3761},
    {"K42K49", 3762},
    {"K42K50", 3763},
    {"K42K51", 3764},
    {"K43K34", 3765},
    {"K43K35", 3766},
    {"K43K36", 3767},
    {"K43K42", 3768},
    {"K43K44", 3769},
    {"K43K50", 3770},
    {"K43K51", 3771},
    {"K43K52", 3772},
    {"K44K35", 3773},
    {"K44K36", 3774},
    {"K44K37", 3775},
    {"K44K43", 3776},
    {"K44K45", 3777},
    {"K44K51", 3778},
    {"K44K52", 3779},
    {"K44K53", 3780},
    {"K45K36", 3781},
    {"K45K37", 3782},
    {"K45K38", 3783},
    {"K45K44", 3784},
    {"K45K46", 3785},
    {"K45K52", 3786},
    {"K45K53", 3787},
    {"K45K54", 3788},
    {"K46K37", 3789},
    {"K46K38", 3790},
    {"K46K39", 3791},
    {"K46K45", 3792},
    {"K46K47", 3793},
    {"K46K53", 3794},
    {"K46K54", 3795},
    {"K46K55", 3796},
    {"K47K38", 3797},
    {"K47K39", 3798},
    {"K47K46", 3799},
    {"K47K54", 3800},
    {"K47K55", 3801},
    {"K48K40", 3802},
    {"K48K41", 3803},
    {"K48K49", 3804},
    {"K48K56", 3805},
    {"K48K57", 3806},
    {"K49K40", 3807},
    {"K49K41", 3808},
    {"K49K42", 3809},
    {"K49K48", 3810},
    {"K49K50", 3811},
    {"K49K56", 3812},
    {"K49K57", 3813},
    {"K49K58", 3814},
    {"K50K41", 3815},
    {"K50K42", 3816},
    {"K50K43", 3817},
    {"K50K49", 3818},
    {"K50K51", 3819},
    {"K50K57", 3820},
    {"K50K58", 3821},
    {"K50K59", 3822},
    {"K51K42", 3823},
    {"K51K43", 3824},
    {"K51K44", 3825},
    {"K51K50", 3826},
    {"K51K52", 3827},
    {"K51K58", 3828},
    {"K51K59", 3829},
    {"K51K60", 3830},
    {"K52K43", 3831},
    {"K52K44", 3832},
    {"K52K45", 3833},
    {"K52K51", 3834},
    {"K52K53", 3835},
    {"K52K59", 3836},
    {"K52K60", 3837},
    {"K52K61", 3838},
    {"K53K44", 3839},
    {"K53K45", 3840},
    {"K53K46", 3841},
    {"K53K52", 3842},
    {"K53K54", 3843},
    {"K53K60", 3844},
    {"K53K61", 3845},
    {"K53K62", 3846},
    {"K54K45", 3847},
    {"K54K46", 3848},
    {"K54K47", 3849},
    {"K54K53", 3850},
    {"K54K55", 3851},
    {"K54K61", 3852},
    {"K54K62", 3853},
    {"K54K63", 3854},
    {"K55K46", 3855},
    {"K55K47", 3856},
    {"K55K54", 3857},
    {"K55K62", 3858},
    {"K55K63", 3859},
    {"K56K48", 3860},
    {"K56K49", 3861},
    {"K56K57", 3862},
    {"K57K48", 3863},
    {"K57K49", 3864},
    {"K57K50", 3865},
    {"K57K56", 3866},
    {"K57K58", 3867},
    {"K58K49", 3868},
    {"K58K50", 3869},
    {"K58K51", 3870},
    {"K58K57", 3871},
    {"K58K59", 3872},
    {"K59K50", 3873},
    {"K59K51", 3874},
    {"K59K52", 3875},
    {"K59K58", 3876},
    {"K59K60", 3877},
    {"K60K51", 3878},
    {"K60K52", 3879},
    {"K60K53", 3880},
    {"K60K59", 3881},
    {"K60K61", 3882},
    {"K61K52", 3883},
    {"K61K53", 3884},
    {"K61K54", 3885},
    {"K61K60", 3886},
    {"K61K62", 3887},
    {"K62K53", 3888},
    {"K62K54", 3889},
    {"K62K55", 3890},
    {"K62K61", 3891},
    {"K62K63", 3892},
    {"K63K54", 3893},
    {"K63K55", 3894},
    {"K63K62", 3895},
    {"CL", 3896},
    {"CR", 3897}
};
    
                                        
                                        
                                

         
         
      
