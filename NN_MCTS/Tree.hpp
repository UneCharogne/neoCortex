/*
    Tree.hpp:
        Library for the definition of the tree class.
        The tree class simply contain a root node (the node class being also defined in this library) and some set/get and cleaning routine. Also, it contains the 
        network used for the move evaluation.
        The node class contains a pointer to the parent node, a vector of pointers to the children nodes, and a pointer to the tree it belongs to.
        It also contains a pointer to the game state, and the values necessary to calculate the UCT. It also has methods necessary for the MCTS.

        @author: Massimiliano Chiappini 
        @contact: massimilianochiappini@gmail.com
        @version: 0.2
*/



#ifndef TREE_HPP
#define TREE_HPP

#include <vector>
#include "Game.hpp"
#include "brian.hpp"




//Forward declarations
class Tree;



//TREE'S NODE
class Node {
  private:
    //TREE TOPOLOGY
    //Pointer to the tree it belongs
    Tree *tree;
    //Pointer to parent node
    Node *parent;
    //Vector of pointers to child nodes
    std::vector<Node*> children;
    //Flag for the order of the children
    bool childrenSorted;
    
    
    //GAME STATE
    //Game state associated to the node
    GameState *state;
    //Id of the move that led to this state
    int id;
    
    
    //Number of visits of the node
    int n;
    //Number of visits to the children from this state
    int nc;
    //Total action value
    double W;
    //Mean action value
    double Q;
    //Probability to move to this state given by the network
    double p;
    //Upper bound confidence
    double U;
  
  
  
  
  public:
    //CONSTRUCTORS
    Node(GameState*, Node*, Tree*, double, int);
    Node(GameState*, Node*, Tree*, double);
    Node(GameState*, Node*, Tree*);
    Node(GameState*, Node*);
    Node(GameState*, Tree*);
    Node(GameState*);
    
  
    //SET GET METHODS
    void setTree(Tree*);
    Tree* getTree(void);

    void setParent(Node*);
    Node* getParent(void);

    int getId(void);
  
    void addChild(Node* child);
    void addChildren(std::vector<Node*>);
    std::vector<Node*> getChildren();
    Node* getRandomChild(void);
    Node* getBestChild(void);
    Node* getChildToPlay(void);
    Node* getChildByState(GameState*);
  
    void setChildrenSorted(bool);
    bool areChildrenSorted(void);
  
    GameState* getState(void);
    int getPlayer(void);
  
    int getNumberOfVisits(void);
    int getNumberOfChildrenVisits(void);
  
    double getTotalAction(void);
    double getMeanAction(void);

    double getU(void); 

    double getPlayProbability(void);
  
    int getNumberOfChildren(void);
  
  
    //MCTS
    bool isLeaf(void);

    void printNetworkDataset(void);

    void cutBranch(void);
    void pruneOtherBranches(Node*);

    void buildChildren(void);
    void sortChildren(void);

    void increaseNumberOfVisits(void);
    void increaseNumberOfChildrenVisits(void);
    void updateAction(double);
    void updateU(void);
};








//TREE
class Tree {
 private:
  Node* root;
  NeuralNetwork* net;
  
  
 public:
  //CONSTRUCTORS
  Tree(Node*, NeuralNetwork *net);
  Tree(GameState*, NeuralNetwork *net);
  
  //SET/GET 
  void setRoot(Node*);
  Node* getRoot(void);

  void setNetwork(NeuralNetwork*);
  NeuralNetwork* getNetwork(void);

  void deleteTree(void);
};
  

#endif

  

  
    
      
    
    
    
    
   
