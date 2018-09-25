//To be compiled as g++ -std=c++11 -o SelfPlay SelfPlay.cpp MCTS.cpp Tree.cpp Game.cpp -I./Brian/inc/ -L./Brian/lib/ -lbrian
//After having compiled Brian

//TODO: Adjust brian to make the soft matt and the tanh automatically
//TODO: Make tree of the Neural Network class as a pointer 

#include "Game.hpp"
#include "Tree.hpp"
#include "MCTS.hpp"
#include "brian.hpp"

#include <stdlib.h>
#include <time.h>
#include <string>
#include <iostream>
#include <fstream>


#define MAX_N_MOVES 120
#define N_GAMES 500
#define SHOW_GAMES 0
#define RANDOM_PLAYER -1


int main(int argc, char* argv[]) {
	if(argc == 1) {
        printf("Error, give the name of the network to use as a parameter!\n");
        exit(EXIT_FAILURE);
    }
    
    std::string networkName(argv[1]);
    
	srand(time(0));
	unsigned int results[3] = {0};

	GameState* currentState;

	system("rm -rf TrainingSet");
	system("mkdir TrainingSet");
    
    NeuralNetwork* net = new NeuralNetwork(networkName);


	//Perform N_GAMES self games
	for(int game=0;game<N_GAMES;game++) {
		std::cout << "Playing game " << game << " of " << N_GAMES << "\n";

		//Initialize the game
		currentState = new TicTacToeState();


	    if(SHOW_GAMES == 1) {
	    	currentState->printState();
	    }

		//Initialize a MCTS players
		MCTS* neoCortex = new MCTS(currentState, net);
		
		int Nmoves = 0;
		while((currentState->isFinalState() == 0) && (Nmoves < MAX_N_MOVES)) {
			for(int i=0;i<MCTS_NUMBER_OF_SWEEPS;i++)
			{
				//std::cout << "Sweep number " << i << ".\n";
				neoCortex->sweep();
			}
			
			if(currentState->getPlayer() == RANDOM_PLAYER) {
				currentState = neoCortex->playRandomMove();
			}
			else {
				currentState = neoCortex->playHighestFrequencyMove();
			}
	        
	        if(SHOW_GAMES == 1) {
	        	std::cout << "Move n. " << Nmoves << "\n";
	        	currentState->printState();
	        }
	        
	        Nmoves++;
		}

		neoCortex->printBoardEvaluations();
		
		if(currentState->getWinner() == 1)
		{
			std::cout << "White won!\n\n";
			results[0]++;
		}
		else if(currentState->getWinner() == -1)
		{
			std::cout << "Black won!\n\n";
			results[2]++;
		}
	    else {
	        std::cout << "It's a draw!\n\n";
	        results[1]++;
	    }

	    //Destroy and clean
	    delete neoCortex;
	}

	std::cout << "\n\nResults:\nWhite won " << (100 * results[0] / N_GAMES) << "%% of the games;\nBlack won " << (100 * results[2] / N_GAMES) << "%% of the games;\nDraws " << (100 * results[1] / N_GAMES) << "%% of the games;\n\n\n";
    
    delete net;
}
			
