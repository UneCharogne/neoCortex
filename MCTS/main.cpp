//To be compiled as g++ -std=c++11 -o main main.cpp MCTS.cpp Tree.cpp (then create makefile)

#include "Game.hpp"
#include "Tree.hpp"
#include "MCTS.hpp"

#include <stdlib.h>
#include <time.h>


int main() {
	srand(time(0));  
	
	GameState currentState;
	int player = 0;
	
	//Get the starting state
	currentState = GameState(STARTING_BOARD, 1);
	
	//Initialize two MCTS players
	std::vector<MCTS> players;
	players.push_back(MCTS(currentState));
	players.push_back(MCTS(currentState));
	
	while(currentState.isFinal() == 0) {
		for(int i=0;i<MCTS_NUMBER_OF_SWEEPS;i++)
		{
			players[0].sweep();
			players[1].sweep();
		}
		
		currentState = players[player].playBestMove();
		player = (player + 1) % 2;
		players[player].playMove(currentState);
		
		printBoard(currentState.getBoard());
	}
	
	if(currentState.isFinal() == 1)
	{
		cout << "White won!\n";
	}
	else
	{
		cout << "Black won!\n";
	}
}


void printBoard(Board board, string filename) {
	ofstream myfile;
	
	myfile.open(filename, ios::out | ios::app);
	
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
			
