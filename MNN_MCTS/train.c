#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include "net.h"

#define MAX_DATA 100000
#define BUFSIZE 100000

int main(int argc, char *argv[]) {
  NN net;
  double learning_rate, momentum, weight_decay;
  int Niterations, batchsize;
  int ninput, noutput, ndata, ndatain, ndataout;
  int i, j;
  int n, nit, ntrainings;
  double **dataset, **target;
  char file_data[80], file_target[80], file_network[80];
  char buffer[BUFSIZE];
  FILE *in, *out;

  // check if user gave file name
  if(argc != 2) {
    printf("\nERROR: network to load not specified!\n");
    exit(1);
  }
  // set names of network, dataset and target files
  sprintf(file_network, "%s_network.txt", argv[1]);
  sprintf(file_data, "%s_input.dat", argv[1]);
  sprintf(file_target, "%s_output.dat", argv[1]);

  // init seed for random generator
  srand48(time(0));


  // load network
  load_net(&net, file_network);
  print_network_structure(&net);
  noutput = get_output_size(&net);
  ninput = get_input_size(&net);

  // read files
  if((in = fopen(file_data, "r")) == NULL) {
    printf("Error opening the file \"%s\", program will be arrested.", file_data);
    exit(EXIT_FAILURE);
  }
  if((out = fopen(file_target, "r")) == NULL) {
    printf("Error opening the file \"%s\", program will be arrested.", file_target);
    exit(EXIT_FAILURE);
  }

  ndatain = 0;
  while (fgets(buffer, BUFSIZE, in) != NULL) {
    ndatain++;
  }
  fclose(in);

  ndataout = 0;
  while (fgets(buffer, BUFSIZE, out) != NULL) {
    ndataout++;
  }
  fclose(out);

  if(ndatain != ndataout) {
    printf ("Error, number of inputs different from the number of outputs, program will be arrested.\n");
    exit(EXIT_FAILURE);
  }
  ndata = ndatain;


  if(ndata < MAX_DATA) {
  	printf("Error, number of data not enough to build a batch.\n");
  	exit(EXIT_FAILURE);
  }
  
  printf("\nFiles read.\n%d examples to process.\n\n", ndata);
	  
  init_training(&net);
  
  for(nit = 0; nit<20;nit++) {printf("\n\nNit = %d\n", (nit+1));
	  	if((in = fopen(file_data, "r")) == NULL) {
			printf("Error opening the file \"%s\", program will be arrested.", file_data);
			exit(EXIT_FAILURE);
	  	}
	  	if((out = fopen(file_target, "r")) == NULL) {
			printf("Error opening the file \"%s\", program will be arrested.", file_target);
			exit(EXIT_FAILURE);
	  	}


	  ntrainings = (ndata / MAX_DATA);
		  
	  if((dataset = (double**)malloc(MAX_DATA * sizeof(double*))) == NULL) {
		printf("Error reallocating the memory for the dataset, program will be arrested.\n");
		exit(EXIT_FAILURE);
	  }

	  for(i=0;i<MAX_DATA;i++) {
	    if((dataset[i] = (double*)malloc(ninput * sizeof(double))) == NULL) {
	      printf("Error allocating the memory for the %d-th line of dataset, program will be arrested.\n", i);
	      exit(EXIT_FAILURE);
	    }
	  }

	  if((target = (double**)malloc(MAX_DATA * sizeof(double*))) == NULL) {
	    printf("Error reallocating the memory for the target, program will be arrested.\n");
	    exit(EXIT_FAILURE);
	  }

	  for(i=0;i<MAX_DATA;i++) {
	    if((target[i] = (double*)malloc(noutput * sizeof(double))) == NULL) {
	      printf("Error allocating the memory for the %d-th line of target, program will be arrested.\n", i);
	      exit(EXIT_FAILURE);
	    }
	  }


	  for(n=0;n<(ntrainings);n++) { 
		  for(i=0;i<MAX_DATA;i++) {
		    for(j=0;j<ninput;j++) {
		      fscanf(in, "%lf ", &(dataset[i][j]));
		    }
		    fscanf(in, "\n");
		  }

		  for(i=0;i<MAX_DATA;i++) {
		    for(j=0;j<noutput;j++) {
		      fscanf(out, "%lf ", &(target[i][j]));
		    }
		    fscanf(out, "\n");
		  }

		  // init training
		  learning_rate = 0.1;
		  if(nit > 14) {
		  	learning_rate = 0.05;
		  }
		  momentum = 0.9;
		  weight_decay = 0.0001;
		  batchsize = 500;
		  Niterations = 1;

		  // train
		  train(&net, MAX_DATA, dataset, target, learning_rate, momentum, weight_decay, batchsize, Niterations);
	  }

	  for(i=0;i<MAX_DATA;i++) {
	    free(dataset[i]);
	  }
	  free(dataset);
	  
	  for(i=0;i<MAX_DATA;i++) {
		free(target[i]);
	  }
	  free(target);
		  
	  if((dataset = (double**)malloc((ndata % MAX_DATA) * sizeof(double*))) == NULL) {
		printf("Error reallocating the memory for the dataset, program will be arrested.\n");
		exit(EXIT_FAILURE);
	  }

	  for(i=0;i<(ndata % MAX_DATA);i++) {
	    if((dataset[i] = (double*)malloc(ninput * sizeof(double))) == NULL) {
	      printf("Error allocating the memory for the %d-th line of dataset, program will be arrested.\n", i);
	      exit(EXIT_FAILURE);
	    }
	  }

	  if((target = (double**)malloc((ndata % MAX_DATA) * sizeof(double*))) == NULL) {
	    printf("Error reallocating the memory for the target, program will be arrested.\n");
	    exit(EXIT_FAILURE);
	  }

	  for(i=0;i<(ndata % MAX_DATA);i++) {
	    if((target[i] = (double*)malloc(noutput * sizeof(double))) == NULL) {
	      printf("Error allocating the memory for the %d-th line of target, program will be arrested.\n", i);
	      exit(EXIT_FAILURE);
	    }
	  }

	  for(i=0;i<(ndata % MAX_DATA);i++) {
	    for(j=0;j<ninput;j++) {
	      fscanf(in, "%lf ", &(dataset[i][j]));
	    }
	    fscanf(in, "\n");
	  }

	  for(i=0;i<(ndata % MAX_DATA);i++) {
	    for(j=0;j<noutput;j++) {
	      fscanf(out, "%lf ", &(target[i][j]));
	    }
	    fscanf(out, "\n");
	  }

	  // init training
	  learning_rate = 0.1;
	  if(nit > 14) {
	  	learning_rate = 0.05;
	  }
	  momentum = 0.9;
	  weight_decay = 0.0001;
	  batchsize = 500;
	  Niterations = 1;

	  // train
	  train(&net, (ndata % MAX_DATA), dataset, target, learning_rate, momentum, weight_decay, batchsize, Niterations);
	  
	  // save trained network
	  sprintf(file_network, "%s_network_new.txt", argv[1]);
	  save_net(&net, file_network);

	  for(i=0;i<(ndata % MAX_DATA);i++) {
	    free(dataset[i]);
	  }
	  free(dataset);
	  
	  for(i=0;i<(ndata % MAX_DATA);i++) {
		free(target[i]);
	  }
	  free(target);

	  fclose(in);
	  fclose(out);
  }

  free_net(&net);

  return 0;
}
