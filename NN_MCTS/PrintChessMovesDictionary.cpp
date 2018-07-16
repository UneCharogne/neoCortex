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


int main() {
	CreateChessMovesDictionary();

	return 0;
}
			
