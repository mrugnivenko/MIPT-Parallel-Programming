#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>
#include <time.h>

#define A 0.0 // границы
#define B 10.0 
#define T1 0.0 // условия
#define T2 0.0
#define c 1.0
#define NUM_THREADS 4
double T = 4.0;
double h = 0.1;
double tau = 0.1;

// граничные условия
double g(double x){
   if (x>=0 && x<=2)
         return x*(2-x);
      else
         return 0;
}
// точное решение
double solution (double x, double t){
   return g(x-c*t);
}

//данные для нитей
typedef struct thread_data_t {
   int rank;
   int size;
   double ** u;
   int nh;
   int nt;
   sem_t * sem_begin;
   sem_t * sem_finish;
   double tau;
   double h;
} thread_data;

void * start_func (void *arg){
    thread_data data = *((thread_data *) arg);//значение лежащее по указателю на thread_data
   //достаем данные из аргумента
   double ** u = data.u;
   int rank = data.rank;
   int size = data.size;
   int nh = data.nh;
   int nt = data.nt;
   double tau = data.tau;
   double h = data.h;
   sem_t * sem_begin = data.sem_begin;
   sem_t * sem_finish = data.sem_finish;

   int i, j, un = 1;
   int st = (int)(nh/size)*rank+1;
   int fn = (int)(nh/size)*(rank+1)+1;
// исспользование разностной схемы
   for (i = 0; i < nt; i++){
      for (j = st; j < fn; j++)
         u[!un][j] = u[un][j] - c * tau* (u[un][j] - u[un][j-1])/(h);
      un = !un;

// открытие семафора для левого края
   sem_post(&sem_begin[rank]);//увеличивает семафор, то есть кого-то пропустили из очереди
// дожидаемся изменения значения левого края другим тредом
   if (rank!=0)
     	sem_wait(&sem_begin[rank-1]);
       //уменьшает семафор, то есть кто-то встал в очередь
     else
     	sem_wait(&sem_begin[size-1]);
// аналогично с правым
   if (rank!=0)
     	sem_post(&sem_finish[rank-1]);
     else
     	sem_post(&sem_finish[size-1]);
   sem_wait(&sem_finish[rank]);
   }
   return NULL;
}


int main(int argc, char *argv[]){
   int nh = (int)((B-A)/h); //количество рабочих точек пространства
   int nt = (int)(T/tau); //количесвто точек по времени
   int j;
// инициальзация массива тредов и семафором
   pthread_t* threads = (pthread_t*)calloc(NUM_THREADS, sizeof(pthread_t));//выделяется память и возвращает указатель на первый байт выделенной памяти типа pthread_t
   sem_t * sem_begin = (sem_t*)calloc(NUM_THREADS, sizeof(sem_t));
   sem_t * sem_finish = (sem_t*)calloc(NUM_THREADS, sizeof(sem_t));
   double *u[2];//массив массивов
   u[0] = (double*)calloc(nh, sizeof(double));//два массива для обновления данных в точках
   u[1] = (double*)calloc(nh, sizeof(double));
// начальные значения
   for (j = 0; j < nh; j++){
         u[0][j] = g(A+(B-A)*j/nh);
         u[1][j] = g(A+(B-A)*j/nh);
      }

   u[0][0] = u[1][0] = T1;
   u[0][nh - 1] = u[1][nh - 1] = T2;
// инициализация начальных данных через структуру
   thread_data * arg = (thread_data *)calloc(NUM_THREADS, sizeof(thread_data));
   for (j = 0; j < NUM_THREADS; j++){
         (arg [j]).rank = j;
         (arg [j]).size = NUM_THREADS;
         (arg [j]).u = u;
         (arg [j]).nh = nh;
         (arg [j]).nt = nt;
         (arg [j]).sem_begin = sem_begin;
         (arg [j]).sem_finish = sem_finish;
         (arg [j]).tau = tau;
         (arg [j]).h = h;
   }

   struct timespec begin, end;
   int i;
   double elapsed;
// инит он и в африке инит
   for (j = 0; j < NUM_THREADS; j++){
      sem_init (&(sem_finish[j]), 0, 0);//инициализируем нулём
  		sem_init (&(sem_begin[j]), 0, 0);
  	}
   clock_gettime(CLOCK_REALTIME, &begin);
// создание тредов
   for (i=0; i < NUM_THREADS; i++){
      pthread_create(&threads[i], NULL, start_func, (void *)&(arg[i]));
   }
// дожидаемся тредов
   for (i=0; i < NUM_THREADS; i++){
      pthread_join(threads[i], NULL);// ожмдаем завершение потока pthr[i]
   }
// убираем за собой семафоры
   for (j = 0; j < NUM_THREADS; j++){
      sem_destroy(&(sem_finish[j]));
      sem_destroy(&(sem_begin[j]));
   }
// печать значений
   printf("x\tNumerical\tExact\n");
   for (i = 0; i < nh; i++){
//		printf("%lf\n", solution(A+h*i,0)); //начальное распределение
      printf("%.1f\t%.4f\t\t%.4f\n", A+h*(i), u[1][i], solution(A+h*(i),T));
   }
// завершение замера времени
   clock_gettime(CLOCK_REALTIME, &end);
   elapsed = end.tv_sec - begin.tv_sec;
   elapsed += (end.tv_nsec - begin.tv_nsec) / 1000000000.0;
   printf("Time:\t%.4f\n", elapsed);

    // убираем за собой
   free(threads);
   free(u[0]);
   free(u[1]);
   free(sem_finish);
   free(sem_begin);
   free(arg);
   return 0;
}
