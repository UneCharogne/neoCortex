#include <vector>
#include <set>
#include <algorithm>
#include <stdexcept>
#include <cmath>
#include <stdlib.h>
#include <cfloat>
#include <iostream>
#include <fstream>
#include "Chess.hpp"
#include "MCTS.hpp"
#include "Tree.hpp"
#include "net.h"


#define DEBUG_MODE 0


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
Node::Node(ChessState *state, Node* parent, Tree *tree, double p, int piece, int startingSquare, int id)  : parent(parent), state(state), tree(tree), p(p), piece(piece), startingSquare(startingSquare), id(id) {
  this->n = 0;
  this->nc = 0;
  this->W = 0;
  this->Q = 0;
}

Node::Node(ChessState *state, Node* parent, Tree *tree, double p)  : parent(parent), state(state), tree(tree), p(p) {
  this->piece = -1;
  this->startingSquare = -1;
  this->id = -1;
  this->n = 0;
  this->nc = 0;
  this->W = 0;
  this->Q = 0;
}

Node::Node(ChessState *state, Node* parent, Tree *tree)  : parent(parent), state(state), tree(tree) {
  this->piece = -1;
  this->startingSquare = -1;
  this->id = -1;
  this->n = 0;
  this->nc = 0;
  this->W = 0;
  this->Q = 0;
  this->p = 0;
}

Node::Node(ChessState *state, Node* parent)  : parent(parent), state(state) {
  this->piece = -1;
  this->startingSquare = -1;
  this->id = -1;
  this->n = 0;
  this->nc = 0;
  this->W = 0;
  this->Q = 0;
  this->p = 0;
}

Node::Node(ChessState *state, Tree *tree) : Node(state, (Node*)NULL, tree) {}

Node::Node(ChessState *state) : Node(state, (Node*)NULL, (Tree*)NULL) {}


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

int Node::getPiece(void) {
  return this->piece;
}

int Node::getStartingSquare(void) {
  return this->startingSquare;
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


Node* Node::getChildByState(ChessState *state) {
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


ChessState* Node::getState(void) {
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


//TO OPTIMIZE
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



void Node::printNetworkDatasets(void) {
  std::ofstream pieces_input, pieces_output, pawn_input, pawn_output, rook_input, rook_output, knight_input, knight_output, bishop_input, bishop_output, queen_input, queen_output, king_input, king_output;
  std::set<int> startingPieces;
  std::vector<double> firstNetworkInput;
  std::vector<double> firstNetworkOutput;
  std::unordered_map<int,std::vector<double>> secondNetworkInput;
  std::unordered_map<int,std::vector<double>> secondNetworkOutput;

  //First of all, let's open the files to print the outputs
  pieces_input.open("TrainingSet/pieces_input.dat", std::ios::out | std::ios::app);
  pieces_output.open("TrainingSet/pieces_output.dat", std::ios::out | std::ios::app);
  pawn_input.open("TrainingSet/pawn_input.dat", std::ios::out | std::ios::app);
  pawn_output.open("TrainingSet/pawn_output.dat", std::ios::out | std::ios::app);
  rook_input.open("TrainingSet/rook_input.dat", std::ios::out | std::ios::app);
  rook_output.open("TrainingSet/rook_output.dat", std::ios::out | std::ios::app);
  knight_input.open("TrainingSet/knight_input.dat", std::ios::out | std::ios::app);
  knight_output.open("TrainingSet/knight_output.dat", std::ios::out | std::ios::app);
  bishop_input.open("TrainingSet/bishop_input.dat", std::ios::out | std::ios::app);
  bishop_output.open("TrainingSet/bishop_output.dat", std::ios::out | std::ios::app);
  queen_input.open("TrainingSet/queen_input.dat", std::ios::out | std::ios::app);
  queen_output.open("TrainingSet/queen_output.dat", std::ios::out | std::ios::app);
  king_input.open("TrainingSet/king_input.dat", std::ios::out | std::ios::app);
  king_output.open("TrainingSet/king_output.dat", std::ios::out | std::ios::app);


  //Get the input for the first network from the current state
  firstNetworkInput = this->state->getFirstNetworkInput();
  //And set up the output
  firstNetworkOutput = std::vector<double>(64, 0.);


  //Then, get all the starting squares in the current board
  for(std::vector<Node*>::iterator child = this->children.begin(); child != this->children.end(); ++child) {
     startingPieces.insert((*child)->getStartingSquare());
  }


  //And, for each of the starting pieces
  for(std::set<int>::iterator square0 = startingPieces.begin(); square0 != startingPieces.end(); ++square0) {
  	int piece0;

  	//Get the piece that has to be moved
  	if(this->getPlayer() == 1) {
  		piece0 = PIECES_TYPES[this->state->getBoard()[(*square0)]];
  	} else {
  		piece0 = PIECES_TYPES[this->state->getBoard()[(63-(*square0))]];
  	}

  	//And, consequently, get the input for the net
  	secondNetworkInput[(*square0)] = this->state->getSecondNetworkInput((*square0));
  	//And set the output
  	switch(piece0) {
  		case PAWN:
  			secondNetworkOutput[(*square0)] = std::vector<double>(12, 0.);
  			break;

  		case ROOK:
  			secondNetworkOutput[(*square0)] = std::vector<double>(28, 0.);
  			break;

  		case KNIGHT:
  			secondNetworkOutput[(*square0)] = std::vector<double>(8, 0.);
  			break;

  		case BISHOP:
  			secondNetworkOutput[(*square0)] = std::vector<double>(28, 0.);
  			break;

  		case QUEEN:
  			secondNetworkOutput[(*square0)] = std::vector<double>(56, 0.);
  			break;

  		case KING:
  			secondNetworkOutput[(*square0)] = std::vector<double>(10, 0.);
  			break;
  	}

  	//Then, cycle over all the children in which this piece is moved
  	double Normalization = 0.;
	  for(std::vector<Node*>::iterator child = this->children.begin(); child != this->children.end(); ++child) {
	   if((*child)->getStartingSquare() == (*square0)) {
	   	//Get the probability to play this move
	   	double p = (*child)->getPlayProbability();

	   	//And add it to the corrisponding outputs
	   	Normalization+= p;
	   	firstNetworkOutput[(*square0)] += p;
	   	secondNetworkOutput[(*square0)][(*child)->getId()] += p;
	   }
	  }

    if(Normalization != 0) {
    	//Then normalize the probability
    	for(std::vector<double>::iterator prob = secondNetworkOutput[(*square0)].begin(); prob != secondNetworkOutput[(*square0)].end(); ++prob) {
    		(*prob) /= Normalization;
    	}

    	//And print it
    	switch(piece0) {
      		case PAWN:
    			for(int i=0;i<secondNetworkInput[(*square0)].size();i++) {
    		 	  pawn_input << secondNetworkInput[(*square0)][i] << " ";
    			}
    			pawn_input << "\n";
    			for(int i=0;i<secondNetworkOutput[(*square0)].size();i++) {
    		 	  pawn_output << secondNetworkOutput[(*square0)][i] << " ";
    			}
    			pawn_output << "\n";	
      			break;

      		case ROOK:
    			for(int i=0;i<secondNetworkInput[(*square0)].size();i++) {
    		 	  rook_input << secondNetworkInput[(*square0)][i] << " ";
    			}
    			rook_input << "\n";
    			for(int i=0;i<secondNetworkOutput[(*square0)].size();i++) {
    		 	  rook_output << secondNetworkOutput[(*square0)][i] << " ";
    			}
    			rook_output << "\n";
      			break;

      		case KNIGHT:
    			for(int i=0;i<secondNetworkInput[(*square0)].size();i++) {
    		 	  knight_input << secondNetworkInput[(*square0)][i] << " ";
    			}
    			knight_input << "\n";
    			for(int i=0;i<secondNetworkOutput[(*square0)].size();i++) {
    		 	  knight_output << secondNetworkOutput[(*square0)][i] << " ";
    			}
    			knight_output << "\n";	
      			break;

      		case BISHOP:
    			for(int i=0;i<secondNetworkInput[(*square0)].size();i++) {
    		 	  bishop_input << secondNetworkInput[(*square0)][i] << " ";
    			}
    			bishop_input << "\n";
    			for(int i=0;i<secondNetworkOutput[(*square0)].size();i++) {
    		 	  bishop_output << secondNetworkOutput[(*square0)][i] << " ";
    			}
    			bishop_output << "\n";	
      			break;

      		case QUEEN:
    			for(int i=0;i<secondNetworkInput[(*square0)].size();i++) {
    		 	  queen_input << secondNetworkInput[(*square0)][i] << " ";
    			}
    			queen_input << "\n";
    			for(int i=0;i<secondNetworkOutput[(*square0)].size();i++) {
    		 	  queen_output << secondNetworkOutput[(*square0)][i] << " ";
    			}
    			queen_output << "\n";	
      			break;

      		case KING:
    			for(int i=0;i<secondNetworkInput[(*square0)].size();i++) {
    		 	  king_input << secondNetworkInput[(*square0)][i] << " ";
    			}
    			king_input << "\n";
    			for(int i=0;i<secondNetworkOutput[(*square0)].size();i++) {
    		 	  king_output << secondNetworkOutput[(*square0)][i] << " ";
    			}
    			king_output << "\n";	
      			break;
      	}
      }
  }

  //Then, normalize the piece probabilities
  double Normalization = 0.;
  for(std::vector<double>::iterator prob = firstNetworkOutput.begin(); prob != firstNetworkOutput.end(); ++prob) {
	   Normalization += (*prob);
  }
  for(std::vector<double>::iterator prob = firstNetworkOutput.begin(); prob != firstNetworkOutput.end(); ++prob) {
	   (*prob) /= Normalization;
  }


  //And print it
  for(int i=0;i<firstNetworkInput.size();i++) {
    pieces_input << firstNetworkInput[i] << " ";
  }
  pieces_input << "\n";
  for(int i=0;i<firstNetworkOutput.size();i++) {
    pieces_output << firstNetworkOutput[i] << " ";
  }
  pieces_output << "\n";
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
  std::vector<double> p1;
  std::unordered_map<int,std::vector<double>> p2;
  std::set<int> startingPieces;
  std::vector<ChessMove> legalMoves;
  std::vector<Node*> newChildren;

  //std::cout << "Building the children.\n\n";


  //Get the move probabilities of the various pieces to move from the current state as evaluated by the neural network
  std::vector<double> firstNetworkInput = this->state->getFirstNetworkInput();
  p1 = std::vector<double>(get_output_size(this->tree->getNetwork1()), 0.);
  predict(this->tree->getNetwork1(), &(firstNetworkInput[0]), &(p1[0]));


  //std::cout << "Probabilities of starting pieces:\n";
  //for(int i=0;i<p1.size();i++) {
  //  std::cout << p1[i] << " ";
  //}
  //std::cout << "\n\n";

  //Get the legal moves from the current state
  legalMoves = this->state->getLegalMoves();

  //std::cout << "Found " << legalMoves.size() << " legal moves, starting from the squares:\n";

  //Then, check which are the possible starting pieces of the legal moves
  for(std::vector<ChessMove>::iterator move = legalMoves.begin(); move != legalMoves.end(); ++move) {
     startingPieces.insert((*move).startingSquare);
  }

  //for(std::set<int>::iterator square0 = startingPieces.begin(); square0 != startingPieces.end(); ++square0) {
  //  std::cout << "- " << (*square0) << "\n";
  //}
  //std::cout << "\n\n";

  //And, for each of them, get the probability of the various moves p2
  for(std::set<int>::iterator square0 = startingPieces.begin(); square0 != startingPieces.end(); ++square0) {
  	int piece0; 

  	//Get the piece type that moves
  	if(this->getPlayer() == 1) {
  		piece0 = PIECES_TYPES[this->state->getBoard()[(*square0)]];
  	} else {
  		piece0 = PIECES_TYPES[this->state->getBoard()[(63-(*square0))]];
  	}


    if(p1[(*square0)] > SECOND_NET_TRESHOLD) {
      std::vector<double> secondNetworkInput = this->state->getSecondNetworkInput(((*square0)));
      p2[(*square0)] = std::vector<double>(get_output_size(this->tree->getNetworks2()[piece0]), 0.);
      predict(this->tree->getNetworks2()[piece0], &(secondNetworkInput[0]), &(p2[(*square0)][0]));
    }
    else {
      if(DEBUG_MODE) {
        std::cout << "Treshold not met.\n";
      }
      p2[(*square0)] = std::vector<double>(get_output_size(this->tree->getNetworks2()[piece0]), 0.);
    }

    //std::cout << "Probabilities of the moves of piece " << piece0 << " from square " << (*square0) << ":\n";
    //for(int i=0;i<p2[(*square0)].size();i++) {
    //  std::cout << p2[(*square0)][i] << " ";
    //}
    //std::cout << "\n\n";
  }

  
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

      double Normalization = 0;
      for(std::vector<ChessMove>::iterator move = legalMoves.begin(); move != legalMoves.end(); ++move) {
        Normalization += p1[(*move).startingSquare] * p2[(*move).startingSquare][(*move).id];
        //std::cout << "The move " << (*move).id << " of piece " << (*move).piece << " starting from square " << (*move).startingSquare << " has total probability " << p1[(*move).startingSquare] * p2[(*move).startingSquare][(*move).id] << "\n";
      }
      //std::cout << "\n\n";

      int i = 0;
      for(std::vector<ChessMove>::iterator move = legalMoves.begin(); move != legalMoves.end(); ++move) {
       newChildren.push_back(new Node((*move).finalState, this, this->tree, ((1. - MCTS_EPSILON) * ((p1[(*move).startingSquare] * p2[(*move).startingSquare][(*move).id]) / Normalization) + MCTS_EPSILON * noises[i]), (*move).piece, (*move).startingSquare, (*move).id));
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
      double Normalization = 0;
      for(std::vector<ChessMove>::iterator move = legalMoves.begin(); move != legalMoves.end(); ++move) {
        Normalization += p1[(*move).startingSquare] * p2[(*move).startingSquare][(*move).id];
        //std::cout << "The move " << (*move).id << " of piece " << (*move).piece << " starting from square " << (*move).startingSquare << " has total probability " << p1[(*move).startingSquare] * p2[(*move).startingSquare][(*move).id] << "\n";
      }
      //std::cout << "\n\n";

      for(std::vector<ChessMove>::iterator move = legalMoves.begin(); move != legalMoves.end(); ++move) {
       newChildren.push_back(new Node((*move).finalState, this, this->tree, ((p1[(*move).startingSquare] * p2[(*move).startingSquare][(*move).id]) / Normalization), (*move).piece, (*move).startingSquare, (*move).id));
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
Tree::Tree(Node *root, NN *net1, std::array<NN*, 6> nets2) : root(root), net1(net1), nets2(nets2) {
  this->root->setTree(this);
  if(DEBUG_MODE) {
    std::cout << "The net of piece " << PAWN << " has output of size " << get_output_size(nets2[PAWN]) << "\n";
    std::cout << "The net of piece " << ROOK << " has output of size " << get_output_size(nets2[ROOK]) << "\n";
    std::cout << "The net of piece " << KNIGHT << " has output of size " << get_output_size(nets2[KNIGHT]) << "\n";
    std::cout << "The net of piece " << BISHOP << " has output of size " << get_output_size(nets2[BISHOP]) << "\n";
    std::cout << "The net of piece " << QUEEN << " has output of size " << get_output_size(nets2[QUEEN]) << "\n";
    std::cout << "The net of piece " << KING << " has output of size " << get_output_size(nets2[KING]) << "\n";
  } 
}
Tree::Tree(ChessState *state, NN *net1, std::array<NN*, 6> nets2) : root(new Node(state, this)), net1(net1), nets2(nets2) {
  if(DEBUG_MODE) {
    std::cout << "The net of piece " << PAWN << " has output of size " << get_output_size(nets2[PAWN]) << "\n";
    std::cout << "The net of piece " << ROOK << " has output of size " << get_output_size(nets2[ROOK]) << "\n";
    std::cout << "The net of piece " << KNIGHT << " has output of size " << get_output_size(nets2[KNIGHT]) << "\n";
    std::cout << "The net of piece " << BISHOP << " has output of size " << get_output_size(nets2[BISHOP]) << "\n";
    std::cout << "The net of piece " << QUEEN << " has output of size " << get_output_size(nets2[QUEEN]) << "\n";
    std::cout << "The net of piece " << KING << " has output of size " << get_output_size(nets2[KING]) << "\n";
  } 
}


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

void Tree::setNetwork1(NN *net1) {
  this->net1 = net1;
}

NN* Tree::getNetwork1(void) {
  return this->net1;
}

void Tree::setNetworks2(std::array<NN*, 6> nets2) {
  this->nets2 = nets2;
}

std::array<NN*, 6> Tree::getNetworks2(void) {
  return this->nets2;
}

void Tree::deleteTree(void) {
  while(this->root->getParent() != NULL) {
    this->setRoot(this->root->getParent());
  }

  this->root->cutBranch();
}
