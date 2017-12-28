#include <vector>
#include <random>
#include <stdexcept>
#include <cmath>
#include <cfloat>
#include "Game.hpp"
#include "Tree.hpp"
#include "MCTS.hpp"


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


//A simulation is performed from the current node, and the reward is returned
double Node* MCTS::simulation(Node* currentNode, double reward) {
  double reward;
  
  //A simulation is performed from the current game state, and the reward is returned
  return currentNode->getState().simulateGame();
}


//The result of the simulation from the leaf is backpropagated across the tree
void MCTS::backPropagation(Node* currentNode, double) {
  do {
    //Update the state of the node
    currentNode->increaseNumberOfVisits();
    currentNode->increaseReward(reward);
    currentNode->updateUCT();
    
    //And then move to its parent
    currentNode = currentNode->getParent();
    
    //Until a non-legitimate (NULL) parent is reached
  } while (currentNode != NULL);    
}


void MCTS::sweep(void) {
  Node* currentNode;
  double reward;
  
  //Starting from the root of the tree
  currentNode = this->tree.getRoot();
  
  //Perform the series of operation of the MCTS sweep
  currentNode = this->selection(currentNode);
  currentNode = this->expansion(currentNode);
  reward = this->simulation(currentNode);
  this->backPropagation(currentNode, reward);
}


//Force to play a move (typically, a move played by the opponent in his turn)
void MCTS::playMove(GameState state) {
  //Look for the move to play in all the children of the current state
  this->tree.setRoot(this->tree.getRoot()->getChildByState(state));
}


//Play the best move given the current exploration of the tree
GameState MCTS::playBestMove(void) {
  //Pick the best child node of the current root, and select it as the new root
  this->tree.setRoot(this->tree.getRoot()->getBestChild());
  
  //And return the corresponding state
  return this->tree.getRoot()->getState();
}
