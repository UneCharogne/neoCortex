#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include "net.h"



int main() {
  int i;
  NN net;
  double x[1], y[1], out[1];
  FILE *f;

  load_net(&net, "net.txt");
  print_network_structure(&net);
  if((f = fopen("output.dat","w")) == NULL) {
    printf("\nERROR while opening output file\n");
    exit(0);
  }
  for(i=0; i<100; i++) {
    x[0] = -5.0+i*0.1;
    y[0] = exp(-0.5*x[0]*x[0])/sqrt(2.0*M_PI);
    predict(&net, x, out);
    fprintf(f, "%lf %lf %lf\n", x[0], out[0], y[0]);
  }
  fclose(f);
  free_net(&net);


  return 0;
}
