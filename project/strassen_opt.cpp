/***************************************************************************
* Program:
*    Optimized Strassen, Senior Project
* Author:
*    Bryson Gibbons
* Summary:
*    An optimized program for multiplying two very large matrices
*    Uses both Strassen's algorithm and standard matrix multiplication
*    
***************************************************************************/

#include <iostream>
#include <thread>
#include "errorColors.h"
#include "matrix.h"
using namespace std;

int main(int argc, char* argv[])
{
   int size = 32;
   ifstream inFile;
   ifstream inFile2;
   string file;
   string file2;

   if (argc == 2)
   {
      file = argv[1];
      file2 = argv[1];
   }
   else if (argc == 3)
   {
      file = argv[1];
      file2 = argv[2];
   }
   else if (argc >= 4)
   {
      file = argv[1];
      file2 = argv[2];
      size = atoi(argv[3]);
   }
   else 
   {
      cout << "Usage: " << argv[0] << " [file1] [file2] [size]\n";
      return 1;
   }   

   Matrix<int> matrixA(size);
   Matrix<int> matrixB(size);
   if (size < 8192)
   {
      matrixA.thread_Start = size;
      matrixA.thread_Stop = size / 4;
   }
   else
   {
      // Prevent gross inflation of memory requirements for matrices larger than 4096
      matrixA.thread_Stop = 2048;
      matrixA.thread_Start = matrixA.thread_Stop * 2;
   }
   
   bool error1;
   bool error2;
   thread m1 = thread(readMatrix<int>, std::ref(matrixA), file, std::ref(error1));
   thread m2 = thread(readMatrix<int>, std::ref(matrixB), file2, std::ref(error2));
   m1.join();
   m2.join();
   if (error1 || error2)
   {
      return 1;
   }
   
   // Perform the multiplication
   matrixA.mult_FarmSlave(matrixB);
   // Output
   cout << matrixA;

   return 0;
}
