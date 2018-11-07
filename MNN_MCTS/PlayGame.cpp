//To be compiled as g++ -ffast-math -O3 -std=c++11 -o PlayGame PlayGame.cpp MCTS.cpp Tree.cpp Chess.cpp net.c

//TODO: Adjust brian to make the soft matt and the other part automatically and make it a bit more elegant
//TODO: Functions to print the training datasets for the network
#include "Chess.hpp"
#include "Tree.hpp"
#include "MCTS.hpp"
#include "net.h"

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <string>
#include <iostream>
#include <fstream>

#define MAX_N_MOVES 600
#define N_GAMES 100
#define SHOW_GAMES 0

int main(int argc, char* argv[]) {
    if(argc < 3) {
        printf("Error, give the numbers of the networks to use as a parameter!\n");
        exit(EXIT_FAILURE);
    }
	srand(time(0));
	srand48(time(0));

	unsigned int results[3] = {0};

	ChessState* currentState;

	char white_pieces_network_name[25] = "pieces_network_";
	strcat(white_pieces_network_name, argv[1]);
	strcat(white_pieces_network_name, ".txt");
	char white_pawn_network_name[25] = "pawn_network_";
	strcat(white_pawn_network_name, argv[1]);
	strcat(white_pawn_network_name, ".txt");
	char white_rook_network_name[25] = "rook_network_";
	strcat(white_rook_network_name, argv[1]);
	strcat(white_rook_network_name, ".txt");
	char white_knight_network_name[25] = "knight_network_";
	strcat(white_knight_network_name, argv[1]);
	strcat(white_knight_network_name, ".txt");
	char white_bishop_network_name[25] = "bishop_network_";
	strcat(white_bishop_network_name, argv[1]);
	strcat(white_bishop_network_name, ".txt");
	char white_queen_network_name[25] = "queen_network_";
	strcat(white_queen_network_name, argv[1]);
	strcat(white_queen_network_name, ".txt");
	char white_king_network_name[25] = "king_network_";
	strcat(white_king_network_name, argv[1]);
	strcat(white_king_network_name, ".txt");
	
    NN* white_net1 = new NN();
    load_net(white_net1, white_pieces_network_name);

    std::array<NN*, 6> white_nets2;
    white_nets2[PAWN] = new NN();
    white_nets2[ROOK] = new NN();
    white_nets2[KNIGHT] = new NN();
    white_nets2[BISHOP] = new NN();
    white_nets2[QUEEN] = new NN();
    white_nets2[KING] = new NN();
    load_net(white_nets2[PAWN], white_pawn_network_name);
    load_net(white_nets2[ROOK], white_rook_network_name);
    load_net(white_nets2[KNIGHT], white_knight_network_name);
    load_net(white_nets2[BISHOP], white_bishop_network_name);
    load_net(white_nets2[QUEEN], white_queen_network_name);
    load_net(white_nets2[KING], white_king_network_name);

	char black_pieces_network_name[25] = "pieces_network_";
	strcat(black_pieces_network_name, argv[2]);
	strcat(black_pieces_network_name, ".txt");
	char black_pawn_network_name[25] = "pawn_network_";
	strcat(black_pawn_network_name, argv[2]);
	strcat(black_pawn_network_name, ".txt");
	char black_rook_network_name[25] = "rook_network_";
	strcat(black_rook_network_name, argv[2]);
	strcat(black_rook_network_name, ".txt");
	char black_knight_network_name[25] = "knight_network_";
	strcat(black_knight_network_name, argv[2]);
	strcat(black_knight_network_name, ".txt");
	char black_bishop_network_name[25] = "bishop_network_";
	strcat(black_bishop_network_name, argv[2]);
	strcat(black_bishop_network_name, ".txt");
	char black_queen_network_name[25] = "queen_network_";
	strcat(black_queen_network_name, argv[2]);
	strcat(black_queen_network_name, ".txt");
	char black_king_network_name[25] = "king_network_";
	strcat(black_king_network_name, argv[2]);
	strcat(black_king_network_name, ".txt");
	
    NN* black_net1 = new NN();
    load_net(black_net1, black_pieces_network_name);

    std::array<NN*, 6> black_nets2;
    black_nets2[PAWN] = new NN();
    black_nets2[ROOK] = new NN();
    black_nets2[KNIGHT] = new NN();
    black_nets2[BISHOP] = new NN();
    black_nets2[QUEEN] = new NN();
    black_nets2[KING] = new NN();
    load_net(black_nets2[PAWN], black_pawn_network_name);
    load_net(black_nets2[ROOK], black_rook_network_name);
    load_net(black_nets2[KNIGHT], black_knight_network_name);
    load_net(black_nets2[BISHOP], black_bishop_network_name);
    load_net(black_nets2[QUEEN], black_queen_network_name);
    load_net(black_nets2[KING], black_king_network_name);

	for(int game=0;game<N_GAMES;game++) {
		std::cout << "Playing game " << (game+1) << " of " << N_GAMES << "\n";

		//Get the starting state
		currentState = new ChessState();


	    if(SHOW_GAMES == 1) {
	    	currentState->printState();
	    }


		//Initialize two MCTS players
		MCTS* Players[2];
		Players[0] = new MCTS(new ChessState(), white_net1, white_nets2, false);
		Players[1] = new MCTS(new ChessState(), black_net1, black_nets2, false);
		
		int Nmoves = 0;
		int player = 0;
		while((currentState->isFinalState() == 0) && (Nmoves < MAX_N_MOVES)) {
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

	        if(SHOW_GAMES == 1) {
	        	std::cout << "Move n. " << Nmoves << "\n";
	        	currentState->printState();
	        }
	        
	        Nmoves++;
		}
	
		if(currentState->getWinner() == 1)
		{
			//std::cout << "White won!\n\n";
			results[0]++;
		}
		else if(currentState->getWinner() == -1)
		{
			//std::cout << "Black won!\n\n";
			results[2]++;
		}
	    else {
	        //std::cout << "It's a draw!\n\n";
	        results[1]++;
	    }

	    std::cout << "\n\nResults:\nWhite won " << (100. * results[0] / (game+1)) << "%% of the games;\nBlack won " << (100. * results[2] / (game+1)) << "%% of the games;\nDraws " << (100. * results[1] / (game+1)) << "%% of the games;\n\n\n";


	    delete Players[0];
	    delete Players[1];
	}

    for(int i=0;i<6;i++) {
    	delete white_nets2[i];
    }
    delete white_net1;

    for(int i=0;i<6;i++) {
    	delete black_nets2[i];
    }
    delete black_net1;
}
