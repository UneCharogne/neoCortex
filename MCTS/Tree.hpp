/*
    Tree.hpp:
        Library for the definition of the tree class.
        The tree class simply contain a root node (the node class being also defined in this library) and some set/get and cleaning routine.
        The node class contains a pointer to the parent node, and a vector of pointers to the children nodes.
        It also contains a pointer to the game state, and the values necessary to calculate the UCT. It also has methods necessary for the MCTS.

        @author: Massimiliano Chiappini 
        @contact: massimilianochiappini@gmail.com
        @version: 0.2
*/



#ifndef TREE_HPP
#define TREE_HPP

#include <vector>
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
    GameState *state;
    
    
    //UCT
    //Number of visits of the node
    int n;
    //Total reward starting from the node for the player of the node
    double reward;
    //UCT value for the player of the parent node
    double UCT;
  
  
  
  
  public:
    //CONSTRUCTORS
    Node(GameState*, Node*);
    Node(GameState*);
    
  
    //SET GET METHODS
    void setParent(Node*);
    Node* getParent(void);
  
    void addChild(Node* child);
    void addChildren(std::vector<Node*>);
    std::vector<Node*> getChildren();
    Node* getRandomChild(void);
    Node* getBestChild(void);
    Node* getChildWithHighestReward(void);
    Node* getChildByState(GameState*);
  
    void setChildrenSorted(bool);
    bool areChildrenSorted(void);
  
    GameState* getState(void);
    int getPlayer(void);
  
    void increaseNumberOfVisits(void);
    int getNumberOfVisits(void);
  
    void increaseReward(double);
    double getReward(void);
  
    int getNumberOfChildren(void);
    
    double getUCT(void); 
  
  
    //MCTS
    bool isLeaf(void);
    void cutBranch(void);
    void pruneOtherBranches(Node*);

    void buildChildren(void);
    void sortChildren(void);
  
    void updateUCT(void);
};








//TREE
class Tree {
 private:
  Node* root;
  
  
 public:
  //CONSTRUCTORS
  Tree(Node*);
  Tree(GameState*);
  
  //SET/GET NODE
  void setRoot(Node*);
  Node* getRoot(void);

  void deleteTree(void);
};
  

#endif

  

  
    
      
    
    
    
    
   
