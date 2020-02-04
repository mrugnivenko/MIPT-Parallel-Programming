#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
          
double f(double x)
{          
return 4 / (1 + x * x);
}          
int N = 10;
int main(int argc, char *argv[]){
        int coord[2] = {};
        int myrank = 0, size = 0, d, r; //size - для количества процессов,myrank - для уникального номера
        double begin, end, total;
        MPI_Status Status; 
        MPI_Init(&argc, &argv); // начало MPI программы -  после этого начинают работу все остальные N процессов
        MPI_Comm_size(MPI_COMM_WORLD,&size); /* переменной size присваивается число, равное кол-ву процессов */
        MPI_Comm_rank(MPI_COMM_WORLD, &myrank); //дает каждому процессу номер
        begin = MPI_Wtime(); 
        double In = 0.0;
        double dx;
        dx = (double) 1 / N; //  шаг интегрирования 

        double x = 0.0; //начальная точка
        int i = 0;
        if(myrank == 0) // оcновной процесс
        {
                for (i = 0; i < N; i++)
                {
                        In += dx * ( 0.5 * ( f(x) + f(x+dx) ) );
                        x += dx;
                }
                end = MPI_Wtime();
                total = end - begin;
                printf("sequent comp.: %20.13lf\n", In);
        }
        if(myrank == 0)
        {
                begin = MPI_Wtime();
                d = N/size;
                r = N%size;
                coord[1] = 0;
                for(i = 0; i < r; i++)
                {
                        coord[0] = coord[1];
                        coord[1] += (d + 1);
                        MPI_Send(coord, 2, MPI_INT, i, 1,MPI_COMM_WORLD); //пересылка процессу с номером i
                }
                for(i = r; i < size; i++)
                {
                        coord[0] = coord[1];
                        coord[1] += (d);
                        MPI_Send(coord, 2, MPI_INT, i, 1,MPI_COMM_WORLD); //пересылка процессу с номером i
                }

        }
        MPI_Recv(coord, 2, MPI_INT, 0, 1, MPI_COMM_WORLD,&Status);
/* получение от процесса с номером 0: */


        In = 0.0;
        x = coord[0];
        while ( x < coord[1])
        {
                In += dx * ( 0.5 * ( f(x*dx) + f((x+1)*dx) ) );
                x += 1;
        }
        printf("myrank = %d, I = %20.13lf\n", myrank, In);

        MPI_Send( &In, 1, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD);
        double result = 0;
        if (myrank == 0)
        {
                for(i = 0; i < size; i++)
                {
                        MPI_Recv(&In, 1, MPI_DOUBLE, i, 1, MPI_COMM_WORLD, &Status);
                        result += In;
                }
                end = MPI_Wtime();
                //MPI_Waitall(size, request, status);
                printf("myrank = %d, I_f = %20.13lf\n", myrank, result);
                printf("s : %f, p: %f\n", total, (end - begin));
                printf("acceleration : %f\n",(total)/(end - begin));
        }
        MPI_Finalize(); //больше  не может  использовать функции и переменные связанные с MPI
        return 0;
}

