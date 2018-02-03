//To be compiled as g++ -std=c++11 -o main main.cpp MCTS.cpp Tree.cpp (then create makefile)

#include "Game.hpp"
#include "Tree.hpp"
#include "MCTS.hpp"

#include <stdlib.h>
#include <time.h>
#include <string>
#include <iostream>
#include <fstream>


int main() {
	srand(time(0));
	
	GameState *currentState;
	
	//Get the starting state
	currentState = new DraughtsState();
    currentState->printState();
	
	//Initialize two MCTS players
	MCTS neoCortex(currentState);
	
	int Nmoves = 0;
	while(currentState->isFinalState() == 0) {
		std::cout << "Playing move n. " << Nmoves << "\n";
		for(int i=0;i<MCTS_NUMBER_OF_SWEEPS;i++)
		{
			//std::cout << "Sweep number " << i << ".\n";
			neoCortex.sweep();
		}
		
		currentState = neoCortex.playBestMove();
        
        currentState->printState();
        //if(Nmoves == 4) exit(0);
        
        Nmoves++;
	}
	
	if(currentState->getWinner() == 1)
	{
		std::cout << "White won!\n\n";
	}
	else if(currentState->getWinner() == -1)
	{
		std::cout << "Black won!\n\n";
	}
    else {
        std::cout << "It's a draw!\n\n";
    }
}
			
