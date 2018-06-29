#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#include "brian.hpp"


//STRUCT CONNECTION
//Constructors
Connection::Connection(Neuron* neuron, double w) {
  this->neuron = neuron;
  this->w = w;
}


//CLASS NEURON
//Constructors
Neuron::Neuron(double bias, std::string type) {
  this->b = bias;

  if (type == "Identity") {
      this->activationFunction = &activation_identity;
  } else if (type == "Tanh") {
      this->activationFunction = &activation_tanh;
  } else if (type == "Sigmoid") {
      this->activationFunction = &activation_sigmoid;
  } else {
    std::cout << "No activation function of type " << type << ", program will be arrested.\n";
    exit(EXIT_FAILURE);
  }

  this->state = 0;
}


Neuron::Neuron(double bias, std::string type, std::vector<Connection> backwardConnections, std::vector<Connection> forwardConnections) {
  this->b = bias;

  if (type == "Identity") {
      this->activationFunction = &activation_identity;
  } else if (type == "Tanh") {
      this->activationFunction = &activation_tanh;
  } else if (type == "Sigmoid") {
      this->activationFunction = &activation_sigmoid;
  } else {
    std::cout << "No activation function of type " << type << ", program will be arrested.\n";
    exit(EXIT_FAILURE);
  }

  this->state = 0.;

  this->backwardConnections = backwardConnections;

  this->forwardConnections = forwardConnections;
}


//Set neuron
void Neuron::setBias(double bias) {
  this->b = bias;
}


void Neuron::addConnection(std::string type, Connection newConnection) {
  if(type == "backward") {
    this->backwardConnections.push_back(newConnection);
  } else if (type == "forward") {
    this->forwardConnections.push_back(newConnection);
  } else {
    std::cout << "No correspondence for connection type " << type << ", program will be arrested.\n";
    exit(EXIT_FAILURE);
  }
}


//Activate neuron
void Neuron::updateState(void) {
  double input;

  //Get the input from the backward connections
  input = this->b;
  for(std::vector<Connection>::iterator connection = this->backwardConnections.begin(); connection != this->backwardConnections.end(); ++connection) {
    input += connection->neuron->getState() * connection->w;
  }

  //And calculate the activation function of the input
  this->state = this->activationFunction(input);
}


double Neuron::getState(void) {
  return this->state;
}




//CLASS NETWORK
//Constructors
NeuralNetwork::NeuralNetwork(std::string fileName) {
  int size;
  char type;
  double b, w;
  int Nl;
  std::vector<int> layerSize;
  std::vector<std::string> layerType;
  std::vector< std::vector<double> > bias;
  std::vector< std::vector< std::vector<double> > > weight;

  FILE *fp;

  if((fp = fopen(fileName.c_str(), "r")) == NULL) {
    std::cout << "Error opening the file \"" << fileName << "\", program will be arrested.\n";
    exit(EXIT_FAILURE);
  }


  //First of all, read the number of layers
  fscanf(fp, "%*s %d\n", &Nl);
  //Which has to be at least equal to 2
  if(Nl < 2) {
    std::cout << "Error, number of layers smaller than 2. Program will be arrested.\n";
    exit(EXIT_FAILURE);
  }

  //Then, read the layer size and types
  for(int l=0;l<Nl;l++) {
    fscanf(fp, "%d ", &size);
    layerSize.push_back(size);
  }
  this->inputSize = layerSize[0];
  this->outputSize = layerSize[Nl - 1];

  fscanf(fp, "\n");
  for(int l=0;l<Nl;l++) {
    fscanf(fp, "%c ", &type);
    if ((type == 'I') || (type == 'M')) {
      layerType.push_back("Identity");
    } else if (type == 'T') {
      layerType.push_back("Tanh");
    } else if (type == 'S') {
      layerType.push_back("Sigmoid");
    } else {
      std::cout << "Layer type \"" << type << " not known, program will be arrested.\n";
      exit(EXIT_FAILURE);
    }
  }
  fscanf(fp, "\n");
  //And the first layer should always be of type identity
  if(layerType[0] != "Identity") {
    std::cout << "The input layer should always be of ype identity. Program will be arrested.\n";
    exit(EXIT_FAILURE);
  }

  //Then, read the biases (not of the first layer, which has trivial biases
  for(int l=1;l<Nl;l++)
  {
    bias.push_back( std::vector<double>() );
    for(int i=0;i<layerSize[l];i++) {
      fscanf(fp, "%lf ", &b);
      bias[(l-1)].push_back(b);
    }
    fscanf(fp, "\n");
  }

  //Then, read the weights
  for(int l=1;l<Nl;l++)
  {
    weight.push_back( std::vector< std::vector<double> >() );

    for(int i=0;i<layerSize[l];i++) {
      weight[(l-1)].push_back( std::vector<double>() );
      for(int j=0;j<layerSize[(l-1)];j++) {
        fscanf(fp, "%lf ", &w);
        weight[(l-1)][i].push_back(w);
      }
      fscanf(fp, "\n");
    }
  }


  //Now that we have read all the data, we can build the network
  //First of all, we can add the first layer, which is specific (always with identity activation function, unimportant bias, and no backward connections
  this->layers.push_back( std::vector<Neuron*>() );
  for(int i=0;i<layerSize[0];i++) {
    layers[0].push_back(new Neuron(0, "Identity"));
  }

  //Then, we can cycle over all the other layers
  for(int l=1;l<Nl;l++)
  {
    this->layers.push_back( std::vector<Neuron*>() );

    for(int i=0;i<layerSize[l];i++){
      //Build the layer
      this->layers[l].push_back(new Neuron(bias[(l-1)][i], layerType[l]));

      //And its backward connections
      for(int j=0;j<layerSize[(l-1)];j++) {
        this->layers[l][i]->addConnection("backward", Connection(this->layers[(l-1)][j], weight[(l-1)][i][j]));
        this->layers[(l-1)][j]->addConnection("forward", Connection(this->layers[l][i], weight[(l-1)][i][j]));
      }
    }
  }
}


//SET/GET methods
int NeuralNetwork::getInputSize(void) {
  return this->inputSize;
}

int NeuralNetwork::getOutputSize(void) {
  return this->outputSize;
}


//Input
void NeuralNetwork::setInput(std::vector<double> input){
  if(input.size() != this->layers.begin()->size()) {
    std::cout << "The size of the input for the network is wrong, program will be arrested.\n";
    exit(EXIT_FAILURE);
  }

  for(int i=0;i<input.size();i++) {
    this->layers[0][i]->setBias(input[i]);
  }
}


//Propagation
void NeuralNetwork::propagateSignal(void) {
  //For each layer
  for(std::vector< std::vector<Neuron*> >::iterator layer = this->layers.begin(); layer != this->layers.end(); ++layer) {
    //Update each neuron in the layer
    for(std::vector<Neuron*>::iterator neuron = layer->begin(); neuron != layer->end(); ++neuron) {
      //Update each neuron in the layer
      (*neuron)->updateState();
    }
  }
}


//Output
std::vector<double> NeuralNetwork::getOutput(void) {
  std::vector<double> output;

  //For each neuron in the final layer
  for(std::vector<Neuron*>::iterator neuron = this->layers[(this->layers.size() - 1)].begin(); neuron != this->layers[(this->layers.size() - 1)].end(); ++neuron) {
    //Get the value
    output.push_back((*neuron)->getState());
  }

  return output;
}




//ACTIVATION FUNCTIONS
double activation_identity(double x) {
  return x;
}

double activation_tanh(double x) {
  return tanh(x);
}

double activation_sigmoid(double x) {
  return (1. / (1. + exp(-1. * x)));
}

