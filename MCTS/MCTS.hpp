#ifndef MCTS_HPP
#define MCTS_HPP


#include "Game.hpp"
#include "Tree.hpp"


#define MCTS_CP 0.707

#define MCTS_NUMBER_OF_SWEEPS 1000

//-1 for opponent who plays against himself, 0 for random opponent, 1 for good opponent
#define MCTS_OPPONENT_LEVEL 1


//Class which performs the Monte Carlo tree search
class MCTS {
  private:
    Tree tree; 

    int player;
  
  
  public:
    //CONSTRUCTORS
    MCTS(Tree, int);
  
    MCTS(GameState, int);
  
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
  
