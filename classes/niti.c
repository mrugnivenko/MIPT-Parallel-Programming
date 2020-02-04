#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<semaphore.h>
#define NUM_THREADS 2
sem_t sem;

void* start_func(void* papram){
int val;
sem_wait(&sem);
sem_post(&sem);
sem_getvalue(&sem,&val);
return NULL;
}

void* start_func(void * param){
int* local;
local = (int*)param;
printf("mpit");
pthread_exit(NULL);
}

int main(int argc , char *argv[]){
int param;
int rc;
void* arg;
pthread_t pthr;
rc = pthread_create(&pthr, NULL, start_func, NULL);
pthread_join(pthr, NULL);
sem_init(&sem,0,1);
sem_destroy(&sem);
for (i = 0; i < NUM_THREADS;i++){
	rc = pthread_create(&pthr[i],NULL, start_func, NULL);
	if (rc) printf("ERROR;return code from pthread_create() is %d\n", rc);
}
for (i = 0; i < NUM_THREADS; i++){
	rc = pthread_join(pthr[i],NULL);
	
}

return 0;

}