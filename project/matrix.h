/***************************************************************************
* Program:
*    Copied from strassen_int_opt
* Author:
*    Bryson Gibbons
* Summary:
*    Senior Project
*    
***************************************************************************/

#ifndef _MATRIX_H
#define _MATRIX_H

//#include <cmath>
#include <iostream>
//#include <fstream>
//#include <iomanip>
//#include <cstdlib>
#include <thread>
// Trying mutual exclusion....
#include <mutex>
#include <chrono>
//#include <condition_variable>
//#include <cstring>
//#include <string>
//#include <unistd.h>
#include "connection.h"
#include "errorColors.h"

//using namespace std;

template <class T>
class Matrix
{
private:
   T** mRows;
   int mSize;
   bool colAlloc;
   bool rowAlloc;
   bool finished;
   bool started;

public:
   Matrix<T>(int size, bool alloc)
   {
      if (alloc)
      {
         mRows = new T*[size];
         for (int i = 0; i < size; i++)
         {
            mRows[i] = new T[size];
         }
         colAlloc = true;
         rowAlloc = true;
      }
      else
      {
         colAlloc = false;
         rowAlloc = false;
      }
      mSize = size;
      finished = false;
      started = false;
      hasWaiting = false;
   }
   
   Matrix<T>(int size)
   {
      mRows = new T*[size];
      for (int i = 0; i < size; i++)
      {
         mRows[i] = new T[size];
      }
      mSize = size;
      colAlloc = true;
      rowAlloc = true;
      finished = false;
      started = false;
      hasWaiting = false;
   }

   Matrix<T>(const Matrix<T>& matrixB)
   {
      mSize = matrixB.getSize();
      colAlloc = true;
      rowAlloc = true;
      finished = false;
      started = false;
      hasWaiting = false;

      mRows = new T*[mSize];
      for (int i = 0; i < mSize; i++)
      {
         mRows[i] = new T[mSize];
         for (int j = 0; j < mSize; j++)
         {
            mRows[i][j] = matrixB.mRows[i][j];
         }
      }
   }
   
   /*********************************************************************
   * Allocate and fill the matrix with the specified data
   *********************************************************************/
   Matrix<T>(const Matrix<T>& matrixA, const Matrix<T>& matrixB, bool add)
   {
      mSize = matrixB.getSize();
      colAlloc = true;
      rowAlloc = true;
      finished = false;
      started = false;
      hasWaiting = false;
      
      mRows = new T*[mSize];
      if (add)
      {
         for (int i = 0; i < mSize; i++)
         {
            mRows[i] = new T[mSize];
            for (int j = 0; j < mSize; j++)
            {
               mRows[i][j] = matrixA.mRows[i][j] + matrixB.mRows[i][j];
            }
         }
      }
      else
      {
         for (int i = 0; i < mSize; i++)
         {
            mRows[i] = new T[mSize];
            for (int j = 0; j < mSize; j++)
            {
               mRows[i][j] = matrixA.mRows[i][j] - matrixB.mRows[i][j];
            }
         }
      }
   }
   
   /*********************************************************************
   * Allocate and fill the matrix with the specified data
   * Wrapper to use with threading
   *********************************************************************/
   void addFill(const Matrix<T>& matrixB, Matrix<T>*& newMatrix, bool add) const
   {
      newMatrix = new Matrix<T>(*this, matrixB, add);
   }
   
   /*********************************************************************
   * Copy Matrix
   * Wrapper to use with threading
   *********************************************************************/
   void copyTo(Matrix<T>*& newMatrix) const
   {
      newMatrix = new Matrix<T>(*this);
   }
   
   /*********************************************************************
   * Allocate and fill the matrix with the specified data
   *********************************************************************/
   void allocMath(const Matrix<T>& matrixA, const Matrix<T>& matrixB, bool add)
   {
      if (colAlloc || rowAlloc)
      {
         return;
      }
      mSize = matrixB.getSize();
      colAlloc = true;
      rowAlloc = true;
      
      mRows = new T*[mSize];
      if (add)
      {
         for (int i = 0; i < mSize; i++)
         {
            mRows[i] = new T[mSize];
            for (int j = 0; j < mSize; j++)
            {
               mRows[i][j] = matrixA.mRows[i][j] + matrixB.mRows[i][j];
            }
         }
      }
      else
      {
         for (int i = 0; i < mSize; i++)
         {
            mRows[i] = new T[mSize];
            for (int j = 0; j < mSize; j++)
            {
               mRows[i][j] = matrixA.mRows[i][j] - matrixB.mRows[i][j];
            }
         }
      }
   }
   
   /*********************************************************************
   * Allocate and fill the matrix with the specified data
   *********************************************************************/
   void allocCopy(const Matrix<T>& matrixB)
   {
      if (colAlloc || rowAlloc)
      {
         return;
      }
      mSize = matrixB.getSize();
      colAlloc = true;
      rowAlloc = true;

      mRows = new T*[mSize];
      for (int i = 0; i < mSize; i++)
      {
         mRows[i] = new T[mSize];
         for (int j = 0; j < mSize; j++)
         {
            mRows[i][j] = matrixB.mRows[i][j];
         }
      }
   }
   
   /*********************************************************************
   * Allocate and fill the matrix with the specified data
   *********************************************************************/
   void allocZero()
   {
      if (colAlloc || rowAlloc)
      {
         return;
      }
      colAlloc = true;
      rowAlloc = true;

      mRows = new T*[mSize];
      for (int i = 0; i < mSize; i++)
      {
         mRows[i] = new T[mSize];
         for (int j = 0; j < mSize; j++)
         {
            mRows[i][j] = 0;
         }
      }
   }

   Matrix<T>& operator=(const Matrix<T>& matrixB)
   {
      if (this == &matrixB)
      {
         return *this;
      }
      if (mSize != matrixB.getSize())
      {
         if (colAlloc && rowAlloc)
         {
            for (int i = 0; i < mSize; i++)
            {
               delete [] mRows[i];
            }
            delete [] mRows;
         }
         else if (colAlloc && !rowAlloc)
         {
            delete [] mRows;
         }
         
         mSize = matrixB.getSize();
         
         mRows = new T*[mSize];
         for (int i = 0; i < mSize; i++)
         {
            mRows[i] = new T[mSize];
            for (int j = 0; j < mSize; ++j)
               mRows[i][j] = matrixB.mRows[i][j];
         }
         colAlloc = true;
         rowAlloc = true;
      }
      else
      {
         for (int i = 0; i < mSize; i++)
         {
            for (int j = 0; j < mSize; ++j)
               mRows[i][j] = matrixB.mRows[i][j];
         }
      }
      finished = false;
      started = false;
      hasWaiting = false;
      return *this;
   }  
   
   /**************************************************************************
    * Destructor: Conditional on whether memory is allocated by the calling object
    *************************************************************************/
   ~Matrix<T>()
   {
      // Only delete the memory if this object allocated it
      if (colAlloc && rowAlloc)
      {
         for (int i = 0; i < mSize; i++)
         {
            delete [] mRows[i];
         }
         delete [] mRows;
      }
      else if (colAlloc && !rowAlloc)
      {
         delete [] mRows;
      }
   }
   
   /**************************************************************************
    * Erase the memory, used to reduce the amount of memory used
    *************************************************************************/
   void erase()
   {
      // Only delete the memory if this object allocated it
      if (colAlloc && rowAlloc)
      {
         for (int i = 0; i < mSize; i++)
         {
            delete [] mRows[i];
         }
         delete [] mRows;
      }
      else if (colAlloc && !rowAlloc)
      {
         delete [] mRows;
      }
      colAlloc = false;
      rowAlloc = false;
   }
   
   /**************************************************************************
    * Reallocate erased memory, used to reduce the amount of memory used
    *************************************************************************/
   void reallocate()
   {
      if (!colAlloc && !rowAlloc)
      {
         mRows = new T*[mSize];
         for (int i = 0; i < mSize; i++)
         {
            mRows[i] = new T[mSize];
         }
      }
      else if (!colAlloc && rowAlloc)
      {
         for (int i = 0; i < mSize; i++)
         {
            mRows[i] = new T[mSize];
         }
      }
      colAlloc = true;
      rowAlloc = true;
   }

   T* operator[](int row) const
   {
      return mRows[row];
   }

   int getSize() const
   {
      return mSize;
   }

   void read(std::istream& is) const
   {
      for (int i = 0; i < mSize; i++)
      {
         for (int j = 0; j < mSize; j++)
         {
            is >> mRows[i][j];
         }      
      }
   }

   void write(std::ostream& os) const
   {
      for (int i = 0; i < mSize; i++)
      {
         for (int j = 0; j < mSize; j++)
         {
            os << mRows[i][j] << " ";
         }
         os << std::endl;
      }
   }

   Matrix<T> operator+(Matrix<T> matrixB)
   {
      Matrix<T> result(mSize);
      for (int i = 0; i < mSize; i++)
      {
         for (int j = 0; j < mSize; j++)
         {
            result[i][j] = (*this)[i][j] + matrixB[i][j];
         }
      }
      return result;
   }

   Matrix<T> operator-(Matrix<T> matrixB)
   {
      Matrix<T> result(mSize);
      for (int i = 0; i < mSize; i++)
      {
         for (int j = 0; j < mSize; j++)
         {
            result[i][j] = (*this)[i][j] - matrixB[i][j];
         }
      }
      return result;
   }

   Matrix<T>& operator+=(Matrix<T> matrixB)
   {
      Matrix<T> result(mSize);
      for (int i = 0; i < mSize; i++)
      {
         for (int j = 0; j < mSize; j++)
         {
            (*this)[i][j] += matrixB[i][j];
         }
      }
      return *this;
   }

   Matrix<T>& operator-=(Matrix<T> matrixB)
   {
      for (int i = 0; i < mSize; i++)
      {
         for (int j = 0; j < mSize; j++)
         {
            (*this)[i][j] -= matrixB[i][j];
         }
      }
      return *this;
   }
   
   Matrix<T>& op00_11(const Matrix<T>& matrixA, const Matrix<T>& matrixB, const Matrix<T>& matrixC, const Matrix<T>& matrixD)
   {
      for (int i = 0; i < mSize; i++)
      {
         for (int j = 0; j < mSize; j++)
         {
            (*this)[i][j] = matrixA[i][j] + matrixB[i][j] - matrixC[i][j] + matrixD[i][j];
         }
      }
      return *this;
   }
   
   bool status(bool* check, bool desired)
   {
      return (*check == desired);
   }
   
   bool checkFinished()
   {
      return !finished;
   }
   
   //std::condition_variable mCV;
   std::mutex mMutex;
   std::mutex mCVMutex;
   std::mutex mWMutex;
   bool hasWaiting;
   
   /********************************************************************************************
   * Opportunistic math: as soon as the necessary threads complete, do the math
   ********************************************************************************************/
   //Matrix<T>& op00_11(const Matrix<T>& matrixA, const Matrix<T>& matrixB, const Matrix<T>& matrixC, const Matrix<T>& matrixD)
   //Matrix<T>& op00_11_con(const Matrix<T>& matrixA, const Matrix<T>& matrixB, const Matrix<T>& matrixC, const Matrix<T>& matrixD, std::thread* tA, std::thread* tB, std::thread* tC, std::thread* tD)
   Matrix<T>& op00_11_con(Matrix<T>& matrixA, Matrix<T>& matrixB, Matrix<T>& matrixC, Matrix<T>& matrixD, std::thread t[], int tA, int tB, int tC, int tD)
   {
      //std::chrono::milliseconds wait = std::chrono::milliseconds(1000);
      //{
      //std::unique_lock<std::mutex> lkA(matrixA.mCVMutex);
      //std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
      //matrixA.mCV.wait_until(lkA, now + wait);
      //}
      {
      std::lock_guard<std::mutex> lkA(matrixA.mMutex);
      if (t[tA].joinable())
      {
      std::cerr << "Passed wait: " << tA << "\n";
         try
         {
            t[tA].join();
         }
         catch (std::system_error &e)
         {
            std::cerr << "Caught std::system_error!\n";
         }
      }
      }
      {
      std::lock_guard<std::mutex> lkB(matrixB.mMutex);
      if (t[tB].joinable())
      {
      std::cerr << "Passed wait: " << tB << "\n";
         try
         {
            t[tB].join();
         }
         catch (std::system_error &e)
         {
            std::cerr << "Caught std::system_error!\n";
         }
      }
      }
      {
      std::lock_guard<std::mutex> lkC(matrixC.mMutex);
      if (t[tC].joinable())
      {
      std::cerr << "Passed wait: " << tC << "\n";
         try
         {
            t[tC].join();
         }
         catch (std::system_error &e)
         {
            std::cerr << "Caught std::system_error!\n";
         }
      }
      }
      {
      std::lock_guard<std::mutex> lkD(matrixD.mMutex);
      if (t[tD].joinable())
      {
      std::cerr << "Passed wait: " << tD << "\n";
         try
         {
            t[tD].join();
         }
         catch (std::system_error &e)
         {
            std::cerr << "Caught std::system_error!\n";
         }
      }
      }
      for (int i = 0; i < mSize; i++)
      {
         for (int j = 0; j < mSize; j++)
         {
            (*this)[i][j] = matrixA[i][j] + matrixB[i][j] - matrixC[i][j] + matrixD[i][j];
         }
      }
      return *this;
   }
   
   /********************************************************************************************
   * Opportunistic math: as soon as the necessary threads complete, do the math
   ********************************************************************************************/
   //Matrix<T>& op01_10(const Matrix<T>& matrixA, const Matrix<T>& matrixB)
   Matrix<T>& op01_10_con(Matrix<T>& matrixA, Matrix<T>& matrixB, std::thread t[], int tA, int tB)
   {
      //std::chrono::milliseconds wait = std::chrono::milliseconds(1000);
      //{
      //std::unique_lock<std::mutex> lkA(matrixA.mCVMutex);
      //std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
      //matrixA.mCV.wait_until(lkA, now + wait);
      //}
      {
      std::lock_guard<std::mutex> lkA(matrixA.mMutex);
      if (t[tA].joinable())
      {
      std::cerr << "Passed wait: " << tA << "\n";
         try
         {
            t[tA].join();
         }
         catch (std::system_error &e)
         {
            std::cerr << "Caught std::system_error!\n";
         }
      }
      }
      {
      std::lock_guard<std::mutex> lkB(matrixB.mMutex);
      if (t[tB].joinable())
      {
      std::cerr << "Passed wait: " << tB << "\n";
         try
         {
            t[tB].join();
         }
         catch (std::system_error &e)
         {
            std::cerr << "Caught std::system_error!\n";
         }
      }
      }
      for (int i = 0; i < mSize; i++)
      {
         for (int j = 0; j < mSize; j++)
         {
            (*this)[i][j] = matrixA[i][j] + matrixB[i][j];
         }
      }
      return *this;
   }
   
   Matrix<T>& op01_10(const Matrix<T>& matrixA, const Matrix<T>& matrixB)
   {
      for (int i = 0; i < mSize; i++)
      {
         for (int j = 0; j < mSize; j++)
         {
            (*this)[i][j] = matrixA[i][j] + matrixB[i][j];
         }
      }
      return *this;
   }
   
   /**************************************************************************
    * Constructor to build a matrix from quadrants
    *************************************************************************/
   Matrix<T>(const Matrix<T>& copy00, const Matrix<T>& copy01, const Matrix<T>& copy10, const Matrix<T>& copy11)
   {
      // Assume that all parameters matrices are equal in size
      mSize = copy00.mSize * 2;
      
      mRows = new T*[mSize];
      // Calculate half of the size to save iterations and operations
      int h = mSize / 2;
      for (int i = 0; i < h; i++)
      {
         // Allocate memory for two rows each time
         // Start at the top row of each half
         mRows[i] = new T[mSize];
         mRows[i + h] = new T[mSize];
         for (int j = 0; j < h; j++)
         {
            // Use i, j, and h to copy 1 value from each quadrant to 
            //    the correct position with each iteration
            mRows[i][j] = copy00.mRows[i][j];
            mRows[i][j + h] = copy01.mRows[i][j];
            mRows[i + h][j] = copy10.mRows[i][j];
            mRows[i + h][j + h] = copy11.mRows[i][j];
         }
      }
      colAlloc = true;
      rowAlloc = true;
   }
   
   /**************************************************************************
    * Get the specified quadrant of the Matrix
    *************************************************************************/
   Matrix<T> getQuadrant(int row, int col) const
   {
      Matrix<T> result(mSize / 2);
      int rRow = 0;
      int rCol = 0;
      // Calculate the quadrant index limits based off of the input row and col
      int qRowMin = row * (mSize / 2);
      int qRowMax = ((row + 1) * (mSize / 2));
      int qColMin = col * (mSize / 2);
      int qColMax = ((col + 1) * (mSize / 2));
      
      // Copy data from the row, col quadrant to result.
      for (int qRow = qRowMin; qRow < qRowMax; ++qRow, ++rRow)
      {
         // Make sure to reset rCol for each row.
         rCol = 0;
         for (int qCol = qColMin; qCol < qColMax; ++qCol, ++rCol)
         {
            result[rRow][rCol] = mRows[qRow][qCol];
         }
      }
      return result;
   }
   
   /**************************************************************************
    * Get the specified quadrant of the Matrix
    * This uses the pointers to the values in the original matrix;
    * Editing this matrix will also edit the specific quadrant of the original
    *************************************************************************/
   Matrix<T>(const Matrix<T>& original, bool row, bool col)
   {
      int origSize = original.mSize;
      mSize = origSize / 2;
      
      int rRow = 0;
      int rCol = 0;
      // Calculate the quadrant index limits based off of the input row and col
      int qRowMin = row * (origSize / 2);
      int qRowMax = ((row + 1) * (origSize / 2));
      int qColMin = col * (origSize / 2);
      int qColMax = ((col + 1) * (origSize / 2));
      
      if (col)
      {
         colAlloc = true;
         mRows = new T*[mSize];
         // Copy pointers from the beginning of the right half of the top/bottom half...
         for (int qRow = qRowMin; qRow < qRowMax; ++qRow, ++rRow)
         {
            mRows[rRow] = &(original[qRow][qColMin]);
         }
      }
      else
      {
         colAlloc = false;
         mRows = &(original.mRows[qRowMin]);
      }
      rowAlloc = false;
   }
   
   // Output the entire matrix on error:
   static bool NetError;
   // Starting point for thread creation
   static int thread_Start;
   // Stopping point for thread creation
   static int thread_Stop;
   static int sysCounter;
   static std::mutex sysCounter_Mutex;
   // Limit the number of simultaneous threads
   static std::mutex threadLimiter[100];
   static int maxThreads;
   
   /**************************************************************************
    * this: m-matrix
    * need to pass in a*, a*, addA, b*, b*, addB
    * 
    *************************************************************************/
   void mult_Fast_Farm(Matrix<T>* a0, Matrix<T>* a1, bool addA, Matrix<T>* b0, Matrix<T>* b1, bool addB, std::string computers[], int numComputers, std::string port)
   {
      // Take over the lock for this matrix.....
      std::lock_guard<std::mutex> lk(mMutex);
      started = true;
      // Allocate the memory and fill it with the specified data...
      if (a1 != NULL)
      {
         this->allocMath(*a0, *a1, addA);
      }
      else
      {
         //*this = *a0;
         this->allocCopy(*a0);
      }
      Matrix<T> matrixB(mSize, false); // Declare but don't allocate
      if (b1 != NULL)
      {
         matrixB.allocMath(*b0, *b1, addB); // Allocate and compute matrixB
      }
      else
      {
         //matrixB = *b0;
         matrixB.allocCopy(*b0);
      }
      //mult_ThreadFarming(matrixB, NULL, computers, numComputers, port, true);
      mult_ThreadFarming(matrixB, computers, numComputers, port, true);
   }
   
   
   //void mult_FarmWrapper(Matrix<T> matrixB, Matrix<T>* result, std::string computers[], int numComputers, std::string port, bool wrapped = false)
   void mult_FarmWrapper(Matrix<T> matrixB, std::string computers[], int numComputers, std::string port, bool wrapped = false)
   {
      //this->mult_ThreadFarming(matrixB, result, computers, numComputers, port, wrapped);
      this->mult_ThreadFarming(matrixB, computers, numComputers, port, wrapped);
   }
   
   /**************************************************************************
    * Matrix multiplication using Strassen's algorithm.
    * The input matrices must be equal in size and square, 
    *    and the size must be n x n, where n is a power of 2
    *************************************************************************/
   //void mult_ThreadFarming(Matrix<T>& matrixB, Matrix<T>* result, std::string computers[], int numComputers, std::string port, bool wrapped = false)
   void mult_ThreadFarming(Matrix<T>& matrixB, std::string computers[], int numComputers, std::string port, bool wrapped = false)
   {
      // Take over the lock for this matrix.....
      std::mutex* pMutex = &mMutex;
      if (wrapped)
      {
         std::cerr << "TF: using wrapper lock!\n";
         pMutex = &mWMutex;
      }
      else
      {
         std::cerr << "TF: Grabbing object lock!\n";
      }
      //std::lock_guard<std::mutex> lk(mMutex);
      std::lock_guard<std::mutex> lk(*pMutex);
      started = true;
      if (mSize > 1)
      {
         std::thread t[16];
         
         // Four quadrants for each matrix being multiplied
         Matrix<T> a00(*this, 0, 0);
         Matrix<T> a01(*this, 0, 1);
         Matrix<T> a10(*this, 1, 0);
         Matrix<T> a11(*this, 1, 1);
         Matrix<T> b00(matrixB, 0, 0);
         Matrix<T> b01(matrixB, 0, 1);
         Matrix<T> b10(matrixB, 1, 0);
         Matrix<T> b11(matrixB, 1, 1);
         
         // Temporary Matrices to hold the 7 multiplication results
         // Initialize to the left side of the multiplication...
         /**/
         //Matrix<T> m1(a00, a11, true ); // Create new object, adding 2nd to 1st
         //Matrix<T> m2(a10, a11, true ); // Create new object, adding 2nd to 1st
         //Matrix<T> m3(a00            ); // Make a copy...
         //Matrix<T> m4(a11            ); // Make a copy...
         //Matrix<T> m5(a00, a01, true ); // Create new object, adding 2nd to 1st
         //Matrix<T> m6(a10, a00, false); // Create new object, subtracting 2nd from 1st
         //Matrix<T> m7(a01, a11, false); // Create new object, subtracting 2nd from 1st
         Matrix<T> m1(mSize / 2, false); // Declare matrix, but do not allocate
         Matrix<T> m2(mSize / 2, false); // Declare matrix, but do not allocate
         Matrix<T> m3(mSize / 2, false); // Declare matrix, but do not allocate
         Matrix<T> m4(mSize / 2, false); // Declare matrix, but do not allocate
         Matrix<T> m5(mSize / 2, false); // Declare matrix, but do not allocate
         Matrix<T> m6(mSize / 2, false); // Declare matrix, but do not allocate
         Matrix<T> m7(mSize / 2, false); // Declare matrix, but do not allocate
         Matrix<int>* null = NULL;
         //a00.erase();
         //a01.erase();
         //a10.erase();
         //a11.erase();
         //this->erase(); // Must reallocate later...
         /*/
         Matrix<T> *m1;
         Matrix<T> *m2;
         Matrix<T> *m3;
         Matrix<T> *m4;
         Matrix<T> *m5;
         Matrix<T> *m6;
         Matrix<T> *m7;
         Matrix<T> *b1;
         Matrix<T> *b2;
         Matrix<T> *b3;
         Matrix<T> *b4;
         Matrix<T> *b5;
         Matrix<T> *b6;
         Matrix<T> *b7;
         Matrix<int>* null = NULL;
         
         if (mSize < 4096)
         {
            m1 = new Matrix<T>(a00, a11, true ); // Create new object, adding 2nd to 1st
            m2 = new Matrix<T>(a10, a11, true ); // Create new object, adding 2nd to 1st
            m3 = new Matrix<T>(a00            ); // Make a copy...
            m4 = new Matrix<T>(a11            ); // Make a copy...
            m5 = new Matrix<T>(a00, a01, true ); // Create new object, adding 2nd to 1st
            m6 = new Matrix<T>(a10, a00, false); // Create new object, subtracting 2nd from 1st
            m7 = new Matrix<T>(a01, a11, false); // Create new object, subtracting 2nd from 1st
            b1 = new Matrix<T>(b00, b11, true ); // Create new object, adding 2nd to 1st
            b2 = new Matrix<T>(b00            ); // Make a copy... -----------------------------Should be able to get away with a pointer copy... - not with the memory deallocation that follows...
            b3 = new Matrix<T>(b01, b11, false); // Create new object, subtracting 2nd from 1st
            b4 = new Matrix<T>(b10, b00, false); // Create new object, adding 2nd to 1st
            b5 = new Matrix<T>(b11            ); // Make a copy... -----------------------------Should be able to get away with a pointer copy... - not with the memory deallocation that follows...
            b6 = new Matrix<T>(b00, b01, true ); // Create new object, subtracting 2nd from 1st
            b7 = new Matrix<T>(b10, b11, true ); // Create new object, adding 2nd to 1st
         }
         else
         {
            t[1]  = std::thread(&Matrix<T>::addFill, &a00, std::ref(a11), std::ref(m1), true );
            t[2]  = std::thread(&Matrix<T>::addFill, &a10, std::ref(a11), std::ref(m2), true );
            t[3]  = std::thread(&Matrix<T>::copyTo , &a00               , std::ref(m3)       );
            t[4]  = std::thread(&Matrix<T>::copyTo , &a11               , std::ref(m4)       );
            t[5]  = std::thread(&Matrix<T>::addFill, &a00, std::ref(a01), std::ref(m5), true );
            t[6]  = std::thread(&Matrix<T>::addFill, &a10, std::ref(a00), std::ref(m6), false);
            t[7]  = std::thread(&Matrix<T>::addFill, &a01, std::ref(a11), std::ref(m7), false);
            
            t[8]  = std::thread(&Matrix<T>::addFill, &b00, std::ref(b11), std::ref(b1), true );
            t[9]  = std::thread(&Matrix<T>::copyTo , &b00               , std::ref(b2)       );
            t[10] = std::thread(&Matrix<T>::addFill, &b01, std::ref(b11), std::ref(b3), false);
            t[11] = std::thread(&Matrix<T>::addFill, &b10, std::ref(b00), std::ref(b4), false);
            t[12] = std::thread(&Matrix<T>::copyTo , &b11               , std::ref(b5)       );
            t[13] = std::thread(&Matrix<T>::addFill, &b00, std::ref(b01), std::ref(b6), true );
            t[14] = std::thread(&Matrix<T>::addFill, &b10, std::ref(b11), std::ref(b7), true );
            
            t[1] .join();
            t[2] .join();
            t[3] .join();
            t[4] .join();
            t[5] .join();
            t[6] .join();
            t[7] .join();
            t[8] .join();
            t[9] .join();
            t[10].join();
            t[11].join();
            t[12].join();
            t[13].join();
            t[14].join();
            //for (int i = 1; i < 15; ++i)
            //{
            //   t[i].join();
            //}
         }
         // Clear out allocated memory....
         b00.erase();
         b01.erase();
         b10.erase();
         b11.erase();
         // We don't need matrixB data anymore. Erase it.
         matrixB.erase();
         /**/
         
         // Get the 7 multiplication results
         // m1 = (a00 + a11) * (b00 + b11);
         // m2 = (a10 + a11) *  b00;
         // m3 =  a00 *        (b01 - b11);
         // m4 =  a11 *        (b10 - b00);
         // m5 = (a00 + a01) *  b11;
         // m6 = (a10 - a00) * (b00 + b01);
         // m7 = (a01 - a11) * (b10 + b11);
         //{
         //std::lock_guard<std::mutex> lock(threadLimiter[0]);
         // Split for the thread number optimization
         // It makes no sense to split into smaller chunks for 7 computers
         if (mSize > thread_Stop && numComputers != 7 && numComputers != 1)
         {
            /*/
            t[1] = std::thread(&Matrix<T>::mult_FarmWrapper, &m1, (b00 + b11), null, computers, numComputers, port, false);
            t[2] = std::thread(&Matrix<T>::mult_FarmWrapper, &m2, (b00)      , null, computers, numComputers, port, false);
            t[3] = std::thread(&Matrix<T>::mult_FarmWrapper, &m3, (b01 - b11), null, computers, numComputers, port, false);
            t[4] = std::thread(&Matrix<T>::mult_FarmWrapper, &m4, (b10 - b00), null, computers, numComputers, port, false);
            t[5] = std::thread(&Matrix<T>::mult_FarmWrapper, &m5, (b11)      , null, computers, numComputers, port, false);
            t[6] = std::thread(&Matrix<T>::mult_FarmWrapper, &m6, (b00 + b01), null, computers, numComputers, port, false);
            t[7] = std::thread(&Matrix<T>::mult_FarmWrapper, &m7, (b10 + b11), null, computers, numComputers, port, false);
            //t[1] = std::thread(&Matrix<T>::mult_ThreadFarming, &m1, (b00 + b11), null, computers, numComputers, port);
            //t[2] = std::thread(&Matrix<T>::mult_ThreadFarming, &m2, (b00)      , null, computers, numComputers, port);
            //t[3] = std::thread(&Matrix<T>::mult_ThreadFarming, &m3, (b01 - b11), null, computers, numComputers, port);
            //t[4] = std::thread(&Matrix<T>::mult_ThreadFarming, &m4, (b10 - b00), null, computers, numComputers, port);
            //t[5] = std::thread(&Matrix<T>::mult_ThreadFarming, &m5, (b11)      , null, computers, numComputers, port);
            //t[6] = std::thread(&Matrix<T>::mult_ThreadFarming, &m6, (b00 + b01), null, computers, numComputers, port);
            //t[7] = std::thread(&Matrix<T>::mult_ThreadFarming, &m7, (b10 + b11), null, computers, numComputers, port);
            /*/
            //t[1] = std::thread(&Matrix<T>::mult_ThreadFarming, &(*m1), std::ref(*b1), null, computers, numComputers, port);
            //t[2] = std::thread(&Matrix<T>::mult_ThreadFarming, &(*m2), std::ref(*b2), null, computers, numComputers, port);
            //t[3] = std::thread(&Matrix<T>::mult_ThreadFarming, &(*m3), std::ref(*b3), null, computers, numComputers, port);
            //t[4] = std::thread(&Matrix<T>::mult_ThreadFarming, &(*m4), std::ref(*b4), null, computers, numComputers, port);
            //t[5] = std::thread(&Matrix<T>::mult_ThreadFarming, &(*m5), std::ref(*b5), null, computers, numComputers, port);
            //t[6] = std::thread(&Matrix<T>::mult_ThreadFarming, &(*m6), std::ref(*b6), null, computers, numComputers, port);
            //t[7] = std::thread(&Matrix<T>::mult_ThreadFarming, &(*m7), std::ref(*b7), null, computers, numComputers, port);
            t[1] = std::thread(&Matrix<T>::mult_Fast_Farm, &m1, &a00, &a11 , true , &b00, &b11 , true , computers, numComputers, port);
            t[2] = std::thread(&Matrix<T>::mult_Fast_Farm, &m2, &a10, &a11 , true , &b00,  null, false, computers, numComputers, port);
            t[3] = std::thread(&Matrix<T>::mult_Fast_Farm, &m3, &a00,  null, false, &b01, &b11 , false, computers, numComputers, port);
            t[4] = std::thread(&Matrix<T>::mult_Fast_Farm, &m4, &a11,  null, false, &b10, &b00 , false, computers, numComputers, port);
            t[5] = std::thread(&Matrix<T>::mult_Fast_Farm, &m5, &a00, &a01 , true , &b11,  null, false, computers, numComputers, port);
            t[6] = std::thread(&Matrix<T>::mult_Fast_Farm, &m6, &a10, &a00 , false, &b00, &b01 , true , computers, numComputers, port);
            t[7] = std::thread(&Matrix<T>::mult_Fast_Farm, &m7, &a01, &a11 , false, &b10, &b11 , true , computers, numComputers, port);
            /**/
         }
         else
         {
            // Mutex: control the access to the system counter (sysCounter) for even distribution
            std::lock_guard<std::mutex> lock(sysCounter_Mutex);
            /*/
            t[1] = std::thread(&Matrix<T>::runParallel, &m1, (b00 + b11), null, computers[sysCounter % numComputers], port, sysCounter); ++sysCounter;
            t[2] = std::thread(&Matrix<T>::runParallel, &m2, (b00)      , null, computers[sysCounter % numComputers], port, sysCounter); ++sysCounter;
            t[3] = std::thread(&Matrix<T>::runParallel, &m3, (b01 - b11), null, computers[sysCounter % numComputers], port, sysCounter); ++sysCounter;
            t[4] = std::thread(&Matrix<T>::runParallel, &m4, (b10 - b00), null, computers[sysCounter % numComputers], port, sysCounter); ++sysCounter;
            t[5] = std::thread(&Matrix<T>::runParallel, &m5, (b11)      , null, computers[sysCounter % numComputers], port, sysCounter); ++sysCounter;
            t[6] = std::thread(&Matrix<T>::runParallel, &m6, (b00 + b01), null, computers[sysCounter % numComputers], port, sysCounter); ++sysCounter;
            t[7] = std::thread(&Matrix<T>::runParallel, &m7, (b10 + b11), null, computers[sysCounter % numComputers], port, sysCounter); ++sysCounter;
            /*/
            //t[1] = std::thread(&Matrix<T>::runParallel, &(*m1), std::ref(*b1), null, computers[sysCounter % numComputers], port, sysCounter); ++sysCounter;
            //t[2] = std::thread(&Matrix<T>::runParallel, &(*m2), std::ref(*b2), null, computers[sysCounter % numComputers], port, sysCounter); ++sysCounter;
            //t[3] = std::thread(&Matrix<T>::runParallel, &(*m3), std::ref(*b3), null, computers[sysCounter % numComputers], port, sysCounter); ++sysCounter;
            //t[4] = std::thread(&Matrix<T>::runParallel, &(*m4), std::ref(*b4), null, computers[sysCounter % numComputers], port, sysCounter); ++sysCounter;
            //t[5] = std::thread(&Matrix<T>::runParallel, &(*m5), std::ref(*b5), null, computers[sysCounter % numComputers], port, sysCounter); ++sysCounter;
            //t[6] = std::thread(&Matrix<T>::runParallel, &(*m6), std::ref(*b6), null, computers[sysCounter % numComputers], port, sysCounter); ++sysCounter;
            //t[7] = std::thread(&Matrix<T>::runParallel, &(*m7), std::ref(*b7), null, computers[sysCounter % numComputers], port, sysCounter); ++sysCounter;
            t[1] = std::thread(&Matrix<T>::runParallel, &m1, &a00, &a11 , true , &b00, &b11 , true , computers[sysCounter % numComputers], port, sysCounter); ++sysCounter;
            t[2] = std::thread(&Matrix<T>::runParallel, &m2, &a10, &a11 , true , &b00,  null, false, computers[sysCounter % numComputers], port, sysCounter); ++sysCounter;
            t[3] = std::thread(&Matrix<T>::runParallel, &m3, &a00,  null, false, &b01, &b11 , false, computers[sysCounter % numComputers], port, sysCounter); ++sysCounter;
            t[4] = std::thread(&Matrix<T>::runParallel, &m4, &a11,  null, false, &b10, &b00 , false, computers[sysCounter % numComputers], port, sysCounter); ++sysCounter;
            t[5] = std::thread(&Matrix<T>::runParallel, &m5, &a00, &a01 , true , &b11,  null, false, computers[sysCounter % numComputers], port, sysCounter); ++sysCounter;
            t[6] = std::thread(&Matrix<T>::runParallel, &m6, &a10, &a00 , false, &b00, &b01 , true , computers[sysCounter % numComputers], port, sysCounter); ++sysCounter;
            t[7] = std::thread(&Matrix<T>::runParallel, &m7, &a01, &a11 , false, &b10, &b11 , true , computers[sysCounter % numComputers], port, sysCounter); ++sysCounter;
            /**/
         }
         // Clear out allocated memory....
         //b00.erase();
         //b01.erase();
         //b10.erase();
         //b11.erase();
         //// Also include matrixB
         //matrixB.erase();
         //}
         //t[1].join();
         //t[2].join();
         //t[3].join();
         //t[4].join();
         //t[5].join();
         //t[6].join();
         //t[7].join();
         
         //b1->erase();
         //b2->erase();
         //b3->erase();
         //b4->erase();
         //b5->erase();
         //b6->erase();
         //b7->erase();
         
         //this->reallocate(); // Reallocate the memory...
         //Matrix<T> c00(*this, 0, 0);
         //Matrix<T> c01(*this, 0, 1);
         //Matrix<T> c10(*this, 1, 0);
         //Matrix<T> c11(*this, 1, 1);
         
         // Use the 7 multiplication results to get the results for each quadrant
         // Save on memory usage by reusing one set of quadrants
         // Also saves on time - no de/reallocation
         //a00 = m1 + m4 - m5 + m7;
         //a01 = m3 + m5;
         //a10 = m2 + m4;
         //a11 = m1 + m3 - m2 + m6;
         /*/
         a00.op00_11(m1, m4, m5, m7);
         a01.op01_10(m3, m5);
         a10.op01_10(m2, m4);
         a11.op00_11(m1, m3, m2, m6);
         /*/
         //if (mSize < 2048)
         //{
         //   //a00.op00_11(m1, m4, m5, m7);
         //   //a01.op01_10(m3, m5);
         //   //a10.op01_10(m2, m4);
         //   //a11.op00_11(m1, m3, m2, m6);
         //   //t[1].join();
         //   //t[4].join();
         //   //t[5].join();
         //   //t[7].join();
         //   //c00.op00_11(m1, m4, m5, m7);
         //   //t[3].join();
         //   //c01.op01_10(m3, m5);
         //   //t[2].join();
         //   //c10.op01_10(m2, m4);
         //   //t[6].join();
         //   //c11.op00_11(m1, m3, m2, m6);
         //   //c00.op00_11_con(m1, m4, m5, m7, &t[1], &t[4], &t[5], &t[7]);
         //   //c01.op01_10_con(m3, m5, &t[3], &t[5]);
         //   //c10.op01_10_con(m2, m4, &t[2], &t[4]);
         //   //c11.op00_11_con(m1, m3, m2, m6, &t[1], &t[3], &t[2], &t[6]);
         //   //c00.op00_11_con(m1, m4, m5, m7, t, 1, 4, 5, 7);
         //   //c01.op01_10_con(m3, m5, t, 3, 5);
         //   //c10.op01_10_con(m2, m4, t, 2, 4);
         //   //c11.op00_11_con(m1, m3, m2, m6, t, 1, 3, 2, 6);
         //   a00.op00_11_con(m1, m4, m5, m7, t, 1, 4, 5, 7);
         //   a01.op01_10_con(m3, m5, t, 3, 5);
         //   a10.op01_10_con(m2, m4, t, 2, 4);
         //   a11.op00_11_con(m1, m3, m2, m6, t, 1, 3, 2, 6);
         //   //a00.op00_11(*m1, *m4, *m5, *m7);
         //   //a01.op01_10(*m3, *m5);
         //   //a10.op01_10(*m2, *m4);
         //   //a11.op00_11(*m1, *m3, *m2, *m6);
         //}
         //else
         //{
            //t[1] = std::thread(&Matrix<T>::op00_11, &a00, m1, m4, m5, m7);
            //t[2] = std::thread(&Matrix<T>::op01_10, &a01, m3, m5);
            //t[3] = std::thread(&Matrix<T>::op01_10, &a10, m2, m4);
            //t[4] = std::thread(&Matrix<T>::op00_11, &a11, m1, m3, m2, m6);
            //t[1].join();
            //t[4].join();
            //t[5].join();
            //t[7].join();
            //t[12] = std::thread(&Matrix<T>::op00_11, &c00, m1, m4, m5, m7);
            //t[3].join();
            //t[13] = std::thread(&Matrix<T>::op01_10, &c01, m3, m5);
            //t[2].join();
            //t[14] = std::thread(&Matrix<T>::op01_10, &c10, m2, m4);
            //t[6].join();
            //t[15] = std::thread(&Matrix<T>::op00_11, &c11, m1, m3, m2, m6);
            //t[12] = std::thread(&Matrix<T>::op00_11_con, &c00, m1, m4, m5, m7, &t[1], &t[4], &t[5], &t[7]);
            //t[13] = std::thread(&Matrix<T>::op01_10_con, &c01, m3, m5, &t[3], &t[5]);
            //t[14] = std::thread(&Matrix<T>::op01_10_con, &c10, m2, m4, &t[2], &t[4]);
            //t[15] = std::thread(&Matrix<T>::op00_11_con, &c11, m1, m3, m2, m6, &t[1], &t[3], &t[2], &t[6]);
            //t[12] = std::thread(&Matrix<T>::op00_11_con, &c00, std::ref(m1), std::ref(m4), std::ref(m5), std::ref(m7), t, 1, 4, 5, 7);
            //t[13] = std::thread(&Matrix<T>::op01_10_con, &c01, std::ref(m3), std::ref(m5), t, 3, 5);
            //t[14] = std::thread(&Matrix<T>::op01_10_con, &c10, std::ref(m2), std::ref(m4), t, 2, 4);
            //t[15] = std::thread(&Matrix<T>::op00_11_con, &c11, std::ref(m1), std::ref(m3), std::ref(m2), std::ref(m6), t, 1, 3, 2, 6);
            t[12] = std::thread(&Matrix<T>::op00_11_con, &a00, std::ref(m1), std::ref(m4), std::ref(m5), std::ref(m7), t, 1, 4, 5, 7);
            t[13] = std::thread(&Matrix<T>::op01_10_con, &a01, std::ref(m3), std::ref(m5), t, 3, 5);
            t[14] = std::thread(&Matrix<T>::op01_10_con, &a10, std::ref(m2), std::ref(m4), t, 2, 4);
            t[15] = std::thread(&Matrix<T>::op00_11_con, &a11, std::ref(m1), std::ref(m3), std::ref(m2), std::ref(m6), t, 1, 3, 2, 6);
            //t[1] = std::thread(&Matrix<T>::op00_11, &a00, *m1, *m4, *m5, *m7);
            //t[2] = std::thread(&Matrix<T>::op01_10, &a01, *m3, *m5);
            //t[3] = std::thread(&Matrix<T>::op01_10, &a10, *m2, *m4);
            //t[4] = std::thread(&Matrix<T>::op00_11, &a11, *m1, *m3, *m2, *m6);
            t[12].join();
            t[13].join();
            t[14].join();
            t[15].join();
         //}
         //delete m1;
         //delete m2;
         //delete m3;
         //delete m4;
         //delete m5;
         //delete m6;
         //delete m7;
         /**/
         //a00 = m1;
         //a00 += m4;
         //a00 -= m5;
         //a00 += m7;
         //a01 = m3;
         //a01 += m5;
         //a10 = m2;
         //a10 += m4;
         //a11 = m1;
         //a11 += m3;
         //a11 -= m2;
         //a11 += m6;
         // The above will re-write matrixA (calling object)
         // Reassemble the quadrants into a single whole
         //if (result != NULL)
         //{
         //   *result = Matrix(a00, a01, a10, a11);
         //}
      }
      else
      {
         // Assume a matrix of size 1
         //if (result != NULL)
         //{
         //   *result[0][0] = mRows[0][0] * matrixB[0][0];
         //}
         mRows[0][0] = mRows[0][0] * matrixB[0][0];
      }
      //std::lock_guard<std::mutex> lk(mMutex);
      finished = true;
      //mCV.notify_all();
   }
   
   /**************************************************************************
    * Matrix multiplication using Strassen's algorithm.
    * The input matrices must be equal in size and square, 
    *    and the size must be n x n, where n is a power of 2
    *************************************************************************/
   //void runParallel(Matrix<T> matrixB, Matrix<T>& result, std::string computer, std::string port, int id)
   //void runParallel(Matrix<T> matrixB, Matrix<T>* result, std::string computer, std::string port, int id)
   void runParallel(Matrix<T>* a0, Matrix<T>* a1, bool addA, Matrix<T>* b0, Matrix<T>* b1, bool addB, std::string computer, std::string port, int id)
   {
      // Allocate the memory and fill it with the specified data...
      if (a1 != NULL)
      {
         this->allocMath(*a0, *a1, addA);
      }
      else
      {
         //*this = *a0;
         this->allocCopy(*a0);
      }
      Matrix<T> matrixB(mSize, false); // Declare but don't allocate
      if (b1 != NULL)
      {
         matrixB.allocMath(*b0, *b1, addB); // Allocate and compute matrixB
      }
      else
      {
         //matrixB = *b0;
         matrixB.allocCopy(*b0);
      }
      
      // Take over the lock for this matrix.....
      //std::lock_guard<std::mutex> lk(mMutex);
      //started = true;
      
      // Set a limit on how many concurrent threads can run....
      std::lock_guard<std::mutex> lock(threadLimiter[id % maxThreads]);
      if (mSize > 1)
      {
         std::cerr << UPur << "STARTING THREAD " << id << " FOR SYSTEM '" << computer << "'!!!" << RCol << "\n";
         Connection net;
         if (net.clientSetup(computer.c_str(), port.c_str()))
         {
            bool failed = false;
            int controlData[5] = {id, mSize, 0, 0, 0};
            int close[5] = {0, 0, 0, 0, 0};
            // All data must be sent as pointers or arrays!!!
            // SHOULD PROBABLY CHANGE TO THROWING AND CATCHING ERRORS!!!!!
            // Send thread id (for debugging purposes)
            if (!net.sendData(&controlData, 4 * 5))
            {
               std::cerr << Red << "Server closed connection: " << computer << "\n";
               std::cerr << "ERROR (" << computer << "): " << net.strError << RCol << "\n";
               failed = true;
            }
            // Send matrix A
            if (failed || !this->writeNet(net))
            {
               std::cerr << Red << "Server closed connection: " << computer << "\n";
               std::cerr << "ERROR (" << computer << "): " << net.strError << RCol << "\n";
               failed = true;
            }
            // Send matrix B
            if (failed || !matrixB.writeNet(net))
            {
               std::cerr << Red << "Server closed connection: " << computer << "\n";
               std::cerr << "ERROR (" << computer << "): " << net.strError << RCol << "\n";
               failed = true;
            }
            // Erase the allocated memory of matrixB. It's not needed anymore.
            matrixB.erase();
            // Receive Result
            //if (failed || !resultreadNet(net))
            //if (failed || (result == NULL ? !this->readNet(net) : !result->readNet(net)))
            if (failed || !this->readNet(net))
            {
               std::cerr << Red << "Server closed connection: " << computer << "\n";
               std::cerr << "ERROR (" << computer << "): " << net.strError << RCol << "\n";
               failed = true;
            }
            // Close the connection cleanly
            net.closeComm();
            if (failed)
            {
               NetError = true;
            }
         }
         else
         {
            // Report error....
            std::cerr << Red << "ERROR: Network connection failed!!!!: " << net.strError << RCol << "\n\n";
            NetError = true;
         }
      }
      else
      {
         // Assume a matrix of size 1
         //result[0][0] = mRows[0][0] * matrixB[0][0];
         //if (result != NULL)
         //{
         //   *result[0][0] = mRows[0][0] * matrixB[0][0];
         //}
         mRows[0][0] = mRows[0][0] * matrixB[0][0];
      }
      //std::lock_guard<std::mutex> lk(mMutex);
      finished = true;
      //mCV.notify_all();
      std::cerr << Gre << "EXITING THREAD " << id << " FOR SYSTEM '" << computer << "'!!!" << RCol  << "\n";
   }
   
   /**************************************************************************
    * this: m-matrix
    * need to pass in a*, a*, addA, b*, b*, addB
    * 
    *************************************************************************/
   //void mult_Fast_Slave(Matrix<T>* a0, Matrix<T>* a1, bool addA, Matrix<T>* b0, Matrix<T>* b1, bool addB, std::unique_lock<std::mutex>&& ctlMut)
   void mult_Fast_Slave(Matrix<T>* a0, Matrix<T>* a1, bool addA, Matrix<T>* b0, Matrix<T>* b1, bool addB)
   {
      // Take over the lock for this matrix.....
      std::lock_guard<std::mutex> lk(mMutex);
      //ctlMut.unlock();
      
      started = true;
      // Allocate the memory and fill it with the specified data...
      if (a1 != NULL)
      {
         this->allocMath(*a0, *a1, addA);
      }
      else
      {
         //*this = *a0;
         this->allocCopy(*a0);
      }
      Matrix<T> matrixB(mSize, false); // Declare but don't allocate
      if (b1 != NULL)
      {
         matrixB.allocMath(*b0, *b1, addB); // Allocate and compute matrixB
      }
      else
      {
         //matrixB = *b0;
         matrixB.allocCopy(*b0);
      }
      //mult_ThreadFarming(matrixB, NULL, computers, numComputers, port, true);
      mult_FarmSlave(matrixB, true);
   }
   /**************************************************************************
    * Matrix multiplication using Strassen's algorithm.
    * The input matrices must be equal in size and square, 
    *    and the size must be n x n, where n is a power of 2
    *************************************************************************/
   //void mult_FarmSlave(Matrix<T>& matrixB, Matrix<T>* result)
   //void mult_FarmSlave(Matrix<T>& matrixB)
   void mult_FarmSlave(Matrix<T>& matrixB, bool wrapped = false)
   {
      // Take over the lock for this matrix.....
      //mMutex.try_lock();
      if (!mMutex.try_lock() && !wrapped)
      {
         std::cerr << Red << "FS: FAILED TO LOCK!!!!!" << RCol << std::endl;
      }
      std::mutex* pMutex = &mMutex;
      if (wrapped)
      {
         std::cerr << "FS: using wrapper lock!\n";
         pMutex = &mWMutex;
      }
      else
      {
         mMutex.unlock(); // Allow the lock_guard to work.
         std::cerr << "FS: Grabbing object lock!\n";
      }
      //std::lock_guard<std::mutex> lk(mMutex);
      std::lock_guard<std::mutex> lk(*pMutex, std::adopt_lock);
      started = true;
      if (mSize > 1)
      {
         std::thread t[16];
         
         // Four quadrants for each matrix being multiplied
         Matrix<T> a00(*this, 0, 0);
         Matrix<T> a01(*this, 0, 1);
         Matrix<T> a10(*this, 1, 0);
         Matrix<T> a11(*this, 1, 1);
         Matrix<T> b00(matrixB, 0, 0);
         Matrix<T> b01(matrixB, 0, 1);
         Matrix<T> b10(matrixB, 1, 0);
         Matrix<T> b11(matrixB, 1, 1);
         
         // Temporary Matrices to hold the 7 multiplication results
         // Initialize to the left side of the multiplication...
         /**/
         //Matrix<T> m1(a00, a11, true ); // Create new object, adding 2nd to 1st
         //Matrix<T> m2(a10, a11, true ); // Create new object, adding 2nd to 1st
         //Matrix<T> m3(a00            ); // Make a copy...
         //Matrix<T> m4(a11            ); // Make a copy...
         //Matrix<T> m5(a00, a01, true ); // Create new object, adding 2nd to 1st
         //Matrix<T> m6(a10, a00, false); // Create new object, subtracting 2nd from 1st
         //Matrix<T> m7(a01, a11, false); // Create new object, subtracting 2nd from 1st
         Matrix<T> m1(mSize / 2, false); // Declare matrix, but do not allocate
         Matrix<T> m2(mSize / 2, false); // Declare matrix, but do not allocate
         Matrix<T> m3(mSize / 2, false); // Declare matrix, but do not allocate
         Matrix<T> m4(mSize / 2, false); // Declare matrix, but do not allocate
         Matrix<T> m5(mSize / 2, false); // Declare matrix, but do not allocate
         Matrix<T> m6(mSize / 2, false); // Declare matrix, but do not allocate
         Matrix<T> m7(mSize / 2, false); // Declare matrix, but do not allocate
         Matrix<int>* null = NULL;
         /*/
         Matrix<T> *m1;
         Matrix<T> *m2;
         Matrix<T> *m3;
         Matrix<T> *m4;
         Matrix<T> *m5;
         Matrix<T> *m6;
         Matrix<T> *m7;
         Matrix<T> *b1;
         Matrix<T> *b2;
         Matrix<T> *b3;
         Matrix<T> *b4;
         Matrix<T> *b5;
         Matrix<T> *b6;
         Matrix<T> *b7;
         Matrix<int>* null = NULL;
         
         if (mSize < 4096)
         {
            m1 = new Matrix<T>(a00, a11, true ); // Create new object, adding 2nd to 1st
            m2 = new Matrix<T>(a10, a11, true ); // Create new object, adding 2nd to 1st
            m3 = new Matrix<T>(a00            ); // Make a copy...
            m4 = new Matrix<T>(a11            ); // Make a copy...
            m5 = new Matrix<T>(a00, a01, true ); // Create new object, adding 2nd to 1st
            m6 = new Matrix<T>(a10, a00, false); // Create new object, subtracting 2nd from 1st
            m7 = new Matrix<T>(a01, a11, false); // Create new object, subtracting 2nd from 1st
            b1 = new Matrix<T>(b00, b11, true ); // Create new object, adding 2nd to 1st
            b2 = new Matrix<T>(b00            ); // Make a copy... -----------------------------Should be able to get away with a pointer copy... - not with the memory deallocation that follows...
            b3 = new Matrix<T>(b01, b11, false); // Create new object, subtracting 2nd from 1st
            b4 = new Matrix<T>(b10, b00, false); // Create new object, adding 2nd to 1st
            b5 = new Matrix<T>(b11            ); // Make a copy... -----------------------------Should be able to get away with a pointer copy... - not with the memory deallocation that follows...
            b6 = new Matrix<T>(b00, b01, true ); // Create new object, subtracting 2nd from 1st
            b7 = new Matrix<T>(b10, b11, true ); // Create new object, adding 2nd to 1st
         }
         else
         {
            t[1]  = std::thread(&Matrix<T>::addFill, &a00, std::ref(a11), std::ref(m1), true );
            t[2]  = std::thread(&Matrix<T>::addFill, &a10, std::ref(a11), std::ref(m2), true );
            t[3]  = std::thread(&Matrix<T>::copyTo , &a00               , std::ref(m3)       );
            t[4]  = std::thread(&Matrix<T>::copyTo , &a11               , std::ref(m4)       );
            t[5]  = std::thread(&Matrix<T>::addFill, &a00, std::ref(a01), std::ref(m5), true );
            t[6]  = std::thread(&Matrix<T>::addFill, &a10, std::ref(a00), std::ref(m6), false);
            t[7]  = std::thread(&Matrix<T>::addFill, &a01, std::ref(a11), std::ref(m7), false);
            
            t[8]  = std::thread(&Matrix<T>::addFill, &b00, std::ref(b11), std::ref(b1), true );
            t[9]  = std::thread(&Matrix<T>::copyTo , &b00               , std::ref(b2)       );
            t[10] = std::thread(&Matrix<T>::addFill, &b01, std::ref(b11), std::ref(b3), false);
            t[11] = std::thread(&Matrix<T>::addFill, &b10, std::ref(b00), std::ref(b4), false);
            t[12] = std::thread(&Matrix<T>::copyTo , &b11               , std::ref(b5)       );
            t[13] = std::thread(&Matrix<T>::addFill, &b00, std::ref(b01), std::ref(b6), true );
            t[14] = std::thread(&Matrix<T>::addFill, &b10, std::ref(b11), std::ref(b7), true );
            
            for (int i = 1; i < 15; ++i)
            {
               t[i].join();
            }
         }
         
         // Clear out allocated memory....
         b00.erase();
         b01.erase();
         b10.erase();
         b11.erase();
         // We don't need matrixB data anymore. Erase it.
         matrixB.erase();
         /**/
         
         // Get the 7 multiplication results
         // m1 = (a00 + a11) * (b00 + b11);
         // m2 = (a10 + a11) *  b00;
         // m3 =  a00 *        (b01 - b11);
         // m4 =  a11 *        (b10 - b00);
         // m5 = (a00 + a01) *  b11;
         // m6 = (a10 - a00) * (b00 + b01);
         // m7 = (a01 - a11) * (b10 + b11);
         
         //std::mutex mut1;
         //std::mutex mut2;
         //std::mutex mut3;
         //std::mutex mut4;
         //std::mutex mut5;
         //std::mutex mut6;
         //std::mutex mut7;
         //std::unique_lock<std::mutex> lk1(mut1);
         //std::unique_lock<std::mutex> lk2(mut2);
         //std::unique_lock<std::mutex> lk3(mut3);
         //std::unique_lock<std::mutex> lk4(mut4);
         //std::unique_lock<std::mutex> lk5(mut5);
         //std::unique_lock<std::mutex> lk6(mut6);
         //std::unique_lock<std::mutex> lk7(mut7);
         
         // Split for the thread number optimization
         if (mSize > thread_Start)
         {
            // Matrix is too large. Slow down to save memory...
            /**/
            //m1.mult_wrapper((b00 + b11), null);
            //m2.mult_wrapper((b00)      , null);
            //m3.mult_wrapper((b01 - b11), null);
            //m4.mult_wrapper((b10 - b00), null);
            //m5.mult_wrapper((b11)      , null);
            //m6.mult_wrapper((b00 + b01), null);
            //m7.mult_wrapper((b10 + b11), null);
            //m1.allocMath(a00, a11, true ); // Create new object, adding 2nd to 1st
            //m2.allocMath(a10, a11, true ); // Create new object, adding 2nd to 1st
            //m3.allocCopy(a00            ); // Make a copy...
            //m4.allocCopy(a11            ); // Make a copy...
            //m5.allocMath(a00, a01, true ); // Create new object, adding 2nd to 1st
            //m6.allocMath(a10, a00, false); // Create new object, subtracting 2nd from 1st
            //m7.allocMath(a01, a11, false); // Create new object, subtracting 2nd from 1st
            //m1.mult_wrapper((b00 + b11));
            //m2.mult_wrapper((b00)      );
            //m3.mult_wrapper((b01 - b11));
            //m4.mult_wrapper((b10 - b00));
            //m5.mult_wrapper((b11)      );
            //m6.mult_wrapper((b00 + b01));
            //m7.mult_wrapper((b10 + b11));
            m1.mult_Fast_Slave(&a00, &a11 , true , &b00, &b11 , true );
            m2.mult_Fast_Slave(&a10, &a11 , true , &b00,  null, false);
            m3.mult_Fast_Slave(&a00,  null, false, &b01, &b11 , false);
            m4.mult_Fast_Slave(&a11,  null, false, &b10, &b00 , false);
            m5.mult_Fast_Slave(&a00, &a01 , true , &b11,  null, false);
            m6.mult_Fast_Slave(&a10, &a00 , false, &b00, &b01 , true );
            m7.mult_Fast_Slave(&a01, &a11 , false, &b10, &b11 , true );
            //lk1.unlock();
            //lk2.unlock();
            //lk3.unlock();
            //lk4.unlock();
            //lk5.unlock();
            //lk6.unlock();
            //lk7.unlock();
            /*/
            //m1.mult_FarmSlave(b1, null);
            //m2.mult_FarmSlave(b2, null);
            //m3.mult_FarmSlave(b3, null);
            //m4.mult_FarmSlave(b4, null);
            //m5.mult_FarmSlave(b5, null);
            //m6.mult_FarmSlave(b6, null);
            //m7.mult_FarmSlave(b7, null);
            m1->mult_FarmSlave(*b1, null);
            m2->mult_FarmSlave(*b2, null);
            m3->mult_FarmSlave(*b3, null);
            m4->mult_FarmSlave(*b4, null);
            m5->mult_FarmSlave(*b5, null);
            m6->mult_FarmSlave(*b6, null);
            m7->mult_FarmSlave(*b7, null);
            /**/
         }
         else if (mSize > thread_Stop)
         {
            /**/
            //t[1] = std::thread(&Matrix<T>::mult_FarmSlave, &m1, (b00 + b11), null);
            //t[2] = std::thread(&Matrix<T>::mult_FarmSlave, &m2, (b00)      , null);
            //t[3] = std::thread(&Matrix<T>::mult_FarmSlave, &m3, (b01 - b11), null);
            //t[4] = std::thread(&Matrix<T>::mult_FarmSlave, &m4, (b10 - b00), null);
            //t[5] = std::thread(&Matrix<T>::mult_FarmSlave, &m5, (b11)      , null);
            //t[6] = std::thread(&Matrix<T>::mult_FarmSlave, &m6, (b00 + b01), null);
            //t[7] = std::thread(&Matrix<T>::mult_FarmSlave, &m7, (b10 + b11), null);
            /*/
            m1.allocMath(a00, a11, true ); // Create new object, adding 2nd to 1st
            m2.allocMath(a10, a11, true ); // Create new object, adding 2nd to 1st
            m3.allocCopy(a00            ); // Make a copy...
            m4.allocCopy(a11            ); // Make a copy...
            m5.allocMath(a00, a01, true ); // Create new object, adding 2nd to 1st
            m6.allocMath(a10, a00, false); // Create new object, subtracting 2nd from 1st
            m7.allocMath(a01, a11, false); // Create new object, subtracting 2nd from 1st
            t[1] = std::thread(&Matrix<T>::mult_wrapper, &m1, (b00 + b11));
            t[2] = std::thread(&Matrix<T>::mult_wrapper, &m2, (b00)      );
            t[3] = std::thread(&Matrix<T>::mult_wrapper, &m3, (b01 - b11));
            t[4] = std::thread(&Matrix<T>::mult_wrapper, &m4, (b10 - b00));
            t[5] = std::thread(&Matrix<T>::mult_wrapper, &m5, (b11)      );
            t[6] = std::thread(&Matrix<T>::mult_wrapper, &m6, (b00 + b01));
            t[7] = std::thread(&Matrix<T>::mult_wrapper, &m7, (b10 + b11));
            /*/
            //t[1] = std::thread(&Matrix<T>::mult_Fast_Slave, &m1, &a00, &a11 , true , &b00, &b11 , true , std::move(lk1));
            //t[2] = std::thread(&Matrix<T>::mult_Fast_Slave, &m2, &a10, &a11 , true , &b00,  null, false, std::move(lk2));
            //t[3] = std::thread(&Matrix<T>::mult_Fast_Slave, &m3, &a00,  null, false, &b01, &b11 , false, std::move(lk3));
            //t[4] = std::thread(&Matrix<T>::mult_Fast_Slave, &m4, &a11,  null, false, &b10, &b00 , false, std::move(lk4));
            //t[5] = std::thread(&Matrix<T>::mult_Fast_Slave, &m5, &a00, &a01 , true , &b11,  null, false, std::move(lk5));
            //t[6] = std::thread(&Matrix<T>::mult_Fast_Slave, &m6, &a10, &a00 , false, &b00, &b01 , true , std::move(lk6));
            //t[7] = std::thread(&Matrix<T>::mult_Fast_Slave, &m7, &a01, &a11 , false, &b10, &b11 , true , std::move(lk7));
            t[1] = std::thread(&Matrix<T>::mult_Fast_Slave, &m1, &a00, &a11 , true , &b00, &b11 , true );
            t[2] = std::thread(&Matrix<T>::mult_Fast_Slave, &m2, &a10, &a11 , true , &b00,  null, false);
            t[3] = std::thread(&Matrix<T>::mult_Fast_Slave, &m3, &a00,  null, false, &b01, &b11 , false);
            t[4] = std::thread(&Matrix<T>::mult_Fast_Slave, &m4, &a11,  null, false, &b10, &b00 , false);
            t[5] = std::thread(&Matrix<T>::mult_Fast_Slave, &m5, &a00, &a01 , true , &b11,  null, false);
            t[6] = std::thread(&Matrix<T>::mult_Fast_Slave, &m6, &a10, &a00 , false, &b00, &b01 , true );
            t[7] = std::thread(&Matrix<T>::mult_Fast_Slave, &m7, &a01, &a11 , false, &b10, &b11 , true );
            /**/
            /*/
            //t[1] = std::thread(&Matrix<T>::mult_FarmSlave, &m1, b1, null);
            //t[2] = std::thread(&Matrix<T>::mult_FarmSlave, &m2, b2, null);
            //t[3] = std::thread(&Matrix<T>::mult_FarmSlave, &m3, b3, null);
            //t[4] = std::thread(&Matrix<T>::mult_FarmSlave, &m4, b4, null);
            //t[5] = std::thread(&Matrix<T>::mult_FarmSlave, &m5, b5, null);
            //t[6] = std::thread(&Matrix<T>::mult_FarmSlave, &m6, b6, null);
            //t[7] = std::thread(&Matrix<T>::mult_FarmSlave, &m7, b7, null);
            t[1] = std::thread(&Matrix<T>::mult_FarmSlave, &(*m1), *b1, null);
            t[2] = std::thread(&Matrix<T>::mult_FarmSlave, &(*m2), *b2, null);
            t[3] = std::thread(&Matrix<T>::mult_FarmSlave, &(*m3), *b3, null);
            t[4] = std::thread(&Matrix<T>::mult_FarmSlave, &(*m4), *b4, null);
            t[5] = std::thread(&Matrix<T>::mult_FarmSlave, &(*m5), *b5, null);
            t[6] = std::thread(&Matrix<T>::mult_FarmSlave, &(*m6), *b6, null);
            t[7] = std::thread(&Matrix<T>::mult_FarmSlave, &(*m7), *b7, null);
            /**/
         //}
         //else if (mSize > 512)
         //{
         //   // At the moment, this appears to slow down execution
         //   m1.mult_wrapper (b00 + b11, null);
         //   m2.mult_wrapper (b00      , null);
         //   m3.mult_wrapper (b01 - b11, null);
         //   m4.mult_wrapper (b10 - b00, null);
         //   m5.mult_wrapper (b11      , null);
         //   m6.mult_wrapper (b00 + b01, null);
         //   m7.mult_wrapper (b10 + b11, null);
         }
         else
         {
            /**/
            // Make this run parallel... this is just more time consuming the way it is
            // Below would probably be faster:
            //Matrix<T> m(mSize);
            //this->multStandard(matrixB, m);
            //this->erase();
            //this->allocCopy(m);
            //finished = true;
            //return;
            //
            /**/
            //m1.reallocate();
            //m2.reallocate();
            //m3.reallocate();
            //m4.reallocate();
            //m5.reallocate();
            //m6.reallocate();
            //m7.reallocate();
            //(a00 + a11).multStandard (b00 + b11, m1);
            //(a10 + a11).multStandard (b00      , m2);
            //(a00)      .multStandard (b01 - b11, m3);
            //(a11)      .multStandard (b10 - b00, m4);
            //(a00 + a01).multStandard (b11      , m5);
            //(a10 - a00).multStandard (b00 + b01, m6);
            //(a01 - a11).multStandard (b10 + b11, m7);
            m1.multStandard2(&a00, &a11 , true , &b00, &b11 , true );
            m2.multStandard2(&a10, &a11 , true , &b00,  null, false);
            m3.multStandard2(&a00,  null, false, &b01, &b11 , false);
            m4.multStandard2(&a11,  null, false, &b10, &b00 , false);
            m5.multStandard2(&a00, &a01 , true , &b11,  null, false);
            m6.multStandard2(&a10, &a00 , false, &b00, &b01 , true );
            m7.multStandard2(&a01, &a11 , false, &b10, &b11 , true );
            /*/
            //t[1] = std::thread(&Matrix<T>::multStandard2, &m1, &a00, &a11 , true , &b00, &b11 , true , std::move(lk1));
            //t[2] = std::thread(&Matrix<T>::multStandard2, &m2, &a10, &a11 , true , &b00,  null, false, std::move(lk2));
            //t[3] = std::thread(&Matrix<T>::multStandard2, &m3, &a00,  null, false, &b01, &b11 , false, std::move(lk3));
            //t[4] = std::thread(&Matrix<T>::multStandard2, &m4, &a11,  null, false, &b10, &b00 , false, std::move(lk4));
            //t[5] = std::thread(&Matrix<T>::multStandard2, &m5, &a00, &a01 , true , &b11,  null, false, std::move(lk5));
            //t[6] = std::thread(&Matrix<T>::multStandard2, &m6, &a10, &a00 , false, &b00, &b01 , true , std::move(lk6));
            //t[7] = std::thread(&Matrix<T>::multStandard2, &m7, &a01, &a11 , false, &b10, &b11 , true , std::move(lk7));
            //std::unique_lock<std::mutex> lk1(m1.mMutex);
            //std::unique_lock<std::mutex> lk2(m2.mMutex);
            //std::unique_lock<std::mutex> lk3(m3.mMutex);
            //std::unique_lock<std::mutex> lk4(m4.mMutex);
            //std::unique_lock<std::mutex> lk5(m5.mMutex);
            //std::unique_lock<std::mutex> lk6(m6.mMutex);
            //std::unique_lock<std::mutex> lk7(m7.mMutex);
            //lk1.release();
            //lk2.release();
            //lk3.release();
            //lk4.release();
            //lk5.release();
            //lk6.release();
            //lk7.release();
            t[1] = std::thread(&Matrix<T>::multStandard2, &m1, &a00, &a11 , true , &b00, &b11 , true );
            t[2] = std::thread(&Matrix<T>::multStandard2, &m2, &a10, &a11 , true , &b00,  null, false);
            t[3] = std::thread(&Matrix<T>::multStandard2, &m3, &a00,  null, false, &b01, &b11 , false);
            t[4] = std::thread(&Matrix<T>::multStandard2, &m4, &a11,  null, false, &b10, &b00 , false);
            t[5] = std::thread(&Matrix<T>::multStandard2, &m5, &a00, &a01 , true , &b11,  null, false);
            t[6] = std::thread(&Matrix<T>::multStandard2, &m6, &a10, &a00 , false, &b00, &b01 , true );
            t[7] = std::thread(&Matrix<T>::multStandard2, &m7, &a01, &a11 , false, &b10, &b11 , true );
            //t[1].join();
            //t[2].join();
            //t[3].join();
            //t[4].join();
            //t[5].join();
            //t[6].join();
            //t[7].join();
            /**/
            /*/
            //(a00 + a11).multStandard (b1, m1);
            //(a10 + a11).multStandard (b2, m2);
            //(a00)      .multStandard (b3, m3);
            //(a11)      .multStandard (b4, m4);
            //(a00 + a01).multStandard (b5, m5);
            //(a10 - a00).multStandard (b6, m6);
            //(a01 - a11).multStandard (b7, m7);
            (a00 + a11).multStandard (*b1, *m1);
            (a10 + a11).multStandard (*b2, *m2);
            (a00)      .multStandard (*b3, *m3);
            (a11)      .multStandard (*b4, *m4);
            (a00 + a01).multStandard (*b5, *m5);
            (a10 - a00).multStandard (*b6, *m6);
            (a01 - a11).multStandard (*b7, *m7);
            /**/
         }
         //std::this_thread::yield();
         //sleep(4);
         //if (lk1.owns_lock()) lk1.unlock();
         //if (lk2.owns_lock()) lk2.unlock();
         //if (lk3.owns_lock()) lk3.unlock();
         //if (lk4.owns_lock()) lk4.unlock();
         //if (lk5.owns_lock()) lk5.unlock();
         //if (lk6.owns_lock()) lk6.unlock();
         //if (lk7.owns_lock()) lk7.unlock();
         //std::unique_lock<std::mutex> lock1(mut1);
         //std::unique_lock<std::mutex> lock2(mut2);
         //std::unique_lock<std::mutex> lock3(mut3);
         //std::unique_lock<std::mutex> lock4(mut4);
         //std::unique_lock<std::mutex> lock5(mut5);
         //std::unique_lock<std::mutex> lock6(mut6);
         //std::unique_lock<std::mutex> lock7(mut7);
         
         /**/
         // Clear out allocated memory....
         //b00.erase();
         //b01.erase();
         //b10.erase();
         //b11.erase();
         //// We don't need matrixB data anymore. Erase it.
         //matrixB.erase();
         /*/
         //b1.erase();
         //b2.erase();
         //b3.erase();
         //b4.erase();
         //b5.erase();
         //b6.erase();
         //b7.erase();
         //b1->erase();
         //b2->erase();
         //b3->erase();
         //b4->erase();
         //b5->erase();
         //b6->erase();
         //b7->erase();
         delete b1;
         delete b2;
         delete b3;
         delete b4;
         delete b5;
         delete b6;
         delete b7;
         /**/
         
         //if (mSize <= thread_Start && mSize > thread_Stop)
         //{
         //   t[1].join();
         //   t[2].join();
         //   t[3].join();
         //   t[4].join();
         //   t[5].join();
         //   t[6].join();
         //   t[7].join();
         //}
         // Use the 7 multiplication results to get the results for each quadrant
         // Save on memory usage by reusing one set of quadrants
         //a00 = m1 + m4 - m5 + m7;
         //a01 = m3 + m5;
         //a10 = m2 + m4;
         //a11 = m1 + m3 - m2 + m6;
         //*/
         //if (mSize <= thread_Stop)
         //{
         //   a00.op00_11(m1, m4, m5, m7);
         //   a01.op01_10(m3, m5);
         //   a10.op01_10(m2, m4);
         //   a11.op00_11(m1, m3, m2, m6);
         //}
         //else
         //{
         //sleep(1);
         //std::cerr << Yel << "Entering busy loops..." << RCol << "\n";
         //while (!m1.started && !m1.finished); std::cerr << Red << "Exiting busy loops 1..." << RCol << "\n";
         //while (!m2.started && !m2.finished); std::cerr << Red << "Exiting busy loops 2..." << RCol << "\n";
         //while (!m3.started && !m3.finished); std::cerr << Red << "Exiting busy loops 3..." << RCol << "\n";
         //while (!m4.started && !m4.finished); std::cerr << Red << "Exiting busy loops 4..." << RCol << "\n";
         //while (!m5.started && !m5.finished); std::cerr << Red << "Exiting busy loops 5..." << RCol << "\n";
         //while (!m6.started && !m6.finished); std::cerr << Red << "Exiting busy loops 6..." << RCol << "\n";
         //while (!m7.started && !m7.finished); std::cerr << Red << "Exiting busy loops 7..." << RCol << "\n";
         t[12] = std::thread(&Matrix<T>::op00_11_con, &a00, std::ref(m1), std::ref(m4), std::ref(m5), std::ref(m7), t, 1, 4, 5, 7);
         t[13] = std::thread(&Matrix<T>::op01_10_con, &a01, std::ref(m3), std::ref(m5), t, 3, 5);
         t[14] = std::thread(&Matrix<T>::op01_10_con, &a10, std::ref(m2), std::ref(m4), t, 2, 4);
         t[15] = std::thread(&Matrix<T>::op00_11_con, &a11, std::ref(m1), std::ref(m3), std::ref(m2), std::ref(m6), t, 1, 3, 2, 6);
         t[12].join();
         t[13].join();
         t[14].join();
         t[15].join();
         //}
         /*/
         if (mSize < 4096)
         {
            a00.op00_11(*m1, *m4, *m5, *m7);
            a01.op01_10(*m3, *m5);
            a10.op01_10(*m2, *m4);
            a11.op00_11(*m1, *m3, *m2, *m6);
         }
         else
         {
            t[1] = std::thread(&Matrix<T>::op00_11, &a00, *m1, *m4, *m5, *m7);
            t[2] = std::thread(&Matrix<T>::op01_10, &a01, *m3, *m5);
            t[3] = std::thread(&Matrix<T>::op01_10, &a10, *m2, *m4);
            t[4] = std::thread(&Matrix<T>::op00_11, &a11, *m1, *m3, *m2, *m6);
            t[1].join();
            t[2].join();
            t[3].join();
            t[4].join();
         }
         delete m1;
         delete m2;
         delete m3;
         delete m4;
         delete m5;
         delete m6;
         delete m7;
         //delete b1;
         //delete b2;
         //delete b3;
         //delete b4;
         //delete b5;
         //delete b6;
         //delete b7;
         /**/
         //a00 = m1;
         //a00 += m4;
         //a00 -= m5;
         //a00 += m7;
         //a01 = m3;
         //a01 += m5;
         //a10 = m2;
         //a10 += m4;
         //a11 = m1;
         //a11 += m3;
         //a11 -= m2;
         //a11 += m6;
         
         // The above will re-write matrixA (calling object)
         // Reassemble the quadrants into a single whole
         //if (result != NULL)
         //{
         //   *result = Matrix(a00, a01, a10, a11);
         //}
      }
      else
      {
         // Assume a matrix of size 1
         //if (result != NULL)
         //{
         //   *result[0][0] = mRows[0][0] * matrixB[0][0];
         //}
         mRows[0][0] = mRows[0][0] * matrixB[0][0];
      }
      finished = true;
   }
   
   /**************************************************************************
    * Matrix multiplication using Strassen's algorithm.
    * The input matrices must be equal in size and square, 
    *    and the size must be n x n, where n is a power of 2
    *************************************************************************/
   //void mult_wrapper(Matrix<T> matrixB, Matrix<T>* result)
   void mult_wrapper(Matrix<T> matrixB)
   {
      //this->mult_FarmSlave(matrixB, result);
      this->mult_FarmSlave(matrixB);
      //this->mult(matrixB);
   }
   
   /**************************************************************************
    * Standard matrix multiplication
    * The input matrices must be equal in size and square, 
    *    and the size must be n x n, where n is a power of 2
    *************************************************************************/
   //Matrix operator*(const Matrix matrixB) const
   //Matrix<T> multStandard(const Matrix<T>& matrixB, Matrix<T>& result) const
   void multStandard(const Matrix<T>& matrixB, Matrix<T>& result) const
   {
      // Take over the lock for result matrix.....
      std::lock_guard<std::mutex> lk(result.mMutex);
      result.started = true;
      for (int i = 0; i < mSize; ++i)
      {
         for (int j = 0; j < mSize; ++j )
         {
            result[i][j] = 0;
            for (int k = 0; k < mSize; ++k)
            {
               result[i][j] += (*this)[i][k] * matrixB[k][j];
            }
         }
      }
      result.finished = true;
      //std::cerr << "Exiting mult_standard...\n";
      //return result;
   }
   
   /**************************************************************************
    * Standard matrix multiplication
    * The input matrices must be equal in size and square, 
    *    and the size must be n x n, where n is a power of 2
    *************************************************************************/
   //Matrix operator*(const Matrix matrixB) const
   //Matrix<T> multStandard(const Matrix<T>& matrixB, Matrix<T>& result) const
   //void multStandard2(Matrix<T>* a0, Matrix<T>* a1, bool addA, Matrix<T>* b0, Matrix<T>* b1, bool addB, std::unique_lock<std::mutex>&& ctlMut)
   void multStandard2(Matrix<T>* a0, Matrix<T>* a1, bool addA, Matrix<T>* b0, Matrix<T>* b1, bool addB)
   {
      if (!mMutex.try_lock())
      {
         std::cerr << Red << "MS2: FAILED TO LOCK!!!!!" << RCol << std::endl;
      }
      else
      {
         mMutex.unlock();
      }
      // Take over the lock for this matrix.....
      std::lock_guard<std::mutex> lk(mMutex, std::adopt_lock);
      started = true;
      //ctlMut.unlock();
      // Allocate the memory and fill it with the specified data...
      this->reallocate();
      //Matrix<T> matrixA(mSize, false); // Declare but don't allocate
      //if (a1 != NULL)
      //{
      //   matrixA.allocMath(*a0, *a1, addA);
      //}
      //else
      //{
      //   matrixA.allocCopy(*a0);
      //}
      //Matrix<T> matrixB(mSize, false); // Declare but don't allocate
      //if (b1 != NULL)
      //{
      //   matrixB.allocMath(*b0, *b1, addB); // Allocate and compute matrixB
      //}
      //else
      //{
      //   matrixB.allocCopy(*b0);
      //}
      bool del_a1 = false;
      bool del_b1 = false;
      if (a1 == NULL)
      {
         a1 = new Matrix<T>(mSize, false);
         a1->allocZero();
         del_a1 = true;
      }
      if (b1 == NULL)
      {
         b1 = new Matrix<T>(mSize, false);
         b1->allocZero();
         del_b1 = true;
      }
      for (int i = 0; i < mSize; ++i)
      {
         for (int j = 0; j < mSize; ++j )
         {
            mRows[i][j] = 0;
            int a;
            int b;
            for (int k = 0; k < mSize; ++k)
            {
               a = addA ? ((*a0)[i][k] + (*a1)[i][k]) : ((*a0)[i][k] - (*a1)[i][k]);
               b = addB ? ((*b0)[k][j] + (*b1)[k][j]) : ((*b0)[k][j] - (*b1)[k][j]);
               //mRows[i][j] += (*this)[i][k] * matrixB[k][j];
               mRows[i][j] += a * b;
            }
         }
      }
      if (del_a1)
      {
         delete a1;
      }
      if (del_b1)
      {
         delete b1;
      }
      finished = true;
      mMutex.unlock();
      //std::cerr << "Exiting mult_standard2...\n";
      //return result;
   }
   
   /************************************************************************
   * Function to read data to fill a matrix from a network socket
   ***********************************************************************/
   bool readNet(Connection& net)
   {
      bool success = true;
      /*/
      for (int i = 0; i < mSize; ++i)
      {
         if (!net.receiveInt(this->mRows[i], mSize * sizeof(T)))
         {
            std::cerr << Red << "Server closed connection\n";
            std::cerr << "ERROR: " << net.strError << RCol << std::endl;
            success = false;
            break;
         }
      }
      /*/
      for (int i = 0; i < mSize && success; ++i)
      {
         /**/
         for (int j = 0; j < mSize; ++j)
         {
            //if (!net.receiveInt(&(this->mRows[i][j]), (int)sizeof(T)))
            if (!net.receiveData(&(this->mRows[i][j]), sizeof(T)))
            {
               std::cerr << Red << "Server closed connection: ReadNet\n";
               std::cerr << "ERROR: " << net.strError << RCol << std::endl;
               success = false;
               break;
            }
         }
         /*/
         // Currently fails, while above does not...
         int max = 64;
         int bytes = sizeof(T) < max ? sizeof(T) : max;
         //int blocks = sizeof(T) / max;
         for (int j = 0; j < sizeof(T); j += max)
         {
            if (!net.receiveInt(&(this->mRows[i][j]), bytes))
            {
               std::cerr << Red << "Server closed connection\n";
               std::cerr << "ERROR: " << net.strError << RCol << std::endl;
               success = false;
               break;
            }
         }
         /**/
      }
      /**/
      return success;
   }
   
   /************************************************************************
   * Function to write data from entire matrix to a network socket
   ***********************************************************************/
   bool writeNet(Connection& net) const
   {
      bool success = true;
      /*/
      for (int i = 0; i < mSize; ++i)
      {
         if (!net.sendInt(this->mRows[i], mSize * (sizeof(T))))
         {
            std::cerr << Red << "Server closed connection\n";
            std::cerr << "ERROR: " << net.strError << RCol << std::endl;
            success = false;
            break;
         }
      }
      /*/
      for (int i = 0; i < mSize && success; ++i)
      {
         /**/
         for (int j = 0; j < mSize; ++j)
         {
            //if (!net.sendInt(&(this->mRows[i][j]), (int)sizeof(T)))
            if (!net.sendData(&(this->mRows[i][j]), sizeof(T)))
            {
               std::cerr << Red << "Server closed connection: WriteNet\n";
               std::cerr << "ERROR: " << net.strError << RCol << std::endl;
               success = false;
               break;
            }
         }
         /*/
         // Currently fails, while above does not...
         int max = 64;
         int bytes = sizeof(T) < max ? sizeof(T) : max;
         //int blocks = sizeof(T) / max;
         for (int j = 0; j < sizeof(T); j += max)
         {
            if (!net.sendInt(&(this->mRows[i][j]), bytes))
            {
               std::cerr << Red << "Server closed connection\n";
               std::cerr << "ERROR: " << net.strError << RCol << std::endl;
               success = false;
               break;
            }
         }
         /**/
      }
      /**/
      return success;
   }
};

// Initialize the static variables for Matrix
template <class T>
int Matrix<T>::thread_Start = 8192;
template <class T>
bool Matrix<T>::NetError = false;
template <class T>
int Matrix<T>::thread_Stop = 512;
template <class T>
int Matrix<T>::sysCounter = 0;
template <class T>
std::mutex Matrix<T>::sysCounter_Mutex;
template <class T>
std::mutex Matrix<T>::threadLimiter[100];
template <class T>
int Matrix<T>::maxThreads = 100;

template <class T>
std::istream& operator>>(std::istream& is, const Matrix<T>& m)
{
   m.read(is);
   return is;
}

template <class T>
std::ostream& operator<< (std::ostream& os, const Matrix<T>& m)
{
   m.write(os);
   return os;
}

// Declare the mutex....
std::mutex byMultiples[10];
std::mutex oneAtATime;

/************************************************************************
* A function to allow threading from the connections
***********************************************************************/
template <class T>
void threadedManager(int socket, unsigned int id)
{
   // Limit execution of this block of code....
   // Automatically releases when leaving scope
   // Takes a little bit longer in real time, but will reduce context switches on the systems....
   //std::lock_guard<std::mutex> lock(oneByOne);
   //std::lock_guard<std::mutex> lock(threeByThree[id % 3]);
   // Use a condition variable instead??
   //std::unique_lock<std::mutex> myLock(limitMultiple);
   //doMultiple.wait(myLock, []{return multCounter < 3;});
   //{
   //std::lock_guard<std::mutex> lock(counterLock);
   //++multCounter;
   //}
   
   //cerr << Gre << "STARTING EXECUTION: THREAD " << id << RCol << "\n";
   Connection net(socket);
   int threadId = 0;
   int controlData[5] = {0, 0, 0, 0, 0};
   int close[5] = {0, 0, 0, 0, 0};
   // Combine these into one read....
   if (!net.receiveData(&controlData, 4 * 5))
   {
      std::cerr << Red << "Server closed connection: Receive control data\n";
      std::cerr << "ERROR: " << net.strError << RCol << std::endl;
   }
   threadId = controlData[0];
   int size = controlData[1];
   int threadMax = 3; // default - works well for many sizes.
   
   ///////////////////////////////////////////////////////////////////////////////////////////////
   ////////////// WORK: Change the value used according to the size of the input matrices (memory)
   ////////////// Also adapt using the memory size of the data type.....
   std::lock_guard<std::mutex> lock(byMultiples[id % 3]);
   ///////////////////////////////////////////////////////////////////////////////////////////////
   Matrix<T> matrixA(size);
   Matrix<T> matrixB(size);
   //Matrix<T> result(size);
   // The benefits of this are negligible on small matrices, but will greatly decrease memory
   //    usage on large matrices, with time benefits as well.
   //if (size < 8192) // big advantage first seen with 8192x8192
   if (size < 8192)
   {
      matrixA.thread_Stop = size / 4;
      matrixA.thread_Start = size;
   }
   else
   {
      matrixA.thread_Stop = 512;
      matrixA.thread_Start = matrixA.thread_Stop * 4;
   }
   //matrixA.thread_Stop = size / 4;
   //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   ////////// LOOK AT STRASSEN_INT_OPT_LARGE - WILL THE SAME METHOD WORK TO SPEED UP THE DISTRIBUTED VERSION? ///////////////////////////////////////////////////////////////////////////////////
   //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   
   // Receive matrix A and Receive matrix B
   if (matrixA.readNet(net) && matrixB.readNet(net))
   {
      {
      // create some inner scoping for the lock
      // We will allow multiple sends/receives at a time, but no doubling up on computation...
      //std::lock_guard<std::mutex> lock(oneAtATime);
      std::cerr << Gre << "STATUS: Multiplying matrices!!!" << RCol << "\n";
      //matrixA.mult_FarmSlave(matrixB, NULL);
      matrixA.mult_FarmSlave(matrixB);
      }
      // Send Result
      matrixA.writeNet(net);
   }
   else
   {
      std::cerr << Red << "ERROR: Failed to receive matrices!!!!" << RCol << "\n";
      net.closeComm();
      return;
   }
   net.closeComm();
   std::cerr << Gre << "FINISHING EXECUTION: THREAD " << id << ", manager thread " << threadId << RCol << "\n";
}

int threadServer(std::string port)
{
   Connection net;
   if (net.serverSetup(port.c_str()))
   {
      //std::cerr << Gre << "Net setup!!!" << RCol << "\n";
      // Handle this to close out socket and re-establish...
      int newSocket = 0;
      unsigned int threadCounter = 0;
      while (net.serverConnection(newSocket) != 0)
      {
         // To run threaded....
         std::thread newCon = std::thread(threadedManager<int>, newSocket, threadCounter++);
         newCon.detach(); // Detach the thread, so that we don't worry about its cleanup
      }
      net.closeServer();
   }
   else
   {
      std::cerr << Red << "\nError: " << net.strError << RCol << "\n\n";
      return 1;
   }
   return 0;
}

#endif //_MATRIX_H
