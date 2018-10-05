#ifndef NET_H
#define NET_H


/************** STRUCTS ******************/
typedef struct {
  int n, nprev;
  char type[10];
  double *units_lin;
  double *units_act;
  double **weights;
  double *biases;
  void (*activation)(double *, double *, int);
} layer;

typedef struct {
  int nl;
  layer *layers;
} NN;

/*************** FUNCTIONS ***************/
void init_net(NN *net, char *hidden, char *out, int nlayers, ...);
void init_layer(layer *l, char *type, int n, int nprev);
void free_layer(layer *l);
void free_net(NN *net);
void load_net(NN *net, char *file_name);
void print_network_structure(NN *net);
int get_output_size(NN *net);
int get_input_size(NN *net);
void linear_activation(layer *l, double *units_prev);
void id_activation(double *units_lin, double *units_act, int n);
void tanh_activation(double *units_lin, double *units_act, int n);
void sigmoid_activation(double *units_lin, double *units_act, int n);
void relu_activation(double *units_lin, double *units_act, int n);
void softmax_activation(double *units_lin, double *units_act, int n);
void mcts_activation(double *units_lin, double *units_act, int n);
void forward_propagation(NN *net, double *input_vector);
void predict(NN *net, double *vector, double *output);

#endif