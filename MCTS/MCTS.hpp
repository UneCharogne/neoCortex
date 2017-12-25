#include <vector>
#include <random>
#include <stdexcept>
#include <Game.hpp>






//TREE'S NODE
class Node {
  private:
    //TREE TOPOLOGY
    //Pointer to parent node
    Node *parent;
    //Vector of pointers to child nodes
    std::vector<Node*> children;
    
    
    //GAME STATE
    //Game state associated to the node
    GameState state;
    
    
    //UCT
    //Number of visits of the node
    int n;
    //Number of wins starting from the node
    int w;
    //UCT value
    double UCT;
  
  
  
  
  public:
    //CONSTRUCTORS
    Node(GameState, Node*, std::vector<Node*>);
    Node(Node*, std::vector<Node*>);
    Node(void);
    
  
    //SET GET METHODS
    void setParent(Node*);
    Node* getParent(void);
  
    void addChild(Node* child);
    void addChildren(std::vector<Node*>);
    Node* getRandomChild(void);
    Node* getBestChild(void);
  
    int getNumberOfVisits(void);
    
    double getUCT(void); 
  
  
    //UCT
    void updateUCT(void);
}
  
    
Node(GameState state, Node* parent, std::vector<Node*> children) : parent(parent), children(children), state(state) {
  this.n = 0;
  this.w = 0;
  this.UCT = 0.f;
}


Node::Node(Node* parent, std::vector<Node*> children) : Node(GameState(), parent, children) {}


Node::Node(void) : Node(null, std::vector<Node*>()) {}


void Node::setParent(Node* parent) {
  this.parent = parent;
}


Node* Node::getParent(void) {
  return this.parent;
}


void Node::addChild(Node *newChild) {
  this.children.push_back(newChild);
}


void Node::addChildren(std::vector<Node*> newChildren) {
  this.children.insert(this.children.end(), newChildren.begin(), newChildren.end());
}


Node* Node::getRandomChild(void) {
  if(this.children.size() == 0)
  {
    throw std::runtime_error("Trying to get a child from a node with no children.");
    return NULL;
  }
  
  return this.children[uniform_int_distribution(0,(this.children.size()-1))];
}
 
      
    
    
    
    
   
