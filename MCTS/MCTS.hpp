#include <vector>
#include <random>
#include <stdexcept>
#include <cmath>
#include <cfloat>
#include "Game.hpp"
#include "MCTS.hpp"



class MCTS {
  private:
    Tree tree; //When we initialize, we also need to add the first layer of children
    Node* currentNode;
  
  
  /*EXPECTED METHODS
  playMove(state)
  //Force a move from outside (as an example, played by the opponent). This may be tricky. For instance, when we do this, let's be sure that the following layer of children has been generated, otherwise do that (unless it is a final state)
  state playBestMove();
  //Just pick the child with the highest UCT from the current tree's root
  
  MCTSsweep
  
  Selection
  //In the selection, we get down picking time by time the highest UCT, until we reach a leaf
  Expansion
  //We are now in a leaf. If this leaf has never been visited before, do nothing. Else, if it is a final state, do nothing. Else, expand the tree adding a list of children.
  Simulation
  //Do a simulation from the leaf we are in
  BackPropagation
  //Backpropagate the reward
  */
}
