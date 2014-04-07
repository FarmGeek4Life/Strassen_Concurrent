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
//#include <chrono>
//#include <condition_variable>
//#include <cstring>
//#include <string>
#include <unistd.h>
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
   bool badMutexGrab;
   bool joined;

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
      badMutexGrab = false;
      joined = false;
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
      badMutexGrab = false;
      joined = false;
   }

   Matrix<T>(const Matrix<T>& matrixB)
   {
      mSize = matrixB.getSize();
      colAlloc = true;
      rowAlloc = true;
      finished = false;
      started = false;
      hasWaiting = false;
      badMutexGrab = false;
      joined = false;

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
      if (colAlloc || rowAlloc)
      {
         return;
      }
      mSize = matrixB.getSize();
      colAlloc = true;
      rowAlloc = true;
      finished = false;
      started = false;
      hasWaiting = false;
      badMutexGrab = false;
      joined = false;
      
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
      badMutexGrab = false;
      joined = false;
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
   
   std::mutex mMutex;
   std::mutex mCVMutex;
   std::mutex mWMutex;
   bool hasWaiting;
   
   /********************************************************************************************
   * Opportunistic math: as soon as the necessary threads complete, do the math
   ********************************************************************************************/
   Matrix<T>& op00_11_con(Matrix<T>& matrixA, Matrix<T>& matrixB, Matrix<T>& matrixC, Matrix<T>& matrixD, std::thread t[], int tA, int tB, int tC, int tD)
   {
      // Logic to prevent math until the specified threads complete
      // It looks bad, but it works (unlike everything else I tried)
      // The ugly logic is necessary when the processor is loaded
      {
      // Get the mutex
      std::lock_guard<std::mutex> lkA(matrixA.mMutex);
      // If I got the mutex before the calculation thread did
      if (!matrixA.started && !matrixA.badMutexGrab)
      {
         std::cerr << Red << "Bad mutex grab A!!!" << RCol << std::endl;
         // change variables to reflect the error
         matrixA.started = true;
         matrixA.badMutexGrab = true;
         // Drop the mutex like it's hot to prevent deadlock (on thread join)
         matrixA.mMutex.unlock();
      }
      // If another results thread got the mutex first, then I got it, but the calculation thread hasn't got it
      else if (matrixA.badMutexGrab)
      {
         std::cerr << Red << "Bad mutex grab looping A!!!" << RCol << std::endl;
         // Drop the mutex like it's hot to prevent deadlock (on thread join)
         matrixA.mMutex.unlock();
         // We do not want to continue past this point until the thread is joined,
         //    if we did we would get bad results (race condition problems)
         // I wish I could have a mutex that waited until it contained a certain value
         while (!matrixA.joined)
         {
            // Don't quite busy loop; this will force other threads to go.
            usleep(50000); // Sleep 50 milliseconds
         }
      }
      if (t[tA].joinable())
      {
      //std::cerr << "Passed wait: " << tA << "\n";
         try
         {
            t[tA].join();
            matrixA.joined = true;
         }
         catch (std::system_error &e)
         {
            std::cerr << "Caught std::system_error!\n";
         }
      }
      }
      {
      std::lock_guard<std::mutex> lkB(matrixB.mMutex);
      if (!matrixB.started && !matrixB.badMutexGrab)
      {
         std::cerr << Red << "Bad mutex grab B!!!" << RCol << std::endl;
         matrixB.started = true;
         matrixB.badMutexGrab = true;
         matrixB.mMutex.unlock();
      }
      else if (matrixB.badMutexGrab)
      {
         std::cerr << Red << "Bad mutex grab looping B!!!" << RCol << std::endl;
         matrixB.mMutex.unlock();
         while (!matrixB.joined)
         {
            usleep(50000); // Sleep 50 milliseconds
         }
      }
      if (t[tB].joinable())
      {
      //std::cerr << "Passed wait: " << tB << "\n";
         try
         {
            t[tB].join();
            matrixB.joined = true;
         }
         catch (std::system_error &e)
         {
            std::cerr << "Caught std::system_error!\n";
         }
      }
      }
      {
      std::lock_guard<std::mutex> lkC(matrixC.mMutex);
      if (!matrixC.started && !matrixC.badMutexGrab)
      {
         std::cerr << Red << "Bad mutex grab C!!!" << RCol << std::endl;
         matrixC.started = true;
         matrixC.badMutexGrab = true;
         matrixC.mMutex.unlock();
      }
      else if (matrixC.badMutexGrab)
      {
         std::cerr << Red << "Bad mutex grab looping C!!!" << RCol << std::endl;
         matrixC.mMutex.unlock();
         while (!matrixC.joined)
         {
            usleep(50000); // Sleep 50 milliseconds
         }
      }
      if (t[tC].joinable())
      {
      //std::cerr << "Passed wait: " << tC << "\n";
         try
         {
            t[tC].join();
            matrixC.joined = true;
         }
         catch (std::system_error &e)
         {
            std::cerr << "Caught std::system_error!\n";
         }
      }
      }
      {
      std::lock_guard<std::mutex> lkD(matrixD.mMutex);
      if (!matrixD.started && !matrixD.badMutexGrab)
      {
         std::cerr << Red << "Bad mutex grab D!!!" << RCol << std::endl;
         matrixD.started = true;
         matrixD.badMutexGrab = true;
         matrixD.mMutex.unlock();
      }
      else if (matrixD.badMutexGrab)
      {
         std::cerr << Red << "Bad mutex grab looping D!!!" << RCol << std::endl;
         matrixD.mMutex.unlock();
         while (!matrixD.joined)
         {
            usleep(50000); // Sleep 50 milliseconds
         }
      }
      if (t[tD].joinable())
      {
      //std::cerr << "Passed wait: " << tD << "\n";
         try
         {
            t[tD].join();
            matrixD.joined = true;
         }
         catch (std::system_error &e)
         {
            std::cerr << "Caught std::system_error!\n";
         }
      }
      }
      // Now actually do the math.
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
   Matrix<T>& op01_10_con(Matrix<T>& matrixA, Matrix<T>& matrixB, std::thread t[], int tA, int tB)
   {
      {
      std::lock_guard<std::mutex> lkA(matrixA.mMutex);
      if (!matrixA.started && !matrixA.badMutexGrab)
      {
         std::cerr << Red << "Bad mutex grab A!!!" << RCol << std::endl;
         matrixA.started = true;
         matrixA.badMutexGrab = true;
         matrixA.mMutex.unlock();
      }
      else if (matrixA.badMutexGrab)
      {
         std::cerr << Red << "Bad mutex grab looping A!!!" << RCol << std::endl;
         matrixA.mMutex.unlock();
         while (!matrixA.joined)
         {
            usleep(50000); // Sleep 50 milliseconds
         }
      }
      if (t[tA].joinable())
      {
      //std::cerr << "Passed wait: " << tA << "\n";
         try
         {
            t[tA].join();
            matrixA.joined = true;
         }
         catch (std::system_error &e)
         {
            std::cerr << "Caught std::system_error!\n";
         }
      }
      }
      {
      std::lock_guard<std::mutex> lkB(matrixB.mMutex);
      if (!matrixB.started && !matrixB.badMutexGrab)
      {
         std::cerr << Red << "Bad mutex grab B!!!" << RCol << std::endl;
         matrixB.started = true;
         matrixB.badMutexGrab = true;
         matrixB.mMutex.unlock();
      }
      else if (matrixB.badMutexGrab)
      {
         std::cerr << Red << "Bad mutex grab looping B!!!" << RCol << std::endl;
         matrixB.mMutex.unlock();
         while (!matrixB.joined)
         {
            usleep(50000); // Sleep 50 milliseconds
         }
      }
      if (t[tB].joinable())
      {
      //std::cerr << "Passed wait: " << tB << "\n";
         try
         {
            t[tB].join();
            matrixB.joined = true;
         }
         catch (std::system_error &e)
         {
            std::cerr << "Caught std::system_error!\n";
         }
      }
      }
      // Now actually do the math.
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
         this->allocCopy(*a0);
      }
      Matrix<T> matrixB(mSize, false); // Declare but don't allocate
      if (b1 != NULL)
      {
         matrixB.allocMath(*b0, *b1, addB); // Allocate and compute matrixB
      }
      else
      {
         matrixB.allocCopy(*b0);
      }
      mult_ThreadFarming(matrixB, computers, numComputers, port, true);
   }
   
   /**************************************************************************
    * Matrix multiplication using Strassen's algorithm.
    * The input matrices must be equal in size and square, 
    *    and the size must be n x n, where n is a power of 2
    *************************************************************************/
   void mult_ThreadFarming(Matrix<T>& matrixB, std::string computers[], int numComputers, std::string port, bool wrapped = false)
   {
      // Take over the lock for this matrix.....
      std::mutex* pMutex = &mMutex;
      if (wrapped)
      {
         pMutex = &mWMutex;
      }
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
         Matrix<T> m1(mSize / 2, false); // Declare matrix, but do not allocate
         Matrix<T> m2(mSize / 2, false); // Declare matrix, but do not allocate
         Matrix<T> m3(mSize / 2, false); // Declare matrix, but do not allocate
         Matrix<T> m4(mSize / 2, false); // Declare matrix, but do not allocate
         Matrix<T> m5(mSize / 2, false); // Declare matrix, but do not allocate
         Matrix<T> m6(mSize / 2, false); // Declare matrix, but do not allocate
         Matrix<T> m7(mSize / 2, false); // Declare matrix, but do not allocate
         Matrix<int>* null = NULL;
         
         // Get the 7 multiplication results
         // m1 = (a00 + a11) * (b00 + b11);
         // m2 = (a10 + a11) *  b00;
         // m3 =  a00 *        (b01 - b11);
         // m4 =  a11 *        (b10 - b00);
         // m5 = (a00 + a01) *  b11;
         // m6 = (a10 - a00) * (b00 + b01);
         // m7 = (a01 - a11) * (b10 + b11);
         
         // It makes no sense to split into smaller chunks for 7 computers
         if (mSize > thread_Stop && numComputers != 7 && numComputers != 1)
         {
            t[1] = std::thread(&Matrix<T>::mult_Fast_Farm, &m1, &a00, &a11 , true , &b00, &b11 , true , computers, numComputers, port);
            t[2] = std::thread(&Matrix<T>::mult_Fast_Farm, &m2, &a10, &a11 , true , &b00,  null, false, computers, numComputers, port);
            t[3] = std::thread(&Matrix<T>::mult_Fast_Farm, &m3, &a00,  null, false, &b01, &b11 , false, computers, numComputers, port);
            t[4] = std::thread(&Matrix<T>::mult_Fast_Farm, &m4, &a11,  null, false, &b10, &b00 , false, computers, numComputers, port);
            t[5] = std::thread(&Matrix<T>::mult_Fast_Farm, &m5, &a00, &a01 , true , &b11,  null, false, computers, numComputers, port);
            t[6] = std::thread(&Matrix<T>::mult_Fast_Farm, &m6, &a10, &a00 , false, &b00, &b01 , true , computers, numComputers, port);
            t[7] = std::thread(&Matrix<T>::mult_Fast_Farm, &m7, &a01, &a11 , false, &b10, &b11 , true , computers, numComputers, port);
         }
         else
         {
            // Mutex: control the access to the system counter (sysCounter) for even distribution
            std::lock_guard<std::mutex> lock(sysCounter_Mutex);
            t[1] = std::thread(&Matrix<T>::runParallel_Fast, &m1, &a00, &a11 , true , &b00, &b11 , true , computers[sysCounter % numComputers], port, sysCounter); ++sysCounter;
            t[2] = std::thread(&Matrix<T>::runParallel_Fast, &m2, &a10, &a11 , true , &b00,  null, false, computers[sysCounter % numComputers], port, sysCounter); ++sysCounter;
            t[3] = std::thread(&Matrix<T>::runParallel_Fast, &m3, &a00,  null, false, &b01, &b11 , false, computers[sysCounter % numComputers], port, sysCounter); ++sysCounter;
            t[4] = std::thread(&Matrix<T>::runParallel_Fast, &m4, &a11,  null, false, &b10, &b00 , false, computers[sysCounter % numComputers], port, sysCounter); ++sysCounter;
            t[5] = std::thread(&Matrix<T>::runParallel_Fast, &m5, &a00, &a01 , true , &b11,  null, false, computers[sysCounter % numComputers], port, sysCounter); ++sysCounter;
            t[6] = std::thread(&Matrix<T>::runParallel_Fast, &m6, &a10, &a00 , false, &b00, &b01 , true , computers[sysCounter % numComputers], port, sysCounter); ++sysCounter;
            t[7] = std::thread(&Matrix<T>::runParallel_Fast, &m7, &a01, &a11 , false, &b10, &b11 , true , computers[sysCounter % numComputers], port, sysCounter); ++sysCounter;
         }
         
         // Use the 7 multiplication results to get the results for each quadrant
         // Save on memory usage by reusing one set of quadrants
         // Also saves on time - no de/reallocation
         //a00 = m1 + m4 - m5 + m7;
         //a01 = m3 + m5;
         //a10 = m2 + m4;
         //a11 = m1 + m3 - m2 + m6;
         t[12] = std::thread(&Matrix<T>::op00_11_con, &a00, std::ref(m1), std::ref(m4), std::ref(m5), std::ref(m7), t, 1, 4, 5, 7);
         t[13] = std::thread(&Matrix<T>::op01_10_con, &a01, std::ref(m3), std::ref(m5), t, 3, 5);
         t[14] = std::thread(&Matrix<T>::op01_10_con, &a10, std::ref(m2), std::ref(m4), t, 2, 4);
         t[15] = std::thread(&Matrix<T>::op00_11_con, &a11, std::ref(m1), std::ref(m3), std::ref(m2), std::ref(m6), t, 1, 3, 2, 6);
         t[12].join();
         t[13].join();
         t[14].join();
         t[15].join();
         // The above will re-write matrixA (calling object)
         // Reassemble the quadrants into a single whole
      }
      else
      {
         mRows[0][0] = mRows[0][0] * matrixB[0][0];
      }
      finished = true;
   }
   
   void runParallel_Fast(Matrix<T>* a0, Matrix<T>* a1, bool addA, Matrix<T>* b0, Matrix<T>* b1, bool addB, std::string computer, std::string port, int id)
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
      runParallel(matrixB, computer, port, id, true);
   }
   /**************************************************************************
    * Matrix multiplication using Strassen's algorithm.
    * The input matrices must be equal in size and square, 
    *    and the size must be n x n, where n is a power of 2
    *************************************************************************/
   void runParallel(Matrix<T>& matrixB, std::string computer, std::string port, int id, bool wrapped = false)
   {
      // Take over the lock for this matrix.....
      std::mutex* pMutex = &mMutex;
      if (wrapped)
      {
         pMutex = &mWMutex;
      }
      std::lock_guard<std::mutex> lk(*pMutex);
      started = true;
      
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
            //if (failed || !this->writeNet(net))
            if (failed || !this->writeNet(net, true))
            {
               std::cerr << Red << "Server closed connection: " << computer << "\n";
               std::cerr << "ERROR (" << computer << "): " << net.strError << RCol << "\n";
               failed = true;
            }
            // Send matrix B
            //if (failed || !matrixB.writeNet(net))
            if (failed || !matrixB.writeNet(net, true))
            {
               std::cerr << Red << "Server closed connection: " << computer << "\n";
               std::cerr << "ERROR (" << computer << "): " << net.strError << RCol << "\n";
               failed = true;
            }
            // Erase the allocated memory of matrixB. It's not needed anymore.
            matrixB.erase();
            // Receive Result
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
         mRows[0][0] = mRows[0][0] * matrixB[0][0];
      }
      finished = true;
      std::cerr << Gre << "EXITING THREAD " << id << " FOR SYSTEM '" << computer << "'!!!" << RCol  << "\n";
   }
   
   /**************************************************************************
    * this: m-matrix
    * need to pass in a*, a*, addA, b*, b*, addB
    * 
    *************************************************************************/
   void mult_Fast_Slave(Matrix<T>* a0, Matrix<T>* a1, bool addA, Matrix<T>* b0, Matrix<T>* b1, bool addB)
   {
      started = true;
      // Allocate the memory and fill it with the specified data...
      if (a1 != NULL)
      {
         this->allocMath(*a0, *a1, addA);
      }
      else
      {
         this->allocCopy(*a0);
      }
      Matrix<T> matrixB(mSize, false); // Declare but don't allocate
      if (b1 != NULL)
      {
         matrixB.allocMath(*b0, *b1, addB); // Allocate and compute matrixB
      }
      else
      {
         matrixB.allocCopy(*b0);
      }
      mult_FarmSlave(matrixB, true);
   }
   /**************************************************************************
    * Matrix multiplication using Strassen's algorithm.
    * The input matrices must be equal in size and square, 
    *    and the size must be n x n, where n is a power of 2
    *************************************************************************/
   void mult_FarmSlave(Matrix<T>& matrixB, bool wrapped = false)
   {
      if (mSize < 512 && mSize > 1 && mSize <= thread_Stop)
      {
         // This is, oddly enough, about 1 second faster for 2048x2048...
         Matrix<T> mA(*this);
         multStandard3(mA, matrixB);
      }
      else if (mSize > 1)
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
         Matrix<T> m1(mSize / 2, false); // Declare matrix, but do not allocate
         Matrix<T> m2(mSize / 2, false); // Declare matrix, but do not allocate
         Matrix<T> m3(mSize / 2, false); // Declare matrix, but do not allocate
         Matrix<T> m4(mSize / 2, false); // Declare matrix, but do not allocate
         Matrix<T> m5(mSize / 2, false); // Declare matrix, but do not allocate
         Matrix<T> m6(mSize / 2, false); // Declare matrix, but do not allocate
         Matrix<T> m7(mSize / 2, false); // Declare matrix, but do not allocate
         Matrix<int>* null = NULL;
         
         // Get the 7 multiplication results
         // m1 = (a00 + a11) * (b00 + b11);
         // m2 = (a10 + a11) *  b00;
         // m3 =  a00 *        (b01 - b11);
         // m4 =  a11 *        (b10 - b00);
         // m5 = (a00 + a01) *  b11;
         // m6 = (a10 - a00) * (b00 + b01);
         // m7 = (a01 - a11) * (b10 + b11);
         
         // Split for the thread number optimization
         if (mSize > thread_Start)
         {
            m1.mult_Fast_Slave(&a00, &a11 , true , &b00, &b11 , true );
            m2.mult_Fast_Slave(&a10, &a11 , true , &b00,  null, false);
            m3.mult_Fast_Slave(&a00,  null, false, &b01, &b11 , false);
            m4.mult_Fast_Slave(&a11,  null, false, &b10, &b00 , false);
            m5.mult_Fast_Slave(&a00, &a01 , true , &b11,  null, false);
            m6.mult_Fast_Slave(&a10, &a00 , false, &b00, &b01 , true );
            m7.mult_Fast_Slave(&a01, &a11 , false, &b10, &b11 , true );
         }
         else if (mSize > thread_Stop)
         {
            t[1] = std::thread(&Matrix<T>::mult_Fast_Slave, &m1, &a00, &a11 , true , &b00, &b11 , true );
            t[2] = std::thread(&Matrix<T>::mult_Fast_Slave, &m2, &a10, &a11 , true , &b00,  null, false);
            t[3] = std::thread(&Matrix<T>::mult_Fast_Slave, &m3, &a00,  null, false, &b01, &b11 , false);
            t[4] = std::thread(&Matrix<T>::mult_Fast_Slave, &m4, &a11,  null, false, &b10, &b00 , false);
            t[5] = std::thread(&Matrix<T>::mult_Fast_Slave, &m5, &a00, &a01 , true , &b11,  null, false);
            t[6] = std::thread(&Matrix<T>::mult_Fast_Slave, &m6, &a10, &a00 , false, &b00, &b01 , true );
            t[7] = std::thread(&Matrix<T>::mult_Fast_Slave, &m7, &a01, &a11 , false, &b10, &b11 , true );
            t[1].join();
            t[2].join();
            t[3].join();
            t[4].join();
            t[5].join();
            t[6].join();
            t[7].join();
         }
         else if (mSize > 512)
         {
            m1.mult_Fast_Slave(&a00, &a11 , true , &b00, &b11 , true );
            m2.mult_Fast_Slave(&a10, &a11 , true , &b00,  null, false);
            m3.mult_Fast_Slave(&a00,  null, false, &b01, &b11 , false);
            m4.mult_Fast_Slave(&a11,  null, false, &b10, &b00 , false);
            m5.mult_Fast_Slave(&a00, &a01 , true , &b11,  null, false);
            m6.mult_Fast_Slave(&a10, &a00 , false, &b00, &b01 , true );
            m7.mult_Fast_Slave(&a01, &a11 , false, &b10, &b11 , true );
         }
         else
         {
            // As odd as it seems, it is faster to do this at 512x512 than doing it as a single operation
            /**/
            m1.multStandard2(&a00, &a11 , true , &b00, &b11 , true );
            m2.multStandard2(&a10, &a11 , true , &b00,  null, false);
            m3.multStandard2(&a00,  null, false, &b01, &b11 , false);
            m4.multStandard2(&a11,  null, false, &b10, &b00 , false);
            m5.multStandard2(&a00, &a01 , true , &b11,  null, false);
            m6.multStandard2(&a10, &a00 , false, &b00, &b01 , true );
            m7.multStandard2(&a01, &a11 , false, &b10, &b11 , true );
            /*/
            t[1] = std::thread(&Matrix<T>::multStandard2, &m1, &a00, &a11 , true , &b00, &b11 , true );
            t[2] = std::thread(&Matrix<T>::multStandard2, &m2, &a10, &a11 , true , &b00,  null, false);
            t[3] = std::thread(&Matrix<T>::multStandard2, &m3, &a00,  null, false, &b01, &b11 , false);
            t[4] = std::thread(&Matrix<T>::multStandard2, &m4, &a11,  null, false, &b10, &b00 , false);
            t[5] = std::thread(&Matrix<T>::multStandard2, &m5, &a00, &a01 , true , &b11,  null, false);
            t[6] = std::thread(&Matrix<T>::multStandard2, &m6, &a10, &a00 , false, &b00, &b01 , true );
            t[7] = std::thread(&Matrix<T>::multStandard2, &m7, &a01, &a11 , false, &b10, &b11 , true );
            t[1].join();
            t[2].join();
            t[3].join();
            t[4].join();
            t[5].join();
            t[6].join();
            t[7].join();
            /**/
         }
         
         // Use the 7 multiplication results to get the results for each quadrant
         // Save on memory usage by reusing one set of quadrants
         //a00 = m1 + m4 - m5 + m7;
         //a01 = m3 + m5;
         //a10 = m2 + m4;
         //a11 = m1 + m3 - m2 + m6;
         a00.op00_11(m1, m4, m5, m7);
         a01.op01_10(m3, m5);
         a10.op01_10(m2, m4);
         a11.op00_11(m1, m3, m2, m6);
         // The above will re-write matrixA (calling object)
         // Reassemble the quadrants into a single whole
      }
      else
      {
         // Assume a matrix of size 1
         mRows[0][0] = mRows[0][0] * matrixB[0][0];
      }
      finished = true;
   }
   
   /**************************************************************************
    * Standard matrix multiplication
    * The input matrices must be equal in size and square, 
    *    and the size must be n x n, where n is a power of 2
    *************************************************************************/
   void multStandard2(Matrix<T>* a0, Matrix<T>* a1, bool addA, Matrix<T>* b0, Matrix<T>* b1, bool addB)
   {
      // Allocate the memory and fill it with the specified data...
      this->reallocate();
      Matrix<T> matrixA(mSize, false); // Declare but don't allocate
      if (a1 != NULL)
      {
         matrixA.allocMath(*a0, *a1, addA);
      }
      else
      {
         matrixA.allocCopy(*a0);
      }
      Matrix<T> matrixB(mSize, false); // Declare but don't allocate
      if (b1 != NULL)
      {
         matrixB.allocMath(*b0, *b1, addB); // Allocate and compute matrixB
      }
      else
      {
         matrixB.allocCopy(*b0);
      }
      for (int i = 0; i < mSize; ++i)
      {
         for (int j = 0; j < mSize; ++j )
         {
            mRows[i][j] = 0;
            for (int k = 0; k < mSize; ++k)
            {
               mRows[i][j] += matrixA[i][k] * matrixB[k][j];
            }
         }
      }
      finished = true;
   }
   
   /*********************************************************************
   * Allocate and fill the matrix with the specified data
   *********************************************************************/
   void multStandard3(const Matrix<T>& matrixA, const Matrix<T>& matrixB)
   {
      for (int i = 0; i < mSize; i++)
      {
         for (int j = 0; j < mSize; j++)
         {
            mRows[i][j] = 0;
            for (int k = 0; k < mSize; ++k)
            {
               mRows[i][j] += matrixA.mRows[i][k] * matrixB.mRows[k][j];
            }
         }
      }
   }
   
   /************************************************************************
   * Function to read data to fill a matrix from a network socket
   ***********************************************************************/
   //bool readNet(Connection& net)
   bool readNet(Connection& net, bool reduceSize = false)
   {
      bool success = true;
      // It may be duplication, but it will bigger branch misprediction problems
      if (reduceSize)
      {
         short temp;
         for (int i = 0; i < mSize && success; ++i)
         {
            for (int j = 0; j < mSize; ++j)
            {
               if (!net.receiveData(&temp, sizeof(short)))
               {
                  std::cerr << Red << "Server closed connection: ReadNet\n";
                  std::cerr << "ERROR: " << net.strError << RCol << std::endl;
                  success = false;
                  break;
               }
               this->mRows[i][j] = (int)temp;
            }
         }
      }
      else
      {
         for (int i = 0; i < mSize && success; ++i)
         {
            for (int j = 0; j < mSize; ++j)
            {
               if (!net.receiveData(&(this->mRows[i][j]), sizeof(T)))
               {
                  std::cerr << Red << "Server closed connection: ReadNet\n";
                  std::cerr << "ERROR: " << net.strError << RCol << std::endl;
                  success = false;
                  break;
               }
            }
         }
      }
      return success;
   }
   
   /************************************************************************
   * Function to write data from entire matrix to a network socket
   ***********************************************************************/
   //bool writeNet(Connection& net) const
   bool writeNet(Connection& net, bool reduceSize = false) const
   {
      bool success = true;
      // It may be duplication, but it will bigger branch misprediction problems
      if (reduceSize)
      {
         short temp;
         for (int i = 0; i < mSize && success; ++i)
         {
            for (int j = 0; j < mSize; ++j)
            {
               temp = (short)(this->mRows[i][j]);
               if (!net.sendData(&temp, sizeof(short)))
               {
                  std::cerr << Red << "Server closed connection: WriteNet\n";
                  std::cerr << "ERROR: " << net.strError << RCol << std::endl;
                  success = false;
                  break;
               }
            }
         }
      }
      else
      {
         for (int i = 0; i < mSize && success; ++i)
         {
            for (int j = 0; j < mSize; ++j)
            {
               if (!net.sendData(&(this->mRows[i][j]), sizeof(T)))
               {
                  std::cerr << Red << "Server closed connection: WriteNet\n";
                  std::cerr << "ERROR: " << net.strError << RCol << std::endl;
                  success = false;
                  break;
               }
            }
         }
      }
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
      // Prevent gross inflation of memory requirements for matrices larger than 4096
      matrixA.thread_Stop = 2048;
      matrixA.thread_Start = matrixA.thread_Stop * 2;
   }
   
   // Receive matrix A and Receive matrix B
   //if (matrixA.readNet(net) && matrixB.readNet(net))
   if (matrixA.readNet(net, true) && matrixB.readNet(net, true))
   {
      {
      // create some inner scoping for the lock
      // We will allow multiple sends/receives at a time, but no doubling up on computation...
      //std::lock_guard<std::mutex> lock(oneAtATime);
      std::cerr << Gre << "STATUS: Multiplying matrices!!!" << RCol << "\n";
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
