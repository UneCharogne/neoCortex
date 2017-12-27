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
    MCTS(GameState);
    MCTS(void);
  
  
    //MCTS
    void sweep(void);
    
    Node* selection(Node*);
   
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


MCTS::MCTS(Tree tree) {
  this->tree = tree;
}

MCTS::MCTS(GameState state) : tree(Tree(state)) { }

MCTS::MCTS() : tree(Tree()) {}


//In the selection step, a path along the tree is followed through the states of highest UCT, a leaf is reached. The pointer to the (most promising) leaf is returned.
Node* MCTS::selection(Node* currentNode) {
  Node* nextNode;
  
  //Until a leaf is not reached
  while(currentNode->isLeaf() == NULL) {
    //Pick the most promising node of the current node as the next node
    nextNode = currentNode->getBestChild();
    
    currentNode = nextNode;
  }
  
  return currentNode;
}


//In the expansion, the tree is eventually expanded 
Node* MCTS::expansion(Node* currentNode) {  
  //The current node is a leaf state.
  //If it has never been visited before, nothing is done.
  if(currentNode->getNumberOfVisits != 0) {
    //Otherwise, if it is a final state, also nothing is done
    if(currentNode->getState().isFinalState() == false) {
      //Otherwise, the tree is expanded by adding a list of children
      currentNode->buildChildren();
      
      //And return a random child
      return currentNode->getRandomChild();
    }
  }
  
  return currentNode;
}
  
