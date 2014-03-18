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
//#include <thread>
#include <future>
#include <chrono>
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

/************************************************************************************
* Use the argument to calculate the value of a specific square in a matrix, and
* return the calculated value. Made for use with pthread
************************************************************************************/
int computeValue(int i, int j)
{
   int value = 0;
   C[i][j] = 0;
   for (int k = 0; k < K; ++k)
   {
      /*/
      C[i][j] += A[i][k] * B[k][j];
      /*/
      value += A[i][k] * B[k][j];
      /**/
   }
   std::this_thread::sleep_for(chrono::seconds(1));
   return value;
}

/***********************************************************************************
*  Create threads and manage their return data, then display results
***********************************************************************************/
int main()
{
   // Variables to store information about the threads
   /*/
   thread threadId[M * N];
   /*/
   future<int> threadId[M * N];
   /**/
   int threadCount = 0;
   /* We have to create M * N worker threads */
   for (int i = 0; i < M; i++)
   {
      for (int j = 0; j < N; j++ )
      {
         /*/
         threadId[threadCount++] = thread(computeValue, i, j); 
         /*/
         threadId[threadCount++] = async(launch::async, computeValue, i, j);
         /**/
         /* Now create the thread passing it tData as a parameter */
      }
   }
   
   // Get and store the result from the threads
   for (int i = 0; i < M * N; ++i)
   {
      // Get the data from the threads
      /*/
      threadId[i].join();
      /*/
      C[i / N][i % N] = threadId[i].get();
      /**/
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
