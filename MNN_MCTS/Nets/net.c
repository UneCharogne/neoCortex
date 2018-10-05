#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include "net.h"


// FUNCTIONS

void init_net(NN *net, char *hidden_type, char *output_type, int nlayers, ...) {
  int i;
  int dim[nlayers];
  va_list list;

  // set number of layers
  net->nl = nlayers;
  if(net->nl<2) {
    printf("\nERROR: a network must have at least 2 layers!\n");
    exit(1);
  }
  // set dimension of layers
  va_start(list, nlayers);
  for(i=0; i<nlayers; i++) {
    dim[i] = va_arg(list, int);
  }
  va_end(list);
  // alloc memory for layers
  net->layers = (layer *) malloc(nlayers * sizeof(layer));
  if(net->layers == NULL) {
    printf("\nERROR: Malloc of net failed.\n");
    exit(1);
  }
  // init input layer
  init_layer(&net->layers[0], "input", dim[0], 0);
  // init hidden layers
  for(i=1; i<nlayers-1; i++) {
    init_layer(&net->layers[i], hidden_type, dim[i], dim[i-1]);
  }
  // init output layer
  init_layer(&net->layers[net->nl-1], output_type, dim[net->nl-1], dim[net->nl-2]);
}

void init_layer(layer *l, char *type, int n, int nprev) {
  int i, j;
  // set activation function
  strcpy(l->type, type);
  if(strcmp(type, "tanh")==0) {
    l->activation = tanh_activation;
  }
  else if(strcmp(type, "sigmoid")==0) {
    l->activation = sigmoid_activation;
  }
  else if(strcmp(type, "relu")==0) {
    l->activation = relu_activation;
  }
  else if(strcmp(type, "linear")==0 || strcmp(type, "input")==0) {
    l->activation = id_activation;
  }
  else if(strcmp(type, "softmax")==0) {
    l->activation = softmax_activation;
  }
  else if(strcmp(type, "mcts")==0) {
    l->activation = mcts_activation;
  }
  else {
    printf("\nERROR: activation type [%s] not allowed!\n", type);
    exit(1);
  }
  // set number of units
  l->n = n;
  // set number of units of previous layer
  l->nprev = nprev;
  // alloc array of units
  l->units_lin = (double *) malloc(n * sizeof(double));
  if(strcmp(type, "input")==0 || strcmp(type, "linear")==0) {
    l->units_act = l->units_lin;
  }
  else {
    l->units_act = (double *) malloc(n * sizeof(double));
  }
  // if there is a previous layer
  // then initialize weights and biases
  if(nprev > 0) {
    // alloc memory
    l->biases = (double *) malloc(n * sizeof(double));
    l->weights = (double **) malloc(n * sizeof(double*));
    if(l->biases == NULL || l->weights == NULL) {
      printf("\nERROR: Malloc of net failed.\n");
      exit(1);
    }
    for(i=0; i<n; i++) {
      l->weights[i] = (double *) malloc(nprev * sizeof(double));
      if(l->weights[i] == NULL) {
        printf("\nERROR: Malloc of net failed.\n");
        exit(1);
      }
    }
    // init values
    for(i=0; i<n; i++) {
      // set biases to zero
      l->biases[i] = 0;
      for(j=0; j<nprev; j++) {
        // Xavier initialization of weights
        l->weights[i][j] = 0.1;
      }
    }
  }
}

void free_layer(layer *l) {
  int i;

  free(l->units_lin);
  if(l->nprev>0) {
    if(strcmp(l->type, "linear")!=0) {
      free(l->units_act);
    }
    for(i=0; i<l->n; i++) {
      free(l->weights[i]);
    }
    free(l->weights);
    free(l->biases);
  }
}

void free_net(NN *net) {
  int i;

  for(i=0; i<net->nl; i++) {
    free_layer(&net->layers[i]);
  }
  free(net->layers);
}

void print_network_structure(NN *net) {
  int i;

  printf("\n**********    Neural Network Structure    **********\n");
  if(net->nl>0) {
    printf("Input  layer of dimesion:   %d\n", net->layers[0].n);
  }
  else {
    printf("The network is empty.\n");
  }
  if(net->nl>2) {
    for(i=1; i<net->nl-1; i++) {
      printf("Hidden layer of dimesion:   %d (%s)\n", net->layers[i].n, net->layers[i].type);
    }
  }
  if(net->nl>1) {
    printf("Output layer of dimesion:   %d (%s)\n", net->layers[net->nl-1].n, net->layers[net->nl-1].type);
  }
}

void linear_activation(layer *l, double *units_prev) {
  int i, k;
  double temp;
  // compute linear combinations
  for(k=0; k<l->n; k++){
    temp = l->biases[k];
    for(i=0; i<l->nprev; i++) {
      temp += l->weights[k][i]*units_prev[i];
    }
    l->units_lin[k] = temp;
  }
}

void id_activation(double *units_lin, double *units_act, int n) {
  return;
}

void tanh_activation(double *units_lin, double *units_act, int n) {
  int i;

  for(i=0; i<n; i++) {
    units_act[i] = tanh(units_lin[i]);
  }
}

void sigmoid_activation(double *units_lin, double *units_act, int n) {
  int i;

  for(i=0; i<n; i++) {
    units_act[i] = 1.0/(1.0+exp(-units_lin[i]));
  }
}

void relu_activation(double *units_lin, double *units_act, int n) {
  int i;

  for(i=0; i<n; i++) {
    units_act[i] = units_lin[i]*(units_lin[i]>0.0);
  }
}

void softmax_activation(double *units_lin, double *units_act, int n) {
  int i;
  double norm;

  norm = 0.0;
  for(i=0; i<n; i++) {
    units_act[i] = exp(units_lin[i]);
    norm += units_act[i];
  }
  for(i=0; i<n; i++) {
    units_act[i] /=norm;
  }
}

void mcts_activation(double *units_lin, double *units_act, int n) {
  int i;
  double norm;

  // softmax avrivation for n-1 elements
  norm = 0.0;
  for(i=0; i<n-1; i++) {
    units_act[i] = exp(units_lin[i]);
    norm += units_act[i];
  }
  for(i=0; i<n-1; i++) {
    units_act[i] /=norm;
  }
  // tanh activation for the last element
  units_act[n-1] = tanh(units_lin[n-1]);
}

void forward_propagation(NN *net, double *input_vector) {
  int i;
  // set units in input layer
  for(i=0; i<net->layers[0].n; i++) {
    net->layers[0].units_lin[i] = input_vector[i];
  }
  // forward propagation trough other layers
  for(i=1; i<net->nl; i++) {
    linear_activation(&net->layers[i], net->layers[i-1].units_act);
    net->layers[i].activation(net->layers[i].units_lin, net->layers[i].units_act, net->layers[i].n);
  }
}

void predict(NN *net, double *vector, double *output) {
  int i, nout;

  forward_propagation(net, vector);
  nout = net->layers[net->nl-1].n;
  for(i=0; i<nout; i++) {
    output[i] = net->layers[net->nl-1].units_act[i];
  }
}

void load_net(NN *net, char *file_name) {
  int *dim;
  int nl;
  int n, i, j;
  char **types;
  double temp;
  FILE *in;

  // open file
  if((in = fopen(file_name,"r")) == NULL) {
    printf("\nERROR while opening file [%s]\n", file_name);
    exit(0);
  }
  // read number of layers
  fscanf(in, "%d", &nl);
  net->nl = nl;
  //printf("%d\n", net->nl);
  if(net->nl<2) {
    printf("\nERROR: network must have at least 2 layers!\n");
    exit(1);
  }
  // alloc memory for dimensions of layers
  dim = (int *) malloc(nl * sizeof(int));
  if(dim == NULL) {
    printf("\nERROR: Malloc of net failed.\n");
    exit(1);
  }
  // read dimension of layers
  for(i=0; i<nl; i++) {
    fscanf(in, "%d", &dim[i]);
  }
  // alloc memory for types of layers
  types = (char **) malloc(nl * sizeof(char*));
  if(types == NULL) {
    printf("\nERROR: Malloc of net failed.\n");
    exit(1);
  }
  for(i=0; i<nl; i++) {
    types[i] = (char *) malloc(10 * sizeof(char));
    if(types[i] == NULL) {
      printf("\nERROR: Malloc of net failed.\n");
      exit(1);
    }
  }
  // read types of layers
  for(i=0; i<nl; i++) {
    fscanf(in, "%s", types[i]);
    //printf("%s ", types[i]);
  }
  //printf("\n");
  // alloc memory for layers
  net->layers = (layer *) malloc(nl * sizeof(layer));
  if(net->layers == NULL) {
    printf("\nERROR: Malloc of net failed.\n");
    exit(1);
  }
  // init input layer
  init_layer(&net->layers[0], "input", dim[0], 0);
  // init hidden and output layers
  for(i=1; i<nl; i++) {
    init_layer(&net->layers[i], types[i], dim[i], dim[i-1]);
  }
  // READ AND SET WEIGHTS AND BIASES
  // read biases
  for(n=1; n<nl; n++) {
    for(i=0; i<net->layers[n].n; i++) {
      fscanf(in, "%lg", &temp);
      net->layers[n].biases[i] = temp;
      //printf("%lg ", net->layers[n].biases[i]);
    }
    //printf("\n");
  }
  // read weights
  for(n=1; n<nl; n++) {
    for(i=0; i<net->layers[n].n; i++) {
      for(j=0; j<net->layers[n].nprev; j++) {
        fscanf(in, "%lg", &temp);
        net->layers[n].weights[i][j] = temp;
        //printf("%lg ", net->layers[n].weights[i][j]);
      }
      //printf("\n");
    }
  }
  for(i=0; i<nl; i++) {
    free(types[i]);
  }
  free(types);
  free(dim);
  fclose(in);
}
