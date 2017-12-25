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
    Node(void) {
      parent = null;
      children = std::vector<Node*>(void);
      
      state = GameState(void);
      
      n = 0;
      w = 0;
    }
    
    Node(Node* Parent, std::vector Children) {
      parent = Parent;
      children = Children;
      
      state = GameState();
      
      n = 0;
      w = 0;
    }
    
    Node(GameState State, Node* Parent, std::vector Children) {
      parent = Parent;
      children = Children;
      
      state = State;
      
      n = 0;
      w = 0;
    }
 
      
    
    
    
    
   
