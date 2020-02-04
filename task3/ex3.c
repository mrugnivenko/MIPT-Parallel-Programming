#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>

#define NUM_THREADS 4
#define N 1e9

sem_t sem;

int x_k=0,y_k=42;

double Global = 0.0;

void* start_func(void* param)
{
	int local;
	int i, x_0, y_0;
	double x, y;
	double *res = (double*)calloc(1, sizeof(double));

	sem_wait(&sem);
	x_k++;
	y_k++;
	x_0 = x_k;
	y_0 = y_k;
	sem_post(&sem);
	local = *(int*)param;

	for (i = 0; i < local; i++)
	{
		x = ((double)rand_r(&x_0) / RAND_MAX) * M_PI;
		y = ((double)rand_r(&y_0) / RAND_MAX);
		if(y <= sin(x))
		{
			*res += x * y;
		}
	}

	sem_wait(&sem);
	Global+=*res;
	sem_post(&sem);

	pthread_exit(res);
}

int main(int argc, char* argv[])
{
	sem_init(&sem,0,1);
	struct timespec begin, end;
	double elapsed;
	clock_gettime(CLOCK_REALTIME, &begin);
	int param0 = N, param, i;
	void* arg;

	pthread_t pthr[NUM_THREADS];

	param = param0/NUM_THREADS;

	for(i=0; i<NUM_THREADS; i++)
	{
		if(pthread_create(&pthr[i], NULL, start_func, (void*)&param) != 0)
			return -1;
	}

	double result = 0;
	double *tmp = (double*)calloc(1, sizeof(double));

	for(i = 0; i< NUM_THREADS;i++)
	{
		if(pthread_join(pthr[i], (void**)&tmp) != 0)
			return -1;
		result += *tmp;
	}

	result = result*M_PI/N;
	printf("Integral:  %lf\n",result);
	sem_destroy(&sem);

	clock_gettime(CLOCK_REALTIME, &end);

	elapsed = end.tv_sec - begin.tv_sec;
	elapsed += (end.tv_nsec - begin.tv_nsec) / 1000000000.0;
   	printf("Time: %.9lf\n",elapsed);
	printf("Int by global method: %lf\n", Global*M_PI/N);
	return 0;
}
