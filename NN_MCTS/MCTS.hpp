/*
    MCTS.hpp:
        Library for the implementation of the Monte Carlo Tree Search (MCTS) with the aid of reinforced machine learning.
        The constructor gets as an input a tree, or equivalently the root containing the starting state of the game and a neural network.
        The routine sweep is the core of the MCTS, divided in three phases:
            - Selection: a path along the currently explored tree is chosen according to the current statistics of the exploration and the NN output.
            - Expansion: the children fo the current leaf are created, expanding the tree.
            - Backpropagation: The evaluation of the current state by the neural network is backpropagated along the tree.

        The routines playMove and playBestMove choose one of the possible moves from the current root and move the root of the tree.

        @author: Massimiliano Chiappini 
        @contact: massimilianochiappini@gmail.com
        @version: 0.2
*/



#ifndef MCTS_HPP
#define MCTS_HPP


#include "Game.hpp"
#include "Tree.hpp"
#include "brian.hpp"


#define MCTS_CP 1.
#define MCTS_tau 1.

#define MCTS_NUMBER_OF_SWEEPS 100

#define MCTS_EPSILON 0.25
#define MCTS_ALPHA 1.


//Class which performs the Monte Carlo tree search
class MCTS {
  private:
    NeuralNetwork *net;
    Tree tree;
  
  
  public:
    //CONSTRUCTORS
    MCTS(Tree, NeuralNetwork*);
  
    MCTS(GameState*, NeuralNetwork*);


    //SET/GET methods
    Tree getTree(void);
  
  
    //MCTS
    void sweep(void);
    
    Node* selection(Node*);
   
    void expansion(Node*);
    
    void backPropagation(Node*);

    
    //Gameplay
    void playMove(GameState*);
    GameState* playBestMove(void);
    GameState* playRandomMove(void);
    GameState* playHighestFrequencyMove(void);;


    //Network training
    void printBoardEvaluations(void);


    //Destructor
    ~MCTS(void);
};

#endif
  
