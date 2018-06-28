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
#include "brian.hpp"


//Dirichlet noise functions
double gamma1(double alpha) {
  double umax, vmin, vmax;
  double u, t, t1;

  umax = pow((alpha/exp(1.0)), 0.5*alpha);
  vmin = -2.0/exp(1.0);
  vmax = 2.0*alpha/exp(1.0)/(exp(1.0)-alpha);
  do {
    u = (double) lrand48()/RAND_MAX;
    u *= umax;
    t = (double) lrand48()/RAND_MAX;
    t = (t*(vmax-vmin)+vmin)/u;
    t1 = exp(t/alpha);
  } while(2.0*log(u)>(t-t1));
  if(alpha>=0.01) {
    return t1;
  }
  else {
    return t/alpha;
  }
}

void dirichlet(double alpha, int size, double *p) {
  int i;
  double norm;

  norm = 0.0;
  for(i=0; i<size; i++) {
    *(p + i) = gamma1(alpha);
    norm += *(p + i);
  }
  for(i=0; i<size; i++) {
    *(p + i) /= norm;
  }
}


//NODE
//Ordering criterium based on Q+U
struct CompareNodes {
    bool operator()(Node* &a, Node* &b) const {
         return (a->getMeanAction() + a->getU()) < (b->getMeanAction() + b->getU());
    }
};
  
   
//CONSTRUCTORS
Node::Node(GameState *state, Node* parent, Tree *tree, double p, int id)  : parent(parent), state(state), tree(tree), p(p), id(id) {
  this->n = 0;
  this->nc = 0;
  this->W = 0;
  this->Q = 0;
}

Node::Node(GameState *state, Node* parent, Tree *tree, double p)  : parent(parent), state(state), tree(tree), p(p) {
  this->id = -1;
  this->n = 0;
  this->nc = 0;
  this->W = 0;
  this->Q = 0;
}

Node::Node(GameState *state, Node* parent, Tree *tree)  : parent(parent), state(state), tree(tree) {
  this->id = -1;
  this->n = 0;
  this->nc = 0;
  this->W = 0;
  this->Q = 0;
  this->p = 0;
}

Node::Node(GameState *state, Node* parent)  : parent(parent), state(state) {
  this->tree = NULL;
  this->id = -1;
  this->n = 0;
  this->nc = 0;
  this->W = 0;
  this->Q = 0;
  this->p = 0;
}

Node::Node(GameState *state, Tree *tree) : Node(state, (Node*)NULL, tree) {}

Node::Node(GameState *state) : Node(state, (Node*)NULL, (Tree*)NULL) {}


//SET/GET METHODS
void Node::setTree(Tree* tree) {
  this->tree = tree;
}

Tree* Node::getTree(void) {
  return this->tree;
}

void Node::setParent(Node* parent) {
  this->parent = parent;
}

Node* Node::getParent(void) {
  return this->parent;
}

int Node::getId(void) {
  return this->id;
}

void Node::addChild(Node *newChild) {
  this->children.push_back(newChild);
  
  //Sort the children on ascending value of Q+U
  this->sortChildren();
}

void Node::addChildren(std::vector<Node*> newChildren) {
  this->children.insert(this->children.end(), newChildren.begin(), newChildren.end());
  
  //Sort the children on ascending value of Q+U
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
  
  //If the children are already sorted in ascending order of Q+U
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


Node* Node::getChildToPlay(void) {
  //The MCTS will play one of the possible moves according to the play probability
  std::vector<double> moveProbabilities;
  double Normalization = 0;

  for(int i=0;i<this->children.size();i++) {
    //printf("this->children[%d]->getU = %lf\n", i, this->children[i]->getU());
    moveProbabilities.push_back(this->children[i]->getPlayProbability());
    Normalization += moveProbabilities[i];
    //printf("moveProbabilities[%d] = %lf\n", i, moveProbabilities[i]);
  }

  double r = ((double)std::rand() / RAND_MAX) * Normalization;
  int i = 0;
  Normalization = moveProbabilities[0];
  while(Normalization < r) {
    i++;
    Normalization += moveProbabilities[i];
  }

  return this->children[i];
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


int Node::getNumberOfVisits(void) {
  return this->n;
}


int Node::getNumberOfChildrenVisits(void) {
  return this->nc;
}


double Node::getP(void) {
  return this->p;
}


void Node::setP(double p) {
  this->p = p;
}


double Node::getTotalAction(void) {
  return this->W;
}


double Node::getMeanAction(void) {
  return this->Q;
}

double Node::getU(void) {
  return this->U;
}


//TOOPTIMIZE
double Node::getPlayProbability(void) {
  double Normalization = 0;
    
  std::vector<Node*> brothers = this->parent->getChildren();
  for(std::vector<Node*>::iterator bro = brothers.begin(); bro != brothers.end(); ++bro) {
    Normalization += pow((*bro)->getNumberOfVisits(), (1. / MCTS_tau));
  }
  return (pow(this->n, (1. / MCTS_tau)) / Normalization);
}


int Node::getNumberOfChildren(void) {
  return this->children.size();
}


//MCTS
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


void Node::printNetworkDataset(void) {
  //First of all, let's open the files to print the outputs
  FILE *fpstate, *fppies;

  if((fpstate = fopen("TrainingSet/states.dat", "a")) == NULL) {
    printf("Error opening the file \"TrainingSet/states.dat\", program will be arrested.\n");
    exit(EXIT_FAILURE);
  }
  if((fppies = fopen("TrainingSet/probabilities.dat", "a")) == NULL) {
    printf("Error opening the file \"TrainingSet/probabilities.dat\", program will be arrested.\n");
    exit(EXIT_FAILURE);
  }


  //Get the input for the network from the current state
  std::vector<double> networkInput = this->state->getNetworkInput();
  //And print it
  for(int i=0;i<networkInput.size();i++) {
    fprintf(fpstate, "%lf ", networkInput[i]);
  }
  fprintf(fpstate, "\n");
  fclose(fpstate);


  //Then, create a vector with the size of the network's output
  int Nprobabilities = (this->tree->getNetwork()->getOutputSize() - 1);
  std::vector<double> probabilities;
  //And fill it with zeroes
  for(int i=0;i<Nprobabilities;i++) {
    probabilities.push_back(0);
  }

  //Then, run over all the moves from this node
  for(std::vector<Node*>::iterator child = this->children.begin(); child != this->children.end(); ++child) {
    //And get their probability
    probabilities[(*child)->getId()] = (*child)->getPlayProbability();
  }

  //Then, print the probabilities
  for(int i=0;i<Nprobabilities;i++) {
    fprintf(fppies, "%lf ", probabilities[i]);
  }
  fprintf(fppies, "\n");
  fclose(fppies);
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


//TO DO
//IN BUILD CHILDREN (OR MAYBE IN THE CONSTRUCTOR) THE STATISTICS HAVE TO BE INITIALIZED (INCLUDING THE MOVE PROBABILITY OBTAINED BY THE NETWORK)
void Node::buildChildren(void) {
  std::vector<double> moveProbabilities;
  std::vector<Move> legalMoves;
  std::vector<Node*> newChildren;

  //Get the move probabilities from the current state as evaluated by the neural network
  //TODO
  this->tree->getNetwork()->setInput(this->state->getNetworkInput());
  this->tree->getNetwork()->propagateSignal();
  moveProbabilities = this->tree->getNetwork()->getOutput();

  double Normalization = 0;
  for(int i=0;i<(moveProbabilities.size() - 1);i++) {
    moveProbabilities[i] = exp(moveProbabilities[i]);
    Normalization += moveProbabilities[i];
  }
  for(int i=0;i<(moveProbabilities.size() - 1);i++) {
    moveProbabilities[i] /= Normalization;
  }

  //Get the legal moves from the current state
  legalMoves = this->state->getLegalMoves();

  
  if(legalMoves.size() != 0)
  {
    //Consequently build the array of children
    if(this == this->tree->getRoot()) {
      double *noises;

      if((noises = (double*)malloc(legalMoves.size() * sizeof(double))) == NULL) {
        std::cout << "Error allocating the memory for the dirichlet noises, program will be arrested.\n";
        exit(EXIT_FAILURE);
      }

      dirichlet(MCTS_ALPHA, legalMoves.size(), noises);

      double Normalization2 = 0;
      for(std::vector<Move>::iterator move = legalMoves.begin(); move != legalMoves.end(); ++move) {
        Normalization2 += moveProbabilities[(*move).id];
      }

      int i = 0;
      for(std::vector<Move>::iterator move = legalMoves.begin(); move != legalMoves.end(); ++move) {
       newChildren.push_back(new Node((*move).finalState, this, this->tree, ((1. - MCTS_EPSILON) * (moveProbabilities[(*move).id] / Normalization2) + MCTS_EPSILON * noises[i]), (*move).id));
       i++;
      }

      for(std::vector<Node*>::iterator child = newChildren.begin(); child != newChildren.end(); ++child) {
       (*child)->updateU();
      }
    
      //And add it to the node
      this->addChildren(newChildren);

      free(noises);
    }
    else {
      double Normalization2 = 0;
      for(std::vector<Move>::iterator move = legalMoves.begin(); move != legalMoves.end(); ++move) {
        Normalization2 += moveProbabilities[(*move).id];
      }

      for(std::vector<Move>::iterator move = legalMoves.begin(); move != legalMoves.end(); ++move) {
       newChildren.push_back(new Node((*move).finalState, this, this->tree, (moveProbabilities[(*move).id] / Normalization2), (*move).id));
      }

      for(std::vector<Node*>::iterator child = newChildren.begin(); child != newChildren.end(); ++child) {
       (*child)->updateU();
      }
    
      //And add it to the node
      this->addChildren(newChildren);
    }
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
  
//GET/SET METHODS
void Node::increaseNumberOfVisits(void) {
  this->n++;
}
  
void Node::increaseNumberOfChildrenVisits(void) {
  this->nc++;
}
  
void Node::updateAction(double v) {
  if(this->parent != NULL) {
    this->parent->setChildrenSorted(false);
  }

  this->W += v;
  this->Q = this->W / this->n;
}

void Node::updateU(void)
{
  Node* parent;
  
  //If the node's parent is a legitimate node
  if(this->parent != NULL)
  {
    //In general, the parent's children won't be sorted anymore once the UCT is computed
    this->parent->setChildrenSorted(false);
    
    //Compute U
    this->U = MCTS_CP * this->p * sqrt((double)this->parent->getNumberOfChildrenVisits() / (1 + this->n));
  }
  else
  {
    throw std::runtime_error("Calculating the U of a node with a NULL parent.");
  }
}

void Node::updateChildrenU(void)
{
  //In general, the children won't be sorted anymore once the U is updated
  this->setChildrenSorted(false);
    
  //Compute U for every child
  for(std::vector<Node*>::iterator child = this->children.begin(); child != this->children.end(); ++child) {
    (*child)->updateU();
  }
}


//TREE
//CONSTRUCTORS
Tree::Tree(Node *root, NeuralNetwork *net) : root(root) {
  this->root->setTree(this);
}
Tree::Tree(GameState *state, NeuralNetwork *net) : root(new Node(state, this)), net(net) {}


//SET/GET METHODS
void Tree::setRoot(Node *root) {
  this->root = root;

  std::vector<Node*> rootChildren = this->root->getChildren();

  double *noises;

  if((noises = (double*)malloc(rootChildren.size() * sizeof(double))) == NULL) {
    std::cout << "Error allocating the memory for the dirichlet noises, program will be arrested.\n";
    exit(EXIT_FAILURE);
  }

  dirichlet(MCTS_ALPHA, rootChildren.size(), noises);

  int i = 0;
  for(std::vector<Node*>::iterator child = rootChildren.begin(); child != rootChildren.end(); ++child) {
    double oldP = (*child)->getP();

    (*child)->setP((1 - MCTS_EPSILON) * oldP + MCTS_EPSILON * noises[i]);
    (*child)->updateU();

    i++;
  }

  free(noises);
}

Node* Tree::getRoot(void) {
  return this->root;
}

void Tree::setNetwork(NeuralNetwork *net) {
  this->net = net;
}

NeuralNetwork* Tree::getNetwork(void) {
  return this->net;
}


void Tree::deleteTree(void) {
  while(this->root->getParent() != NULL) {
    this->setRoot(this->root->getParent());
  }

  this->root->cutBranch();
}
