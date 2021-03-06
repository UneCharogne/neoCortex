#include <vector>
#include <algorithm>
#include <stdexcept>
#include <cmath>
#include <stdlib.h>
#include <cfloat>
#include <iostream>
#include "Game.hpp"
#include "MCTS.hpp"
#include "Tree.hpp"


//NODE
//Ordering criterium based on UCT
struct CompareNodes {
    bool operator()(Node* &a, Node* &b) const {
         return a->getUCT() < b->getUCT();
    }
};
  
   
//CONSTRUCTORS
Node::Node(GameState *state, Node* parent)  : parent(parent), state(state) {
  this->n = 0;
  this->reward = 0;
  this->UCT = DBL_MAX;
}

Node::Node(GameState *state) : Node(state, (Node*)NULL) {}


//SET/GET METHODS
void Node::setParent(Node* parent) {
  this->parent = parent;
}

Node* Node::getParent(void) {
  return this->parent;
}

void Node::addChild(Node *newChild) {
  this->children.push_back(newChild);
  
  //Sort the children on ascending value of UCT
  this->sortChildren();
}

void Node::addChildren(std::vector<Node*> newChildren) {
  this->children.insert(this->children.end(), newChildren.begin(), newChildren.end());
  
  //Sort the children on ascending value of UCT
  this->sortChildren();
}

std::vector<Node*> Node::getChildren(void) {
  return this->children;
}

Node* Node::getRandomChild(void) {
  if(this->children.size() == 0)
  {
    throw std::runtime_error("Trying to get a child from a node with no children.");
    return NULL;
  }
  
  return this->children[std::rand()%this->children.size()];
}

Node* Node::getBestChild(void) {
  if(this->children.size() == 0)
  {
    throw std::runtime_error("Trying to get a child from a node with no children.");
    return NULL;
  }
  
  //If the children are already sorted in ascending order of UCT
  if(this->areChildrenSorted())
  {
    //Return the last child
    return this->children[this->children.size() - 1];
  }
  else
  {
    //Otherwise, sort them and then return the last one
    this->sortChildren();
    return this->children[this->children.size() - 1];
  }
}

Node* Node::getChildWithHighestReward(void) {
  Node* highestRewardedChild;
  double highestReward;

  if(this->children.size() == 0)
  {
    throw std::runtime_error("Trying to get a child from a node with no children.");
    return NULL;
  }
  
  //Then look for the child with the highest reward
  highestReward = -1 * DBL_MAX;
  for(int i=0;i<this->children.size();i++) {
    if(this->children[i]->getReward() > highestReward) {
      highestReward = this->children[i]->getReward();
      highestRewardedChild = this->children[i];
    }
  }

  return highestRewardedChild;
}
 


Node* Node::getChildByState(GameState *state) {
  //Cycle over all the children until one with a state matching the input one is found
  for(int i=0;i<this->children.size();i++) {
    if((*this->children[i]->getState()) == (*state)) {
      return this->children[i];
    }
  }
  
  //If no matching state is found, throw an exception and return a null vector
  throw std::runtime_error("No child with a state matching the searched one has been found.");
  return NULL;
}

  
void Node::setChildrenSorted(bool childrenSorted) {
  this->childrenSorted = childrenSorted;
}
  
  
bool Node::areChildrenSorted(void) {
  return this->childrenSorted;
}


GameState* Node::getState(void) {
  return this->state;
}


int Node::getPlayer(void) {
  return this->state->getPlayer();
}
  

void Node::increaseNumberOfVisits(void) {
  this->n++;
}


int Node::getNumberOfVisits(void) {
  return this->n;
}
  

void Node::increaseReward(double reward) {
  this->reward += reward;
}


double Node::getReward(void) {
  return this->reward;
}


int Node::getNumberOfChildren(void) {
  return this->children.size();
}


//MCTS
double Node::getUCT(void) {
  return this->UCT;
}


bool Node::isLeaf(void) {
  if(this->children.size() == 0)
  {
    return true;
  }
  else
  {
    return false;
  }
}


void Node::cutBranch(void) {
  std::vector<Node*>::iterator child;

  child = this->children.begin();
  while(child != this->children.end())
  {
    (*child)->cutBranch();

    child = this->children.erase(child); 
  }

  delete this->state;

  delete this;
}


void Node::pruneOtherBranches(Node* branchToSave) {
  std::vector<Node*>::iterator branch;

  branch = this->children.begin();
  while(branch != this->children.end()) {
    if((*branch) == branchToSave) {
      ++branch;
    }
    else {
      (*branch)->cutBranch();
      branch = this->children.erase(branch);
    }
  }
}


void Node::buildChildren(void) {
  std::vector<Move> legalMoves;
  std::vector<Node*> newChildren;
  
  //Get the legal moves from the current state
  legalMoves = this->state->getLegalMoves();

  //std::cout << "We try to build the children";
  
  if(legalMoves.size() != 0)
  {
    //std::cout << "There are legal moves from this state.\n";
    //Consequently build the array of children
    for(std::vector<Move>::iterator move = legalMoves.begin(); move != legalMoves.end(); ++move) {
     newChildren.push_back(new Node((*move).finalState, this));
     //std::cout << newChildren[newChildren.size()-1]->getParent();
    }
  
    //And add it to the node
    this->addChildren(newChildren);
  }
  else
  {
    throw std::runtime_error("Trying to build the children of a state with no legal moves.");
  }
}


void Node::sortChildren(void)
{
  std::sort(this->children.begin(), this->children.end(), CompareNodes());

  this->childrenSorted = true;
}
  
  
void Node::updateUCT(void)
{
  Node* parent;
  
  //If the node's parent is a legitimate node
  if(this->parent != NULL)
  {
    //In general, the parent's children won't be sorted anymore once the UCT is computed
    this->parent->setChildrenSorted(false);
    
    //Check if there are any visit of the node
    if(this->n != 0)
    {
      //Compute UCT
      this->UCT = ((this->reward / this->n) + MCTS_CP * sqrt(2.f * (log(this->parent->getNumberOfVisits()) / this->n)));
    }
    else
    {
      //Tecnically, UCT in this case is infinitely big. We can just use the biggest possible double number.
      this->UCT = DBL_MAX;
    }
  }
  else
  {
    throw std::runtime_error("Calculating the UCT of a node with a NULL parent.");
  }
}


//TREE
//CONSTRUCTORS
Tree::Tree(Node *root) : root(root) {}

Tree::Tree(GameState *state) : root(new Node(state)) {}


//SET/GET METHODS
void Tree::setRoot(Node *root) {
  this->root = root;
}


Node* Tree::getRoot(void) {
  return this->root;
}


void Tree::deleteTree(void) {
  while(this->root->getParent() != NULL) {
    this->setRoot(this->root->getParent());
  }

  this->root->cutBranch();
}
