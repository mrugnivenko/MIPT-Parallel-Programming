#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <stdlib.h>
#include <vector>
using namespace std;

using subfunc = double (*)(double); // pointer to function accepts double and returns double

struct data {
  double a;
  double b;
  int N;
  subfunc f; 
  double result;
};

double sinxxx(double x) {
  return x == 0 ? 1. : sin(x) / x;
}

void *
integral(void *arg) {
  data *a = (data *)arg;
  // printf("a=%.6f b=%.6f\n", a->a, a->b);
  double d = a->b - a->a;
  a->result = a->f(a->a) + a->f(a->b);
  for (int i = 1; i < a->N; i++) {
    a->result += 2. * a->f(a->a + i*d / a->N);
  }
  a->result = a->result * d / a->N / 2.;
  return nullptr;
}

int main(int argc, char **argv) {
  const int THREADS = argc > 1 ? atoi(argv[1]) : 2;
  const int N = argc > 2 ? atoi(argv[2]) : 1000000;
  vector<pthread_t> threads(THREADS);
  vector<data> args(THREADS);
  
  for (int i = 0; i < THREADS; i++) {
    args[i].a = (double)i / THREADS;
    args[i].b = (double)(i+1) / THREADS;
    args[i].N = N / THREADS;
    args[i].f = sinxxx;
    pthread_create(&threads[i], nullptr, integral, &args[i]); 
  }
  double sum = 0;
  for (int i = 0; i < THREADS; i++) {
    pthread_join(threads[i], nullptr);
    sum += args[i].result;
  }
  printf("result=%.6f\n", sum);
}

