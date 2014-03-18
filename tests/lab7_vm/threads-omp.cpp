/***********************************************************************
* Program:
*    Lab OpenMPmatrixMult  
*    Brother Jones, CS 345
* Author:
*    Bryson Gibbons
* Summary:
*    A simple threaded program that performs matrix multiplication on
*    two matrices that are statically declared, and outputs the result.
* Conclusions:
*    -OpenMP is very powerful, but it has a limit where the data controls
*     do not work with non-scalar data types, like arrays or any data
*     type that is accessed using pointers
*    -The manner in which variables are declared affects the capability
*     for parallelization of a portion of code. If 'k' is declared before
*     the #pragma omp parallel statement, there is a race condition with
*     the innermost for loop that cannot be remedied without removing
*     the parallel processing capability for that section of code.
*     If k is declared after the #pragma omp parallel statement, it
*     is thread-safe even if the number of threads specified for the
*     parallel processing is beyond what can be used by the other 2 loops.
*    -You can nest #pragma omp parallel statements, but their
*     usability and understandability is greatly affected.
*     The only advantage to this in this case is that you can make
*     each for loop completely parallel, and prevent a race condition.
*     However, there are also usually limitations on how parallel you
*     can make the nested statements based on the implementation of OpenMP.
************************************************************************/
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <unistd.h>
using namespace std;

#define M 5 // number of rows in matrix A
#define K 6 // number of rows in matrix B -- number of columns in matrix A
#define N 8 // number of columns in matrix B

/***********************************************************************************
*  Create threads and manage their return data, then display results
***********************************************************************************/
int main()
{
   // [rows][columns]
   int A [M][K] = { {1,4,3,7,9,1}, {2,5,4,8,6,3}, {3,6,5,8,2,3},
   {3,8,8,1,4,1}, {1,5,4,5,7,9} };
   int B [K][N] = { {1,5,6,5,7,9,8,2}, {1,2,3,5,5,6,7,8}, {3,5,9,7,3,1,4,1},
   {8,3,1,2,6,5,2,4}, {3,8,8,1,4,1,3,3}, {8,7,6,5,4,3,2,1} };
   int C [M][N]; // this is where the answer will be placed
   
   // Declare the variables that we want private, but need to be used in relation to other threads
   int i;
   int j;
   // We want M * N threads, and to have both j and i be private, in their respective scopes of usage
   // A, B, and C can be listed as shared or private, it doesn't matter because they will be
   //   treated as pointers by OpenMP, so we will let them be handled according to the default.
   //#pragma omp parallel for num_threads(M * N)	\
   //   firstprivate(j) lastprivate(i)
   #pragma omp parallel for collapse(2) num_threads(M * N)
   for (i = 0; i < M; i++)
   {
      for (j = 0; j < N; j++ )
      {
         // Make sure we start off with a good value
         C[i][j] = 0;
         // By declaring k here, we prevent it from being shared, and ensure thread-safe-ness
         for (int k = 0; k < K; ++k)
         {
            C[i][j] += A[i][k] * B[k][j];
         }
         sleep(1);
      }
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
