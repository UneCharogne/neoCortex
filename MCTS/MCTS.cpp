#include <vector>
#include <random>
#include <stdexcept>
#include <cmath>
#include <cfloat>
#include <iostream>
#include "Game.hpp"
#include "Tree.hpp"
#include "MCTS.hpp"


MCTS::MCTS(Tree tree, int player) {
  this->tree = tree;
  this->player = player;
}

MCTS::MCTS(GameState state, int player) : tree(Tree(state)), player(player) { }

MCTS::MCTS() : tree(Tree()), player(1) {}


//In the selection step, a path along the tree is followed through the states of highest UCT, a leaf is reached. The pointer to the (most promising) leaf is returned.
Node* MCTS::selection(Node* currentNode) {
  //std::cout << "Selection.\n";
  Node* nextNode;

  //Until a leaf is not reached
  while(currentNode->isLeaf() == false) {
    //Pick the most promising node of the current node as the next node
    nextNode = currentNode->getBestChild();
    
    currentNode = nextNode;
  }
  
  return currentNode;
}


//In the expansion, the tree is eventually expanded 
Node* MCTS::expansion(Node* currentNode) {
  //std::cout << "Expansion.\n";
  //The current node is a leaf state.
  //If it has never been visited before, nothing is done.
  if(currentNode->getNumberOfVisits() != 0) {
    //std::cout << "this state has been visited indeed.\n";
    //Otherwise, if it is a final state, also nothing is done
    if(currentNode->getState().isFinalState() == 0) {
      //Otherwise, the tree is expanded by adding a list of children
      //std::cout << "Children built!\n";
      currentNode->buildChildren();
      
      //And return a random child
      return currentNode->getRandomChild();
    }
  }
  
  return currentNode;
}


//A simulation is performed from the current node, and the reward is returned
double MCTS::simulation(Node* currentNode) {
  //std::cout << "Simulation.\n";
  //A simulation is performed from the current game state, and the reward is returned
  double reward = currentNode->getState().simulateGame();

  return reward;
}


//The result of the simulation from the leaf is backpropagated across the tree
void MCTS::backPropagation(Node* currentNode, double reward) {
  //We are in a leaf, from which we simulated a game that gave a certain reward
  //First of all, we have to update the number of visits and the total reward of the current state
  currentNode->increaseNumberOfVisits();
  currentNode->increaseReward(currentNode->getPlayer() * reward);
  
  //Then, we can backpropagate the reward until the root of the tree is found, updating the UCT along our path
  Node* parentNode = currentNode->getParent();
  while (parentNode!= NULL) {
    //Update the number of visits and the reward of the parent
    parentNode->increaseNumberOfVisits();
    parentNode->increaseReward(currentNode->getPlayer() * MCTS_OPPONENT_LEVEL * reward);
    
    //And therefore the UCT of the current node
    currentNode->updateUCT();
    
    //And then move to its parent
    currentNode = parentNode;
    parentNode = currentNode->getParent();
    //Until a root (with a null parent) is found, for which updating the UCT is pointless
  }
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
  //std::cout << "Playing a given move.\n";
  //Look for the move to play in all the children of the current state
  this->tree.setRoot(this->tree.getRoot()->getChildByState(state));
}


//Play the best move given the current exploration of the tree
GameState MCTS::playBestMove(void) {
  //std::cout << "Playing best move.\n";
  //Pick the best child node of the current root, and select it as the new root
  this->tree.setRoot(this->tree.getRoot()->getBestChild());
  
  //And return the corresponding state
  return this->tree.getRoot()->getState();
}
