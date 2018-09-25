#include <vector>
#include <random>
#include <stdexcept>
#include <cmath>
#include <cfloat>
#include <iostream>
#include <fstream>
#include "Game.hpp"
#include "Tree.hpp"
#include "MCTS.hpp"
#include "brian.hpp"


MCTS::MCTS(Tree tree, NeuralNetwork *net) : tree(tree), net(net) { 
  this->tree.setNetwork(net);
}

MCTS::MCTS(GameState *state, NeuralNetwork *net) : tree(Tree(state, net)), net(net) { }


Tree MCTS::getTree(void) {
  return this->tree;
}


//In the selection step, a path along the tree is followed through the states of highest UCT, a leaf is reached. The pointer to the (most promising) leaf is returned.
Node* MCTS::selection(Node* currentNode) {
  //std::cout << "Selection.\n";
  Node* nextNode;
  
  //Until a leaf is not reached
  while(currentNode->isLeaf() == false) {
    //Pick the most promising child of the current node as the next node
    nextNode = currentNode->getBestChild();
    
    currentNode = nextNode;
  }
  
  return currentNode;
}


//In the expansion, the tree is eventually expanded 
void MCTS::expansion(Node* currentNode) {
  //std::cout << "Expansion.\n";
  //The current node is a leaf state.
  //If it is a final state, nothing is done
  if(currentNode->getState()->isFinalState() == false) {
    //Otherwise, the tree is expanded by adding a list of children
    currentNode->buildChildren();
  }
}


//The result of the simulation from the leaf is backpropagated across the tree
void MCTS::backPropagation(Node* currentNode) {
  std::vector<double> netOutput;
  double v;
    
  if(currentNode->getState()->isFinalState() == false) {
    this->net->setInput(currentNode->getState()->getNetworkInput());
    this->net->propagateSignal();
    netOutput = this->net->getOutput();
    v = tanh(netOutput[netOutput.size() - 1]);
  }
  else {
    v = currentNode->getPlayer() * currentNode->getState()->getWinner();
  }
    
  v = -v;

  //Tne , we have to update the number of visits and the action of the current state
  currentNode->increaseNumberOfVisits();
  currentNode->updateAction(v);

  
  //Then, we can backpropagate the reward until the root of the tree is found, updating the UCT along our path
  Node* parentNode = currentNode->getParent();
  while (currentNode != this->tree.getRoot()) {
    v = -v;

    //Update the number of visits and the reward of the parent
    parentNode->increaseNumberOfVisits();
    parentNode->increaseNumberOfChildrenVisits();
    parentNode->updateAction(v);
    //parentNode->updateAction(parentNode->getPlayer() * v);
    
    //And therefore the UCT of the current node
    parentNode->updateChildrenU();
    
    //And then move to its parent
    currentNode = parentNode;
    parentNode = currentNode->getParent();
    //Until a root is found, for which updating the U is pointless
  }
}


void MCTS::sweep(void) {
  Node* currentNode;
  
  //Starting from the root of the tree
  currentNode = this->tree.getRoot();
  
  //Perform the series of operation of the MCTS sweep
  currentNode = this->selection(currentNode);
  this->expansion(currentNode);
  this->backPropagation(currentNode);
}


//Force to play a move (typically, a move played by the opponent in his turn)
void MCTS::playMove(GameState *state) {
  //Look for the move to play in all the children of the current state
  Node* moveToPlay = this->tree.getRoot()->getChildByState(state);
  //Prune the other branches
  this->tree.getRoot()->pruneOtherBranches(moveToPlay);
  //Play the move
  this->tree.setRoot(moveToPlay);
}


//Play the best move given the current exploration of the tree
//TO DO CHANGE FROM GET BEST CHILD TO GET CHILD TO PLAY
GameState* MCTS::playBestMove(void) {
  //Print the input and target output for the network
  this->tree.getRoot()->printNetworkDataset();

  //Pick the best child node of the current root, and select it as the new root
  Node* moveToPlay = this->tree.getRoot()->getChildToPlay();
  //Prune the other branches
  this->tree.getRoot()->pruneOtherBranches(moveToPlay);
  //Play the move
  this->tree.setRoot(moveToPlay);
  return this->tree.getRoot()->getState();
}


//Play the best move given the current exploration of the tree
//TO DO CHANGE FROM GET BEST CHILD TO GET CHILD TO PLAY
GameState* MCTS::playRandomMove(void) {
  //Print the input and target output for the network
  this->tree.getRoot()->printNetworkDataset();

  //Pick the best child node of the current root, and select it as the new root
  Node* moveToPlay = this->tree.getRoot()->getRandomChild();
  //Prune the other branches
  this->tree.getRoot()->pruneOtherBranches(moveToPlay);
  //Play the move
  this->tree.setRoot(moveToPlay);
  return this->tree.getRoot()->getState();
}


//Play the best move given the current exploration of the tree
//TO DO CHANGE FROM GET BEST CHILD TO GET CHILD TO PLAY
GameState* MCTS::playHighestFrequencyMove(void) {
    //Print the input and target output for the network
    this->tree.getRoot()->printNetworkDataset();
    std::vector<Node*> children = this->tree.getRoot()->getChildren();
    
    Node* moveToPlay = children[0];
    for(int i=1; i<children.size(); i++) {
        if(children[i]->getNumberOfVisits() > moveToPlay->getNumberOfVisits()) {
            moveToPlay = children[i];
        }
    }
    
    //Prune the other branches
    this->tree.getRoot()->pruneOtherBranches(moveToPlay);
    //Play the move
    this->tree.setRoot(moveToPlay);
    return this->tree.getRoot()->getState();
}


void MCTS::printBoardEvaluations(void) {
  std::ofstream zfile;
    
  zfile.open("TrainingSet/z.dat", std::ios::out | std::ios::app);

  Node* currentState = this->tree.getRoot();
  int w = currentState->getState()->getWinner();

  std::vector<int> z;
  z.push_back(currentState->getPlayer() * w);
  while(currentState->getParent() != NULL) {
    currentState = currentState->getParent();
    z.push_back(currentState->getPlayer() * w);
  }

  for(int i=(z.size() - 1);i>0;i--) {
    zfile << (double)z[i] << "\n";
  }
  
  zfile.close();
}


MCTS::~MCTS(void) {
  this->getTree().deleteTree();

  //delete this->net;
}
