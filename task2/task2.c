#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#define PI (double) 3.14159265359 

double solution(double x, double t, double u_0, double k, double l, double eps)  //считаем точное решение
{
double sum = 0.0; // сюда будем записывать точное решение
int m = 0; // сумма по m
double f;
do
{
f = exp(-k*PI*PI*(2*m+1)*(2*m+1)*t/l/l) / (2*m+1) * sin(PI*(2*m+1)*x/l);
sum += f;
m++;
}
while (fabs(f) > eps); //то есть будем идти до необходимой точности
sum *= u_0 * 4 / PI; // домножим на постоянный коэффициент сейчас, чтобы не возить его постоянно в сумме
return sum;
}

int calc_left_border(int p, int N, int pid)  // ищем левую границу отрезка
{
if (pid >= p || p < 1 || N < p) //номер процесса не явлется номре из списка или номер меньше 1 или процессов больше, чем отрезков разбиения
return -1; // ясно, что это плохо
int m = N / p;
int r = N - m * p;
int result;
if (pid < r)
result = pid * (m + 1);
else
result = r * (m + 1) + (pid - r) * m;
return result;
}

int calc_points(int p, int N, int pid) // ищем правую границу отрезка
{
if (pid >= p || p < 1 || N < p) //номер процесса не явлется номре из списка или номер меньше 1 или процессов меньше, чем отрезков разбиения
return -1; // ясно, что это плохо
int m = N / p;
int r = N - m * p;
int result;
if (pid < r)
result = m + 1;
else
result = m;
return result;
}


int main(int argc, char **argv)
{

int rank, size;
MPI_Status Status;
MPI_Init(&argc, &argv); //типа мэйна в мпи
MPI_Comm_size(MPI_COMM_WORLD, &size); //переменной size присваивается количество процессов
MPI_Comm_rank(MPI_COMM_WORLD, &rank); //переменной rank присваивается номер процесса

double start = MPI_Wtime();  // начало всей параллельной штуки

int N = 11; // количество точек разбиения
double T = 0.1;  // необходимый момент времени
double dt = 0.0002; // шаг по времени


double len = 1.0; // длина стержня
double h = (double) len / (N - 1);//длинна отрезочка разбиения
int steps = T / dt; //количество шагов по времени
double k = 1.0; // коэффициент теплопроводности
double u_0 = 1.0; // начальное усовие

double * u[2]; // создаём указатель на массив из двух элементов

u[0] = (double *) calloc(N, sizeof(double)); //освобождаем память для массива из N даблов - температуры в точках
u[1] = (double *) calloc(N, sizeof(double)); // аналогино, но один из массивов вспомогательный
// зададим начальные условия
int i, j; // j - точка 
for (j = 1; j < N - 1; j++)
u[0][j] = u_0; //в начальный момент времени распределение температоуры, кроме граничных точек там ноль
u[0][0] = u[1][0] = u[0][N - 1] = u[1][N - 1] = 0.0; //граничные условия на концах - там нули, причём нам надо два одинаковых массива

int l = calc_left_border(size, N - 2, rank) + 1; //ищем левую границу
int n = calc_points(size, N - 2, rank); //ищем сколько от неё до правой
int r = l + n; // правая граница

int s = 0; //для семафора

for (i = 0; i < steps; i++) //шаги по времени 
{
if (rank % 2 == 0) //чтобы не было тупика
{
if (rank > 0)
{
MPI_Ssend(&u[s][l - 0], 1, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD); //отсылка значения u[s][l-0] процессом rank - 1
MPI_Recv (&u[s][l - 1], 1, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, &Status); //получение значения u[s][l-1] процессом rank - 1
}
if (rank < size - 1)
{
MPI_Recv (&u[s][r - 0], 1, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, &Status); //получение значения u[s][r-0] процессом rank + 1 
MPI_Ssend(&u[s][r - 1], 1, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD); //отсылка значения u[s][r-1] процессом rank + 1
}
}
else
{
if (rank < size - 1)
{
MPI_Recv (&u[s][r - 0], 1, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, &Status);
MPI_Ssend(&u[s][r - 1], 1, MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD);
}
if (rank > 0)
{
MPI_Ssend(&u[s][l - 0], 1, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD);
MPI_Recv (&u[s][l - 1], 1, MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, &Status);
}
}
for (j = l; j < r; j++)
{
u[1 - s][j] = u[s][j] + (dt * k / (h * h)) * (u[s][j - 1] - 2 * u[s][j] + u[s][j + 1]);
}
s = 1 - s;
}

if (rank == 0)
{
for (i = 1; i < size; i++)
{
MPI_Recv(&u[s][calc_left_border(size, N - 2, i) + 1], calc_points(size, N - 2, i), MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &Status);
} //отправка i процессу
}
else
{
MPI_Ssend(&u[s][l], n, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD); //принятие от 0 процесса
}
if (rank == 0) //печать апроксимированного распределения температур по стержню
{
printf("Aproksimat : ");
printf("[");
for (i = 0; i < N - 1; i++)
{
printf("%f, ", u[s][i]);
}
printf("%f]\n", u[s][N - 1]);
}



if (rank == 0)  // печать реального распределения по стержню
{
printf("Solution : ");
printf("[");
for (i = 0; i < N - 1; i++)
{
printf("%f, ", solution(h * i, T, u_0, k, len, 1e-7));
}
printf("%f]\n", solution(h * (N - 1), T, u_0, k, len, 1e-7));
}
free(u[0]); //освобождаем память
free(u[1]);

if (rank == 0) //основной процесс распечатает время работы
{
printf("Time: %lf\n", MPI_Wtime() - start);
}
MPI_Finalize();
return 0;

}
