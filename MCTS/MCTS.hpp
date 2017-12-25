#include <vector>
#include <Game.hpp>

//TREE NODE
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
  
  
  
  
  public:
    //CONSTRUCTORS
    Node(GameState, Node*, std::vector<Node*>);
    Node(Node*, std::vector<Node*>);
    Node(void);
    
  
    //SET GET METHODS
    void setParent(Node* parent);
    Node* getParent(void);
  
    void addChild(Node* child);
    void addChildren(std::vector<Node*> children);
    Node* getRandomChild(void);
    Node* getBestChild(void);
    
    double getUCT(void);  
}
  
    
Node(GameState state, Node* parent, std::vector<Node*> children) : parent(parent), children(children), state(state) {
  this.n = 0;
  this.w = 0;
}


Node::Node(Node* parent, std::vector<Node*> children) : Node(GameState(), parent, children) {}


Node::Node(void) : Node(null, std::vector<Node*>()) {}
 
      
    
    
    
    
   
