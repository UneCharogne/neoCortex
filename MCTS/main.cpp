//To be compiled as g++ -std=c++11 -o main main.cpp MCTS.cpp Tree.cpp (then create makefile)

#include "Game.hpp"
#include "Tree.hpp"
#include "MCTS.hpp"

#include <stdlib.h>
#include <time.h>
#include <string>
#include <iostream>
#include <fstream>


void printBoard(Board, std::string, bool);


int main() {
	srand(time(0));  
	
	GameState currentState;
	int player = 0;
	
	//Get the starting state
	currentState = GameState(STARTING_BOARD, 1);
        printBoard(currentState.getBoard(), "moves.dat", true);
	
	//Initialize two MCTS players
	std::vector<MCTS> players;
	players.push_back(MCTS(currentState, 1));
	players.push_back(MCTS(currentState, -1));
	
	int Nmoves = 0;
	while(currentState.isFinalState() == 0) {
		std::cout << "Playing move n. " << Nmoves << "\n";
		for(int i=0;i<MCTS_NUMBER_OF_SWEEPS;i++)
		{
			//std::cout << "Sweep number " << i << ".\n";
			players[0].sweep();
			players[1].sweep();
		}
		
		currentState = players[player].playBestMove();
		player = (player + 1) % 2;
		players[player].playMove(currentState);
		
		printBoard(currentState.getBoard(), "moves.dat", false);

		Nmoves++;
	}
	
	if(currentState.isFinalState() == 1)
	{
		std::cout << "White won!\n";
	}
	else
	{
		std::cout << "Black won!\n";
	}
}


void printBoard(Board board, std::string filename, bool firstmove) {
	std::ofstream myfile;
	
        if(firstmove) {
		myfile.open(filename, std::ios::out | std::ios::trunc);
	}
	else {
		myfile.open(filename, std::ios::out | std::ios::app);
	}
	
	myfile << "\n\n";	
	for(int j=0;j<10;j++) {
		myfile << "_";
	}
	myfile << "\n";
	
	for(int i=7;i>=0;i--) {
		for(int j=0;j<8;j++) {
			if(board[(8*i)+j] == 0){
				myfile << "|_|";
			}
			if(board[(8*i)+j] == 1){
				myfile << "|o|";
			}
			if(board[(8*i)+j] == 2){
				myfile << "|O|";
			}
			if(board[(8*i)+j] == -1){
				myfile << "|x|";
			}
			if(board[(8*i)+j] == -2){
				myfile << "|X|";
			}
		}
		myfile << "\n";
	}
	
	myfile.close();
}
			
