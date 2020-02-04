//
//  task31.c
//  
//
//  Created by Виталий Алексеевич on 08/05/2019.
//

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>
#define NUM_THREADS 4 //количество потоков
#define N 1e9 // число точек, которые надо кинуть
sem_t sem;//задаём семафор, то есть переменную, которая увеличивается и уменьшается

double Global = 0.0 ;

// здесь реализован алгоритм возвращения значения через _exit() метка (1) отправлнение, (2) прием
int x_k=0,y_k=100;
void* start_func(void* param /*указатель над данные неизвестного типа*/)// функция вернёт указатель на что-то
{
    int local;//сюда положим то, что по адресу param
    int i, x_0, y_0;//i для счётчика,
    double x, y;//для локального подсчёта интеграла
    double *res = (double*)calloc(1, sizeof(double)); /*указатель над данные типа  дабл размером в один дабл, сюда пишем значение интеграла*/
    // изменения счетчика
    sem_wait(&sem);//уменьшает семафор, то есть кто-то встал в очередь
    x_k++;
    y_k++;
    x_0 = x_k;
    y_0 = y_k;
    sem_post(&sem);//увеличивает семафор, то ест кого-то пропустили из очереди, тут
    local = *(int*)param; //это интовое значение, на которое указывает указатель,
    // подсчет интеграла локально
    for (i = 0; i < local; i++)
    {
        x = ((double)rand_r(&x_0/*это указательна x_0*/) / RAND_MAX) * M_PI;//M_PI - пи, rand_r - возвращает случайное число в промежутке [0,RAND_MAX]
        y = ((double)rand_r(&y_0) / RAND_MAX);
        if(y <= sin(x)){
            *res += x * y;//разыменовываем указатель и в переменную res кладём число
        }
    }
    
    sem_wait(&sem);
    Global+=*res;
    sem_post(&sem);
    //    (1)
    pthread_exit(res);//завершает поток и возвращает результат выполнения   потока, типа retrun ...
}
int main(int argc, char* argv[])
{
    // инициальзация семофора 1
    sem_init(&sem,0,1);//инициализируем семафор, сначала там 1, то есть очередь открыта
    struct timespec begin, end;//структура с секундами и милисекундам для работы со временем
    double elapsed; // тут будет разночть времён, то есть время работы программы
    clock_gettime(CLOCK_REALTIME, &begin);//получает время с системных часов  реального времени(с начала эпохи) и пишет в структуру begin
    int param0 = N, param, i; // в param0 лежит общее количество точек
    void* arg;
    // создание массива тредов
    pthread_t pthr[NUM_THREADS];
    // заполнение массива тредов и отправка им задания
    for(i=0; i<NUM_THREADS; i++)
    {
        param = param0/(NUM_THREADS-i);//количество точек, которые использует поток
        if(param%NUM_THREADS != 0)
            param++;
        param0-=param;//количество точек стало меньше, ну и потоков, что их могут забрать тоже, поэтому вычитали i
        if(pthread_create(&pthr[i], NULL, start_func, (void*)&param) != 0)/*создаём поток, даём ему точки и прогу*/ {return -2;}
    }
    double result = 0;//запишем результат сюда
    double *tmp = (double*)calloc(1, sizeof(double));//указатель на данны типа в один дабл
    // прием данных при помощи _join
    for(i = 0; i< NUM_THREADS;i++){
        //        (2)
        // прием значений из каждого треда и их суммирование
        if(pthread_join(pthr[i], (void**)&tmp) != 0){return -1;}// ожмдаем завершение потока pthr[i], в tmp помещают данные, возвращаемые потоком через функцию pthread_exit()
        result += *tmp;
    }
    // печать результата
    result = result*M_PI/N;
    printf("result = %lf\n",result);
    sem_destroy(&sem);//удаляем семафор
    
    clock_gettime(CLOCK_REALTIME, &end);//получает время с системных часов  реального времени(с начала эпохи) и пишет в структуру end
    
    // печать времени работы
    elapsed = end.tv_sec - begin.tv_sec;    // время работы в секундах
    elapsed += (end.tv_nsec - begin.tv_nsec) / 1000000000.0;//добавляем наносекунды, переведённые в секунды
    printf("time = %.9lf\n",elapsed);    //
    printf("Int by global method: %lf\n", Global*M_PI/N);
    return 0;
}
