#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// PROTOTYPES
double gamma1(double alpha);
void dirichlet(double alpha, int size, double *p);


// FUNCTIONS
double gamma1(double alpha) {
  double umax, vmin, vmax;
  double u, t, t1;

  umax = pow((alpha/exp(1.0)), 0.5*alpha);
  vmin = -2.0/exp(1.0);
  vmax = 2.0*alpha/exp(1.0)/(exp(1.0)-alpha);
  do {
    u = (double) lrand48()/RAND_MAX;
    u *= umax;
    t = (double) lrand48()/RAND_MAX;
    t = (t*(vmax-vmin)+vmin)/u;
    t1 = exp(t/alpha);
  } while(2.0*log(u)>(t-t1));
  if(alpha>=0.01) {
    return t1;
  }
  else {
    return t/alpha;
  }
}

void dirichlet(double alpha, int size, double *p) {
  int i;
  double norm;

  norm = 0.0;
  for(i=0; i<size; i++) {
    *(p + i) = gamma1(alpha);
    norm += *(p + i);
  }
  for(i=0; i<size; i++) {
    *(p + i) /= norm;
  }
}
