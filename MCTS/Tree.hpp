#include <vector>
#include "Game.hpp"
#include "MCTS.hpp"


#ifndef TREE_HPP
#define TREE_HPP



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
    Node* getChildByState(GameState);
  
    void setChildrenSorted(bool);
    bool areChildrenSorted(void);
  
    GameState getState(void);
  
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








//TREE
class Tree {
 private:
  Node* root;
  
  
 public:
  //CONSTRUCTORS
  Tree(Node*);
  Tree(GameState state);
  Tree(void);
  
  //SET/GET NODE
  void setRoot(Node*);
  Node* getRoot(void);
}
  

#endif

  

  
    
      
    
    
    
    
   
