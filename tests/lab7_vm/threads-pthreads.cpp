/***********************************************************************
* Program:
*    Lab Threads  
*    Brother Jones, CS 345
* Author:
*    Bryson Gibbons
* Summary:
*    A simple threaded program that performs matrix multiplication on
*    two matrices that are statically declared, and outputs the result.
************************************************************************/
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <pthread.h>
#include <unistd.h>
using namespace std;

#define M 5 // number of rows in matrix A
#define K 6 // number of rows in matrix B -- number of columns in matrix A
#define N 8 // number of columns in matrix B

// [rows][columns]
int A [M][K] = { {1,4,3,7,9,1}, {2,5,4,8,6,3}, {3,6,5,8,2,3},
{3,8,8,1,4,1}, {1,5,4,5,7,9} };
int B [K][N] = { {1,5,6,5,7,9,8,2}, {1,2,3,5,5,6,7,8}, {3,5,9,7,3,1,4,1},
{8,3,1,2,6,5,2,4}, {3,8,8,1,4,1,3,3}, {8,7,6,5,4,3,2,1} };
int C [M][N]; // this is where the answer will be placed

/* structure for passing data to threads */
struct ThreadData
{
int i; /* row */
int j; /* column */
};

/************************************************************************************
* Use the argument to calculate the value of a specific square in a matrix, and
* return the calculated value. Made for use with pthread
************************************************************************************/
void *computeValue(void *data)
{
   int *value = new int;
   int i = ((ThreadData*)data)->i;
   int j = ((ThreadData*)data)->j;
   *value = 0;
   for (int k = 0; k < K; ++k)
   {
      *value += A[i][k] * B[k][j];
   }
   sleep(1);
   return (void *)value;
}

/***********************************************************************************
*  Create threads and manage their return data, then display results
***********************************************************************************/
int main()
{
   //pthread_setaffinity_np()
   // Variables to store information about the threads
   pthread_t *threadId = new pthread_t[M * N];
   int threadCount = 0;
   /* We have to create M * N worker threads */
   for (int i = 0; i < M; i++)
   {
      for (int j = 0; j < N; j++ )
      {
         ThreadData* tData = new ThreadData;
         tData->i = i;
         tData->j = j;
         /* Now create the thread passing it tData as a parameter */
         pthread_create(&threadId[threadCount++], NULL, computeValue, (void*)tData);
      }
   }
   
   // Get and store the result from the threads
   for (int i = 0; i < M * N; ++i)
   {
      int *data;
      // Get the data from the threads
      pthread_join(threadId[i], (void**)&data);
      C[i / N][i % N] = *data;
   }
   
   // Output the results of the calculations
   cout << "The product of matrices A and B is:\n";
   for (int i = 0; i < M; ++i)
   {
      for (int j = 0; j < N; ++j)
      {
         cout << setw(8) << C[i][j];
      }
      cout << endl;
   }
   
   return 0;
}
