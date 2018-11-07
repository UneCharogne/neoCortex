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
    l->derivative = tanh_derivative;
    l->loss = square_loss;
  }
  else if(strcmp(type, "sigmoid")==0) {
    l->activation = sigmoid_activation;
    l->derivative = sigmoid_derivative;
    l->loss = square_loss;
  }
  else if(strcmp(type, "relu")==0) {
    l->activation = relu_activation;
    l->derivative = relu_derivative;
    l->loss = square_loss;
  }
  else if(strcmp(type, "linear")==0 || strcmp(type, "input")==0) {
    l->activation = id_activation;
    l->derivative = id_derivative;
    l->loss = square_loss;
  }
  else if(strcmp(type, "softmax")==0) {
    l->activation = softmax_activation;
    l->derivative = id_derivative;
    l->loss = cross_entropy_loss;
  }
  else if(strcmp(type, "mcts")==0) {
    l->activation = mcts_activation;
    l->derivative = mcts_derivative;
    l->loss = mcts_loss;
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
        l->weights[i][j] = ran_gauss(0.0, sqrt(2.0/(l->n+l->nprev)));
        //l->weights[i][j] = 0.01;
      }
    }
  }
}

void init_gradients(layer *l) {
  int i, j;

  // alloc memory
  l->deltas = (double *) malloc(l->n * sizeof(double));
  l->delta_biases = (double *) malloc(l->n * sizeof(double));
  l->delta_weights = (double **) malloc(l->n * sizeof(double*));
  l->grad_biases = (double *) malloc(l->n * sizeof(double));
  l->grad_weights = (double **) malloc(l->n * sizeof(double*));
  if(l->deltas == NULL || l->delta_biases == NULL || l->delta_weights == NULL ||
    l->grad_biases == NULL || l->grad_weights == NULL) {
    printf("\nERROR: Malloc of net failed.\n");
    exit(1);
  }
  for(i=0; i<l->n; i++) {
    l->grad_weights[i] = (double *) malloc(l->nprev * sizeof(double));
    if(l->grad_weights[i] == NULL) {
      printf("\nERROR: Malloc of net failed.\n");
      exit(1);
    }
    l->delta_weights[i] = (double *) malloc(l->nprev * sizeof(double));
    if(l->delta_weights[i] == NULL) {
      printf("\nERROR: Malloc of net failed.\n");
      exit(1);
    }
  }
  // initialize
  for(i=0; i<l->n; i++) {
    l->deltas[i] = 0.0;
    l->delta_biases[i] = 0.0;
    l->grad_biases[i] = 0.0;
    for(j=0; j<l->nprev; j++) {
      l->delta_weights[i][j] = 0.0;
      l->grad_weights[i][j] = 0.0;
    }
  }
}

void init_training(NN *net) {
  int i;

  for(i=1; i<net->nl; i++) {
    init_gradients(&net->layers[i]);
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
      if(l->grad_weights != NULL) {
        free(l->grad_weights[i]);
      }
      if(l->delta_weights != NULL) {
        free(l->delta_weights[i]);
      }
    }
    free(l->weights);
    free(l->biases);
    free(l->grad_weights);
    free(l->delta_weights);
    free(l->grad_biases);
    free(l->delta_biases);
    free(l->deltas);
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

int get_output_size(NN *net) {
  return net->layers[net->nl-1].n;
}

int get_input_size(NN *net) {
  return net->layers[0].n;
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

void id_derivative(double *units_lin, double *units_act, double *fprime, int n) {
  int i;

  for(i=0; i<n; i++) {
    fprime[i] = 1;
  }
}

void tanh_activation(double *units_lin, double *units_act, int n) {
  int i;

  for(i=0; i<n; i++) {
    units_act[i] = tanh(units_lin[i]);
  }
}

void tanh_derivative(double *units_lin, double *units_act, double *fprime, int n) {
  int i;

  for(i=0; i<n; i++) {
    fprime[i] = 1.0-units_act[i]*units_act[i];
  }
}

void sigmoid_activation(double *units_lin, double *units_act, int n) {
  int i;

  for(i=0; i<n; i++) {
    units_act[i] = 1.0/(1.0+exp(-units_lin[i]));
  }
}

void sigmoid_derivative(double *units_lin, double *units_act, double *fprime, int n) {
  int i;

  for(i=0; i<n; i++) {
    fprime[i] = units_act[i]*(1.0-units_act[i]);
  }
}

void relu_activation(double *units_lin, double *units_act, int n) {
  int i;

  for(i=0; i<n; i++) {
    units_act[i] = units_lin[i]*(units_lin[i]>0.0);
  }
}

void relu_derivative(double *units_lin, double *units_act, double *fprime, int n) {
  int i;

  for(i=0; i<n; i++) {
    fprime[i] = 1.0*(units_lin[i]>0);
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

  // softmax activation for n-1 elements
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

void mcts_derivative(double *units_lin, double *units_act, double *fprime, int n) {
  int i;

  // id derivative for n-1 elements
  for(i=0; i<n-1; i++) {
    fprime[i] = 1;
  }
  // tanh derivative for last element
  fprime[n-1] = 1.0-units_act[n-1]*units_act[n-1];
}

double square_loss(double *units_act, double *target, int n) {
  double loss = 0.0;
  int i;

  for(i=0; i<n; i++) {
    loss += 0.5*(units_act[i]-target[i])*(units_act[i]-target[i]);
  }
  return loss;
}

double cross_entropy_loss(double *units_act, double *target, int n) {
  double loss = 0.0;
  int i;

  for(i=0; i<n; i++) {
    loss += -target[i]*log(units_act[i]);
  }
  return loss;
}

double mcts_loss(double *units_act, double *target, int n) {
  double loss = 0.0;
  int i;

  // cross-entropy for n-1 elements
  for(i=0; i<n-1; i++) {
    loss += -target[i]*log(units_act[i]);
  }
  // square for last element
  loss += 0.5*(units_act[n-1]-target[n-1])*(units_act[n-1]-target[n-1]);
  return loss;
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

void delta(layer *l, layer *lnext) {
  int i, j, k;
  double fprime[l->n];

  // compute derivatives
  l->derivative(l->units_lin, l->units_act, fprime, l->n);
  // compute deltas
  for(i=0; i<l->n; i++) {
    l->deltas[i] = 0.0;
    for(j=0; j<lnext->n; j++) {
      l->deltas[i] += lnext->deltas[j]*lnext->weights[j][i];
    }
    l->deltas[i] *= fprime[i];
  }
}

void delta_out(layer *l, double *target) {
  int i;
  double fprime[l->n];

  // compute derivatives
  l->derivative(l->units_lin, l->units_act, fprime, l->n);
  // compute deltas
  for(i=0; i<l->n; i++) {
    l->deltas[i] = fprime[i]*(l->units_act[i]-target[i]);
  }
}

void update_gradients(layer *l, layer *lprev) {
  int i, j;

  for(i=0; i<l->n; i++) {
    // biases' gradients
    l->grad_biases[i] += l->deltas[i];
    for(j=0; j<l->nprev; j++) {
      // weights' gradients
      l->grad_weights[i][j] += l->deltas[i]*lprev->units_act[j];
    }
  }
}

void reset_gradients(layer *l) {
  int i, j;

  for(i=0; i<l->n; i++) {
    l->grad_biases[i] = 0.0;
    for(j=0; j<l->nprev; j++) {
      l->grad_weights[i][j] = 0.0;
    }
  }
}

void gradient_descent(layer *l, double rate, double momentum, double weight_decay, int batchsize) {
  int i, j;

  // update weights and biases
  for(i=0; i<l->n; i++) {
    l->delta_biases[i] = momentum*l->delta_biases[i]-rate*(l->grad_biases[i]/batchsize);
    l->biases[i] += l->delta_biases[i];
    for(j=0; j<l->nprev; j++) {
      l->delta_weights[i][j] = momentum*l->delta_weights[i][j]
      -rate*(l->grad_weights[i][j]/batchsize+weight_decay*l->weights[i][j]);
      l->weights[i][j] += l->delta_weights[i][j];
    }
  }
  // set gradients to zero
  reset_gradients(l);
}

void back_propagation(NN *net, double *target) {
  int i;
  // output layer
  delta_out(&net->layers[net->nl-1], target);
  update_gradients(&net->layers[net->nl-1], &net->layers[net->nl-2]);
  // hidden layers (if present)
  for(i=net->nl-2; i>0; i--) {
    delta(&net->layers[i], &net->layers[i+1]);
    update_gradients(&net->layers[i], &net->layers[i-1]);
  }
}

void sgd_update(NN *net, double rate, double momentum, double weight_decay, int batchsize) {
  int i;

  for(i=1; i<net->nl; i++) {
    gradient_descent(&net->layers[i], rate, momentum, weight_decay, batchsize);
  }
}

void train(NN *net, int ntrain, double **dataset, double **target, double rate, double momentum, double weight_decay, int batchsize, int Niterations) {
  int ninput, noutput;
  int nbatches, batchsize_last;
  int *index;
  int i, j, k, n;
  double loss;

  noutput = get_output_size(net);
  ninput = get_input_size(net);
  // determine number of batches
  nbatches = ntrain/batchsize;
  batchsize_last = ntrain%batchsize;
  // alloc memory for indices
  index = (int *) malloc(ntrain * sizeof(int));
  if(index == NULL) {
    printf("\nERROR: Malloc of index failed.\n");
    exit(1);
  }
  for(n=0; n<Niterations; n++) {
    // random indices
    random_indices(index, ntrain);
    // loop over batches
    k = 0;
    for(i=0; i<nbatches; i++) {
      for(j=0; j<batchsize; j++) {
        // produce output
        //index[k] = k;
        forward_propagation(net, dataset[index[k]]);
        // back-propagate error
        back_propagation(net, target[index[k]]);
        k++;
      }
      // update
      sgd_update(net, rate, momentum, weight_decay, batchsize);
    }
    // last batch (if present)
    if(batchsize_last>0) {
      for(j=0; j<batchsize_last; j++) {
        // produce output
        forward_propagation(net, dataset[index[k]]);
        // back-propagate error
        back_propagation(net, target[index[k]]);
        k++;
      }
      // update
      sgd_update(net, rate, momentum, weight_decay, batchsize_last);
    }
    // evaluate loss
    loss = evaluate_loss(net, dataset, target, ntrain);
    printf("Iteration %d,  loss = %lg\n", n+1, loss);
  }
}

double evaluate_loss(NN *net, double **dataset, double **target, int ndata) {
  int i;
  double error = 0.0;

  for(i=0; i<ndata; i++) {
    // produce output
    forward_propagation(net, dataset[i]);
    // compute error
    error += net->layers[net->nl-1].loss(net->layers[net->nl-1].units_act, target[i], net->layers[net->nl-1].n);
  }
  return error/ndata;
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

void save_net(NN *net, char *file_name) {
  int n, i, j;
  FILE *out;

  // open file
  if((out = fopen(file_name,"w")) == NULL) {
    printf("\nERROR while opening file [%s]\n", file_name);
    exit(0);
  }
  // write number of layers
  fprintf(out, "%d\n", net->nl);
  // write dimension of layers
  for(n=0; n<net->nl; n++) {
    fprintf(out, "%d ", net->layers[n].n);
  }
  fprintf(out, "\n");
  // write types of layers
  for(n=0; n<net->nl; n++) {
    fprintf(out, "%s ", net->layers[n].type);
  }
  fprintf(out, "\n");
  // write biases
  for(n=1; n<net->nl; n++) {
    for(i=0; i<net->layers[n].n; i++) {
      fprintf(out, "%lg ", net->layers[n].biases[i]);
    }
    fprintf(out, "\n");
  }
  // write weights
  for(n=1; n<net->nl; n++) {
    for(i=0; i<net->layers[n].n; i++) {
      for(j=0; j<net->layers[n].nprev; j++) {
        fprintf(out, "%lg ", net->layers[n].weights[i][j]);
      }
      fprintf(out, "\n");
    }
  }
  fclose(out);
}

// TOOL FUNCTIONS

double ran_gauss(double mean, double sigma) {
  double u, u1, u2;
  u1 = (double) lrand48()/RAND_MAX;
  u2 = (double) lrand48()/RAND_MAX;
  u = mean + sigma*sqrt(-2.0*log(1.0-u1))*cos(2.0*M_PI*u2);
  return u;
}

void random_indices(int *index, int ndata) {
  int i, n, r;
  int temp[ndata];

  for(i=0; i<ndata; i++) {
    temp[i] = i;
  }
  n = ndata-1;
  for(i=0; i<ndata; i++) {
    r = lrand48()%(n+1);
    index[i] = temp[r];
    temp[r] = temp[n];
    n = n-1;
  }
}
