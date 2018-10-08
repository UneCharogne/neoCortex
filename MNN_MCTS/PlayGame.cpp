//To be compiled as g++ -std=c++11 -o PlayGame PlayGame.cpp MCTS.cpp Tree.cpp Game.cpp -I./Brian/inc/ -L./Brian/lib/ -lbrian

//TODO: Adjust brian to make the soft matt and the other part automatically and make it a bit more elegant
//TODO: Functions to print the training datasets for the network
#include "Chess.hpp"
#include "Tree.hpp"
#include "MCTS.hpp"
#include "brian.hpp"

#include <stdlib.h>
#include <time.h>
#include <string>
#include <iostream>
#include <fstream>


int main(int argc, char* argv[]) {
    if(argc < 3) {
        printf("Error, give the name of the networks to use as a parameter!\n");
        exit(EXIT_FAILURE);
    }
	
    std::string whiteNetworkName(argv[1]);
    std::string blackNetworkName(argv[2]);
    
	srand(time(0));
	
	ChessState *currentState;
	
	//Get the starting state
	currentState = new ChessState();
    currentState->printState();

	//Initialize two MCTS players
	MCTS* Players[2];
	Players[0] = new MCTS(new ChessState(), new NeuralNetwork(whiteNetworkName), false);
	Players[1] = new MCTS(new ChessState(), new NeuralNetwork(blackNetworkName), false);
	
	int Nmoves = 0;
	int player = 0;
	while(currentState->isFinalState() == 0) {
		std::cout << "Playing move n. " << Nmoves << "\n";

		//Think
		for(int i=0;i<MCTS_NUMBER_OF_SWEEPS;i++)
		{
			Players[0]->sweep();
			Players[1]->sweep();
		}
		
		//Play
		currentState = Players[player]->playBestMove();
		player = (player + 1) % 2;
		Players[player]->playMove(currentState);
		std::cout << "test.\n";

        currentState->printState();
        
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


    delete Players[0];
    delete Players[1];
}
			