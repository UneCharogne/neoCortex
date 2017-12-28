#include "Game.hpp"
#include "Tree.hpp"

#ifndef MCTS_HPP
#define MCTS_HPP

#define MCTS_CP 0.707

class MCTS {
  private:
    Tree tree; 
  
  
  public:
    //CONSTRUCTORS
    MCTS(Tree);
  
    MCTS(GameState);
  
    MCTS(void);
  
  
    //MCTS
    void sweep(void);
    
    Node* selection(Node*);
   
    Node* expansion(Node*);
  
    double simulation(Node*);
    
    void backPropagation(Node*, double);
  
    
    //Gameplay
    void playMove(GameState);
  
    state playBestMove(void);
}

#endif
  
