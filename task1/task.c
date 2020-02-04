#include <stdio.h> //библиотека функций ввода и вывода
#include <mpi.h> // заголовочный файл mpi для работы с параллельными процессами

int N = 1000;

float f(double x) //double для большей точности
{
return 4 / (1 + x * x);
}

int main(int argc, char *argv[])
{
MPI_Status Status;
int size = 0, myrank = 0; //size - для количества процессов,myrank - для уникального номера
MPI_Init(&argc, &argv); // начало MPI программы -  после этого начинают работу все остальные N процессов

MPI_Comm_size(MPI_COMM_WORLD, &size); /* переменной size присваивается число, равное кол-ву процессов */
MPI_Comm_rank(MPI_COMM_WORLD, &myrank);  /* каждому процессу присваивается уникальный номер */
double start = MPI_Wtime();
double summ = 0.0;
double d = 1.0 / N; //  шаг интегрирования 
double finish_posl;
double x = 1.0;
int i = 0;
if (myrank == 0) // оcновной процесс
{
printf("N = %d\n",N);
for (i = 0; i < N; i++)
{
summ +=  d * ( 0.5 * ( f(x) + f(x + d) ) );
x += d;
}
finish_posl = MPI_Wtime()-start;
printf("int_posl: %20.15lf time_posl: %lf\n", summ, finish_posl);
}
double coords[2]; //массив двух точек для метода трапеций
if(myrank == 0)
{
start = MPI_Wtime();
for(i = 0; i < size-1; i++)
{
coords[0] = d * ((int) (N / (size-1))) * i;
coords[1] = d * ((int) (N / (size-1))) * (i+1);
MPI_Send(coords, 2, MPI_DOUBLE, i, 1,MPI_COMM_WORLD); //пересылка процессу с номером i
}
coords[0] = 1 - (N % (size - 1)) * d;
coords[1] = 1;
MPI_Send(coords, 2, MPI_DOUBLE, i, 1,MPI_COMM_WORLD); //пересылка последнему процессу 
}

MPI_Recv(coords, 2, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD,&Status); /* получение от процесса с номером 0: */

summ = 0.0;
x = coords[0];

while ( x < coords[1] )
{
summ = summ + d * (0.5 * (f(x) + f(x + d)));
x = x + d;
}
printf("myrank = %d, integral = %lf\n", myrank, summ);

MPI_Send( &summ, 1, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD);
if (myrank == 0)
{
double res = 0;
for(i = 0; i < size; i++)
{
MPI_Recv(&summ, 1, MPI_DOUBLE, i, 1, MPI_COMM_WORLD, &Status);
res = res + summ;
}
double finish_proc = MPI_Wtime() - start;
printf("papall = %20.15lf time = %lf\n", res, finish_proc);
printf("s = %lf\n", finish_posl / finish_proc);
}

MPI_Finalize();
return 0;
}
                                                             

