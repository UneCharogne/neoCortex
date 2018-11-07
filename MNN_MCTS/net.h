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
  double *deltas;
  double **grad_weights;
  double *grad_biases;
  double **delta_weights;
  double *delta_biases;
  void (*activation)(double *, double *, int);
  void (*derivative)(double *, double *, double *, int);
  double (*loss)(double *, double *, int);
} layer;

typedef struct {
  int nl;
  layer *layers;
} NN;

/*************** FUNCTIONS ***************/

// INITIALIZATION AND FINALIZATION
void init_net(NN *net, char *hidden, char *out, int nlayers, ...);
void init_layer(layer *l, char *type, int n, int nprev);
void init_gradients(layer *l);
void free_layer(layer *l);
void free_net(NN *net);
void load_net(NN *net, char *file_name);
void save_net(NN *net, char *file_name);

// INFORMATIVE
void print_network_structure(NN *net);
int get_output_size(NN *net);
int get_input_size(NN *net);

// FORWARD PROPAGATION
void linear_activation(layer *l, double *units_prev);
void id_activation(double *units_lin, double *units_act, int n);
void tanh_activation(double *units_lin, double *units_act, int n);
void sigmoid_activation(double *units_lin, double *units_act, int n);
void relu_activation(double *units_lin, double *units_act, int n);
void softmax_activation(double *units_lin, double *units_act, int n);
void mcts_activation(double *units_lin, double *units_act, int n);
void forward_propagation(NN *net, double *input_vector);
void predict(NN *net, double *vector, double *output);

// BACK PROPAGATION
void id_derivative(double *units_lin, double *units_act, double *fprime, int n);
void tanh_derivative(double *units_lin, double *units_act, double *fprime, int n);
void sigmoid_derivative(double *units_lin, double *units_act, double *fprime, int n);
void relu_derivative(double *units_lin, double *units_act, double *fprime, int n);
void mcts_derivative(double *units_lin, double *units_act, double *fprime, int n);
void delta_out(layer *l, double *target);
void delta(layer *l, layer *lnext);
void update_gradients(layer *l, layer *lprev);
void reset_gradients(layer *l);
void gradient_descent(layer *l, double rate, double momentum, double weight_decay, int batchsize);
void back_propagation(NN *net, double *target);

// TRAINING
void init_training(NN *net);
void sgd_update(NN *net, double rate, double momentum, double weight_decay, int batchsize);
void train(NN *net, int ntrain, double **dataset, double **target, double rate, double momentum, double weight_decay, int batchsize, int Niterations);

// LOSS
double square_loss(double *units_act, double *target, int n);
double cross_entropy_loss(double *units_act, double *target, int n);
double mcts_loss(double *units_act, double *target, int n);
double evaluate_loss(NN *net, double **dataset, double **target, int ndata);

// TOOL FUNCTIONS
double ran_gauss(double mean, double sigma);
void random_indices(int *index, int ndata);

#endif
