/*
    MCTS.hpp:
        Library for the implementation of the Monte Carlo Tree Search (MCTS).
        The constructor gets as an input a tree, or equivalently the root containing the starting state of the game.
        The routine sweep is the core of the MCTS, divided in four phases:
            - Selection: a path along the currently explored tree is chosen according to the current values of the UCTs, until a leaf state is reached.
            - Expansion: the children fo the current leaf are created, expanding the tree.
            - Simulation: a random game simulation is performed from one of the child just created via the expansion, until the end of the game is reached and a reward
              is measured.
            - Backpropagation: The reward of the simulation is backpropagated along the tree, updating the UCT of the visited states.

        The routines playMove and playBestMove choose one of the possible moves from the current root and move the root of the tree.

        @author: Massimiliano Chiappini 
        @contact: massimilianochiappini@gmail.com
        @version: 0.2
*/



#ifndef MCTS_HPP
#define MCTS_HPP


#include "Game.hpp"
#include "Tree.hpp"


#define MCTS_CP 0.707

#define MCTS_NUMBER_OF_SWEEPS 1000


//Class which performs the Monte Carlo tree search
class MCTS {
  private:
    Tree tree;
  
  
  public:
    //CONSTRUCTORS
    MCTS(Tree);
  
    MCTS(GameState*);


    //SET/GET methods
    Tree getTree(void);
  
  
    //MCTS
    void sweep(void);
    
    Node* selection(Node*);
   
    Node* expansion(Node*);
  
    double simulation(Node*);
    
    void backPropagation(Node*, double);
  
    
    //Gameplay
    void playMove(GameState*);
    GameState* playBestMove(void);
};

#endif
  
