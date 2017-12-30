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
  
    GameState playBestMove(void);
};

#endif
  
