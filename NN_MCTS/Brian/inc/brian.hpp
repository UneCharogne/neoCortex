/*############################### NEURAL NETWORK LIBRARY ###############################*/
/*############################# by Massimiliano Chiappini ##############################*/
/* The class Neuron is defined, representing a single neuron of the network and its     */
/* local topology, namely its connections with other neurons. A connection is made of   */
/* a reference to the connected neuron and the weight of the connection. A neuron also  */
/* contains a bias, and an activation function.                                         */
/* The class Network contains the neurons composing the neural network and the layers   */
/* structure. It also contains methods for the propagation of an input through the      */
/* network, and the return of the output.                                               */
/*                                                                                      */
/* @author: Massimiliano Chiappini                                                      */
/* @contact: massimilianochiappini@gmail.com                                            */
/* @version: 0.1                                                                        */


#ifndef BRAIN_HPP
#define BRAIN_HPP

#include <vector>
#include <string>
#include <math.h>




//Forward declaration
class Neuron;




//Struct representing a connection between two neurons
struct Connection {
  Neuron* neuron;
  double w;

  Connection(Neuron*, double);
};




//Single neuron class
class Neuron {
  private:
    //Bias
    double b;

    double (*activationFunction) (double);

    double state;

	  std::vector<Connection> backwardConnections;
	  std::vector<Connection> forwardConnections;

  public:
    //Constructors
    Neuron(double, std::string);
    Neuron(double, std::string, std::vector<Connection>, std::vector<Connection>);

    //Set the bias
    void setBias(double);

    //Add connection
    void addConnection(std::string, Connection);

    //Activate neuron
    void updateState(void);
    double getState(void);
};




//Neural network class
class NeuralNetwork {
  private:
    std::vector< std::vector<Neuron*> > layers;
    int inputSize;
    int outputSize;

  public:
    //Constructors
    NeuralNetwork(std::string);

    //SET/GET methods
    int getInputSize(void);
    int getOutputSize(void);

    //Input
    void setInput(std::vector<double>);
    //Propagation
    void propagateSignal(void);
    //Output
    std::vector<double> getOutput(void);
};




//ACTIVATION FUNCTIONS
double activation_identity(double);

double activation_tanh(double);

double activation_sigmoid(double);

#endif
