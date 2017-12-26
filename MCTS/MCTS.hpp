#include <vector>
#include <random>
#include <stdexcept>
#include <cmath>
#include <cfloat>
#include "Game.hpp"
#include "MCTS.hpp"



class MCTS {
  private:
    Tree tree; 
  
  
  public:
    //CONSTRUCTORS
    MCTS(Tree);
    MCTS(void);
  
  
    //MCTS
    void sweep(void);
    
    Node* selection(Node*);
    //In the selection, we get down picking time by time the highest UCT, until we reach a leaf. We return the pointer to the leaf.

    Node* expansion(Node*);
    //We are now in a leaf. If this leaf has never been visited before, do nothing. Else, if it is a final state, do nothing. Else, expand the tree adding a list of children. Then, pick a random child. Return the state (either the origianal one if nothing happend or the randomly picked one).
  
    double simulation(Node*);
    //Do a simulation from the leaf we are in (return the reward)
    
    void backPropagation(Node*, double);
    //Backpropagate the reward (return nothing)
  
    
    //Gameplay
    void playMove(GameState);
    //Force a move from outside (as an example, played by the opponent). This may be tricky. For instance, when we do this, let's be sure that the following layer of children has been generated, otherwise do that (unless it is a final state)
    state playBestMove(void);
    //Just pick the child with the highest UCT from the current tree's root
}
