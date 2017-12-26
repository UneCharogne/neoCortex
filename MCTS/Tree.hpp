#include <vector>
#include <random>
#include <stdexcept>
#include "Game.hpp"






//TREE'S NODE
class Node {
  private:
    //TREE TOPOLOGY
    //Pointer to parent node
    Node *parent;
    //Vector of pointers to child nodes
    std::vector<Node*> children;
    //Flag for the order of the children
    bool childrenSorted;
    
    
    //GAME STATE
    //Game state associated to the node
    GameState state;
    
    
    //UCT
    //Number of visits of the node
    int n;
    //Total reward starting from the node
    double reward;
    //UCT value
    double UCT;
  
  
  
  
  public:
    //CONSTRUCTORS
    Node(GameState, Node*);
    Node(GameState);
    Node(void);
    
  
    //SET GET METHODS
    void setParent(Node*);
    Node* getParent(void);
  
    void addChild(Node* child);
    void addChildren(std::vector<Node*>);
    Node* getRandomChild(void);
    Node* getBestChild(void);
  
    void setChildrenSorted(bool);
    bool areChildrenSorted(void);
  
    void increaseNumberOfVisits(void);
    int getNumberOfVisits(void);
  
    void increaseReward(double);
  
    int getNumberOfChildren(void);
    
    double getUCT(void); 
  
  
    //MCTS
    bool isLeaf(void);
    void buildChildren(void);
    void sortChildren(void);
  
    void updateUCT(void);
}


//Ordering criterium based on UCT
struct CompareNodes {
    bool operator()(const Node* &a, const Node* &b) const {
         return a->getUCT() < b->getUCT();
    }
};
  
   
//CONSTRUCTORS
Node::Node(GameState state, Node* parent)  : parent(parent), state(state) {
  this.n = 0;
  this.w = 0;
  this.UCT = 0.f;
}

Node::Node(GameState state) : Node(state, (Node*)null) {}

Node::Node(void) : Node(GameState()) {}


//SET/GET METHODS
void Node::setParent(Node* parent) {
  this.parent = parent;
}

Node* Node::getParent(void) {
  return this.parent;
}

void Node::addChild(Node *newChild) {
  this.children.push_back(newChild);
  
  //Sort the children on ascending value of UCT
  this.sortChildren();
}

void Node::addChildren(std::vector<Node*> newChildren) {
  this.children.insert(this.children.end(), newChildren.begin(), newChildren.end());
  
  //Sort the children on ascending value of UCT
  this.sortChildren();
}

Node* Node::getRandomChild(void) {
  if(this.children.size() == 0)
  {
    throw std::runtime_error("Trying to get a child from a node with no children.");
    return NULL;
  }
  
  return this.children[uniform_int_distribution(0,(this.children.size()-1))];
}

Node* Node::getBestChild(void) {
  if(this.children.size() == 0)
  {
    throw std::runtime_error("Trying to get a child from a node with no children.");
    return NULL;
  }
  
  //If the children are already sorted in ascending order of UCT
  if(this.areChildrenSorted())
  {
    //Return the last child
    return this.children[this.children.size() - 1];
  }
  else
  {
    //Otherwise, sort them and then return the last one
    this.sortChildren();
    return this.children[this.children.size() - 1];
}
  
  
void setChildrenSorted(bool childrenSorted) {
  this.childrenSorted = childrenSorted;
}
  
  
bool areChildrenSorted(void) {
  return this.childrenSorted;
}
  

void increaseNumberOfVisits(void) {
  this.n++;
}


int getNumberOfVisits(void) {
  return this.n;
}
  

void increaseReward(double reward) {
  this.reward += reward;
}


int getNumberOfChildren(void) {
  return this.children.size();
}


double getUCT(void) {
  return this.UCT;
}


bool isLeaf(void) {
  return this.state.isFinalState();
}


void buildChildren(void) {
  std::vector<GameState> legalMoves;
  std::vector<Node*> newChildren;
  
  //Get the legal moves from the current state
  legalMoves = this.state.getLegalMoves();
 
  if(legalMoves.size() != 0)
  {
    //Consequently build the array of children
    for(std::vector<GameState>::iterator state = legalMoves.begin(); state != legalMoves.end(); ++state) {
     newChildren.push_back(new Node(*state, this));
    }
  
    //And add it to the node
    thid.addChildren(newChildren);
  }
  else
  {
    throw std::runtime_error("Trying to build the children of a final state.");
  }
}


void sortChildren(void)
{
  sort(this.children.begin(), this.children.end(), CompareNodes());
  
  this.setChildrenSorted(true);
}
  
  
void updateUCT(void)
{
  Node* parent;
  
  //Get the node's parent
  parent = this.getParent();
  
  //If it is a legitimate node
  if(this.getParent() != null)
  {
    //Check if there are any visit of the node
    if(this.
       
      //Otherwise use DBL MX
       
       
       //Then set that the parent is not sorted anymore
      
    
    
    
    
   
