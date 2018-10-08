//To be compiled as g++ -std=c++11 -o SelfPlay SelfPlay.cpp MCTS.cpp Tree.cpp Chess.cpp -I./Brian/inc/ -L./Brian/lib/ -lbrian
//After having compiled Brian

//TODO: Adjust brian to make the soft matt and the tanh automatically
//TODO: Make tree of the Neural Network class as a pointer 

#include "Chess.hpp"
#include "Tree.hpp"
#include "MCTS.hpp"
#include "net.h"

#include <stdlib.h>
#include <time.h>
#include <string>
#include <iostream>
#include <fstream>


#define MAX_N_MOVES 240
#define N_GAMES 10
#define SHOW_GAMES 0


int main(int argc, char* argv[]) {
	srand(time(0));
	
	unsigned int results[3] = {0};

	ChessState* currentState;

	system("rm -rf TrainingSet");
	system("mkdir TrainingSet");

	char pieces_network_name[25] = "pieces_network.txt";
	char pawn_network_name[25] = "pawn_network.txt";
	char rook_network_name[25] = "rook_network.txt";
	char knight_network_name[25] = "knight_network.txt";
	char bishop_network_name[25] = "bishop_network.txt";
	char queen_network_name[25] = "queen_network.txt";
	char king_network_name[25] = "king_network.txt";
	
    NN* net1 = new NN();
    load_net(net1, pieces_network_name);

    std::array<NN*, 6> nets2;
    nets2[PAWN] = new NN();
    nets2[ROOK] = new NN();
    nets2[KNIGHT] = new NN();
    nets2[BISHOP] = new NN();
    nets2[QUEEN] = new NN();
    nets2[KING] = new NN();
    load_net(nets2[PAWN], pawn_network_name);
    load_net(nets2[ROOK], rook_network_name);
    load_net(nets2[KNIGHT], knight_network_name);
    load_net(nets2[BISHOP], bishop_network_name);
    load_net(nets2[QUEEN], queen_network_name);
    load_net(nets2[KING], king_network_name);
    
	//Perform N_GAMES self games
	for(int game=0;game<N_GAMES;game++) {
		std::cout << "Playing game " << (game+1) << " of " << N_GAMES << "\n";

		//Initialize the game
		currentState = new ChessState();


	    if(SHOW_GAMES == 1) {
	    	currentState->printState();
	    }

		//Initialize a MCTS players
		MCTS* neoCortex = new MCTS(currentState, net1, nets2, true);
		
		int Nmoves = 0;
		while((currentState->isFinalState() == 0) && (Nmoves < MAX_N_MOVES)) {
			for(int i=0;i<MCTS_NUMBER_OF_SWEEPS;i++)
			{
				//std::cout << "Sweep number " << i << ".\n";
				neoCortex->sweep();
			}
			
			currentState = neoCortex->playBestMove();
	        
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
    
    for(int i=0;i<6;i++) {
    	delete nets2[i];
    }
    delete net1;
}
			
