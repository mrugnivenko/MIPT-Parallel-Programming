#include <stdio.h> //библиотека функций ввода и вывода
#include <mpi.h> // заголовочный файл mpi для работы с параллельными процессами

int N=1000;
double  time;
double start_proc;
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
MPI_Comm_rank(MPI_COMM_WORLD, &myrank); /* каждому процессу присваивается уникальный номер */
double start_posl = MPI_Wtime(); //время начала  процесса
double sum = 0.0; //переменная для интеграла, который считаем послеовательно
double delta;
delta = (double) 1 / N; //  шаг интегрирования 

double x = 0.0; //начальная точка
int i = 0;
if(myrank == 0) // очновной процесс
{
for (i = 0; i < N; i++)
{
sum += delta * ( 0.5 * ( f(x) + f(x+delta) ) );
x += delta;
}
time = MPI_Wtime() - start_posl;
printf("posledovatelno %20.15lf\n", sum);
}

double coords[2] = {}; //массив двух точек для метода трапеций
if(myrank == 0)
{
start_proc = MPI_Wtime();
for(i = 0; i < size - 1; i++)
{
coords[0] = (double)i / size;
coords[1] = ((double) (i + 1) ) / size;
MPI_Send(coords, 2, MPI_DOUBLE, i, 1,MPI_COMM_WORLD); //пересылка процессу с номером i
}
coords[0] = 1.0 - ( (double) 1) / size;
coords[1] = 1.0;
MPI_Send(coords, 2, MPI_DOUBLE, size - 1, 1, MPI_COMM_WORLD);//пересылка последнему процессу 
}
MPI_Recv(coords, 2, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD,&Status);
/* получение от процесса с номером 0: */

sum = 0.0;
x = coords[0];
while ( x < coords[1] )
{
sum += delta * ( 0.5 * ( f(x) + f(x+delta) ) );
x += delta;
}
printf("myrank = %d, my_integral = %lf\n", myrank, sum);

MPI_Send( &sum, 1, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD);
double result = 0;
if (myrank == 0)
{
for(i = 0; i < size; i++)
{
MPI_Recv(&sum, 1, MPI_DOUBLE, i, 1, MPI_COMM_WORLD, &Status);
result += sum;
}
double finish_proc = MPI_Wtime();
printf("myrank = %d, result_integral = %20.15lf\n", myrank, result);
printf("uskorenie : %f\n",(time)/(finish_proc - start_proc));
}
MPI_Finalize();
return 0;
}

