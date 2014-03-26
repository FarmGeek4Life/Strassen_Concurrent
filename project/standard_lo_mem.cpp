/***************************************************************************
* Program:
*    Exploration 3, Implement a Divide and Conquer Algorithm
*    Brother Neff, CS 306
* Author:
*    Bryson Gibbons
* Summary:
*    Implement Strassen's Algorithm for multiplying two matrices
*    
***************************************************************************/

#include <cmath>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <thread>
#include <mutex>
using namespace std;

template <class T>
void alloc(T**& mRows, int size)
{
   mRows = new T*[size];
   for (int i = 0; i < size; i++)
   {
      mRows[i] = new T[size];
   }
}

template <class T>
void delAlloc(T** mRows, int mSize)
{
   for (int i = 0; i < mSize; i++)
   {
      delete [] mRows[i];
   }   
   delete [] mRows;
   mRows = NULL;
}

template <class T>
void read(istream& is, T** mRows, int mSize)
{
   for (int i = 0; i < mSize; i++)
   {
      for (int j = 0; j < mSize; j++)
      {
         is >> mRows[i][j];
      }      
   }
}

template <class T>
void write(ostream& os, T** mRows, int mSize)
{
   for (int i = 0; i < mSize; i++)
   {
      for (int j = 0; j < mSize; j++)
      {
         os << mRows[i][j] << " ";
      }
      os << endl;
   }
}

std::mutex resultLock;

/************************************************************************
* Low level matrix multiplier.....
***********************************************************************/
//template <class T>
//void multiply(int i, int j, int start, int stop, T** matrixA, T** matrixB, T* result)
void multiply(int i, int j, int start, int stop, int** matrixA, int** matrixB, int* result)
{
   // Initialize to known value...
   int myValue = 0;
   for (int k = start; k < stop; ++k)
   {
      myValue += matrixA[i][k] * matrixB[k][j];
   }
   std::lock_guard<std::mutex> lock(resultLock);
   *result += myValue;
}
void multiply2(int i, int j, int** matrixA, int** matrixB, int* result, int size)
{
   // Initialize to known value...
   *result = 0;
   for (int k = 0; k < size; ++k)
   {
      *result += matrixA[i][k] * matrixB[k][j];
   }
}

int main(int argc, char* argv[])
{
   int size = 32;
   ifstream inFile;
   ifstream inFile2;
   string file;
   string file2;
   string fileOut;

   if (argc == 5)
   {
      file = argv[1];
      file2 = argv[2];
      fileOut = argv[3];
      size = atoi(argv[4]);
   }
   else 
   {
      cout << "Usage: " << argv[0] << " [file1] [file2] [outFile] [size]\n";
      return 1;
   }
   int** matrixA;
   int** matrixB;
   alloc(matrixA, size);
   alloc(matrixB, size);

   inFile.open(file.c_str());
   
   if (inFile.is_open())
   {
      read(inFile, matrixA, size);
      inFile.close();
   }
   else 
   {
      cerr << "Unable to open " + file;
      return 1;
   }

   inFile2.open(file2.c_str());
   
   if (inFile2.is_open())
   {
      read(inFile2, matrixB, size);
      inFile2.close();
   }
   else
   {
      cerr << "Unable to open " + file2;
      return 1;
   }
   
   int threadCount = 128;
   int jumpVal = size / threadCount;
   ofstream fout(fileOut.c_str());
   thread t[256];
   int results[256];
   for (int i = 0; i < size; ++i)
   {
      //for (int j = 0; j < size; ++j)
      for (int j = 0; j < size; j += threadCount)
      {
         // Initialize to known value...
         int result = 0;
         //for (int k = 0; k < size; ++k)
         //{
         //   result += matrixA[i][k] * matrixB[k][j];
         //}
         int m = 0;
         // double up, and do 2 or 3 rows at a time????
         //for (int k = 0; k < size; k += jumpVal)
         for (int k = j; k < j + threadCount; ++k, ++m)
         {
            //result += matrixA[i][k] * matrixB[k][j];
            //void multiply(int i, int j, int start, int stop, T** matrixA, T** matrixB, T* result)
            //t[m++] = thread(multiply, i, j, k, k + jumpVal, matrixA, matrixB, &result);
            t[m] = thread(multiply2, i, k, matrixA, matrixB, &results[m], size);
         }
         for (int k = 0; k < threadCount; ++k)
         {
            t[k].join();
            fout << results[k] << " ";
         }
         //fout << result << " ";
      }
      fout << endl;
   }

   return 0;
}
