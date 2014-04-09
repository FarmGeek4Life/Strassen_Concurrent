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
#include <unistd.h>
#include "errorColors.h"
using namespace std;

template <class T>
class Matrix
{
private:
   T** mRows;
   int mSize;
   bool colAlloc;
   bool rowAlloc;
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
      started = false;
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
      started = false;
      badMutexGrab = false;
      joined = false;
   }

   Matrix<T>(const Matrix<T>& matrixB)
   {
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
      started = false;
      badMutexGrab = false;
      joined = false;
   }

   Matrix<T> steal(Matrix<T>& matrixB)
   {
      mSize = matrixB.getSize();
      colAlloc = true;
      rowAlloc = true;
      
      // Steal the pointers, instead of copying data...
      mRows = matrixB.mRows;
      matrixB.colAlloc = false;
      matrixB.rowAlloc = false;
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
   
   std::mutex mMutex;
   std::mutex mCVMutex;
   std::mutex mWMutex;
   
   /********************************************************************************************
   * Deadlock-avoiding code for preventing duplicate attempts to join a thread
   *    Also prevents code from continuing while a thread has not been joined
   *    Unfortunately, there appears to be some sort of race condition with this that
   *    prevents the use of the opportunistic math with the standard multiplication.
   ********************************************************************************************/
   void safetyJoin(std::thread* pThread)
   {
      // Grab the lock. Hopefully the thread grabbed it first.
      std::lock_guard<std::mutex> lk(mMutex);
      if (!started && !badMutexGrab)
      {
         // Uh oh, we got the lock before the thread did.
         std::cerr << Red << "Bad mutex grab A!!!" << RCol << std::endl;
         // Set some status, and release the lock
         started = true;
         badMutexGrab = true;
         mMutex.unlock();
      }
      else if (badMutexGrab)
      {
         // We managed to get the lock twice before the thread did.
         std::cerr << Red << "Bad mutex grab looping A!!!" << RCol << std::endl;
         // Release the lock and let the thread take it.
         mMutex.unlock();
         // Almost busy-loop... We want other threads to go
         while (!joined)
         {
            usleep(50000); // Sleep 50 milliseconds
         }
      }
      // This is true if a thread is joinable
      // a thread is not joinable if join() has already been called on it
      if (pThread->joinable())
      {
         try
         {
            pThread->join();
            joined = true;
         }
         catch (std::system_error &e)
         {
            std::cerr << "Caught std::system_error!\n";
         }
      }
   }
   
   /********************************************************************************************
   * Opportunistic math: as soon as the necessary threads complete, do the math
   ********************************************************************************************/
   Matrix<T>& op00_11_con(Matrix<T>& matrixA, Matrix<T>& matrixB, Matrix<T>& matrixC, Matrix<T>& matrixD, std::thread t[], int tA, int tB, int tC, int tD)
   {
      matrixA.safetyJoin(&t[tA]);
      matrixB.safetyJoin(&t[tB]);
      matrixC.safetyJoin(&t[tC]);
      matrixD.safetyJoin(&t[tD]);
      //{
      //std::lock_guard<std::mutex> lkA(matrixA.mMutex);
      //if (!matrixA.started && !matrixA.badMutexGrab)
      //{
      //   cerr << "Bad mutex grab A!!!" << endl;
      //   matrixA.started = true;
      //   matrixA.badMutexGrab = true;
      //   matrixA.mMutex.unlock();
      //}
      //else if (matrixA.badMutexGrab)
      //{
      //   cerr << "Bad mutex grab looping A!!!" << endl;
      //   matrixA.mMutex.unlock();
      //   while (!matrixA.joined)
      //   {
      //      usleep(50000); // Sleep 50 milliseconds
      //      std::lock_guard<std::mutex> lkA1(matrixA.mMutex);
      //   }
      //}
      //if (!matrixA.joined)
      //{
      //      t[tA].join();
      //      matrixA.joined = true;
      //}
      //if (t[tA].joinable())
      //{
      //   //std::cerr << "Passed wait: " << tA << "\n";
      //   try
      //   {
      //      t[tA].join();
      //      matrixA.joined = true;
      //   }
      //   catch (std::system_error &e)
      //   {
      //      std::cerr << "Caught std::system_error!\n";
      //   }
      //}
      //else
      //{
      //   if (!matrixA.joined)
      //   {
      //      std::cerr << "Passed join: A, " << tA << "\n";
      //   }
      //   //std::cerr << "Skipped join: " << tA << "\n";
      //   while (!matrixA.joined)
      //   {
      //      usleep(50000); // Sleep 50 milliseconds
      //   }
      //}
      //}
      //{
      //std::lock_guard<std::mutex> lkB(matrixB.mMutex);
      //if (!matrixB.started && !matrixB.badMutexGrab)
      //{
      //   cerr << "Bad mutex grab B!!!" << endl;
      //   matrixB.started = true;
      //   matrixB.badMutexGrab = true;
      //   matrixB.mMutex.unlock();
      //}
      //else if (matrixB.badMutexGrab)
      //{
      //   cerr << "Bad mutex grab looping B!!!" << endl;
      //   matrixB.mMutex.unlock();
      //   while (!matrixB.joined)
      //   {
      //      usleep(50000); // Sleep 50 milliseconds
      //      std::lock_guard<std::mutex> lkB1(matrixB.mMutex);
      //   }
      //}
      //if (!matrixB.joined)
      //{
      //      t[tB].join();
      //      matrixB.joined = true;
      //}
      //if (t[tB].joinable())
      //{
      //   if (!matrixB.joined)
      //   {
      //      std::cerr << "Passed join: B, " << tB << "\n";
      //   }
      //   //std::cerr << "Passed wait: " << tB << "\n";
      //   try
      //   {
      //      t[tB].join();
      //      matrixB.joined = true;
      //   }
      //   catch (std::system_error &e)
      //   {
      //      std::cerr << "Caught std::system_error!\n";
      //   }
      //}
      //else
      //{
      //   //std::cerr << "Skipped join: " << tB << "\n";
      //   while (!matrixB.joined)
      //   {
      //      usleep(50000); // Sleep 50 milliseconds
      //   }
      //}
      //}
      //{
      //std::lock_guard<std::mutex> lkC(matrixC.mMutex);
      //if (!matrixC.started && !matrixC.badMutexGrab)
      //{
      //   cerr << "Bad mutex grab C!!!" << endl;
      //   matrixC.started = true;
      //   matrixC.badMutexGrab = true;
      //   matrixC.mMutex.unlock();
      //}
      //else if (matrixC.badMutexGrab)
      //{
      //   cerr << "Bad mutex grab looping C!!!" << endl;
      //   matrixC.mMutex.unlock();
      //   while (!matrixC.joined)
      //   {
      //      usleep(50000); // Sleep 50 milliseconds
      //      std::lock_guard<std::mutex> lkC1(matrixC.mMutex);
      //   }
      //}
      //if (!matrixC.joined)
      //{
      //      t[tC].join();
      //      matrixC.joined = true;
      //}
      //if (t[tC].joinable())
      //{
      //   //std::cerr << "Passed wait: " << tC << "\n";
      //   try
      //   {
      //      t[tC].join();
      //      matrixC.joined = true;
      //   }
      //   catch (std::system_error &e)
      //   {
      //      std::cerr << "Caught std::system_error!\n";
      //   }
      //}
      //else
      //{
      //   if (!matrixC.joined)
      //   {
      //      std::cerr << "Passed join: C, " << tC << "\n";
      //   }
      //   //std::cerr << "Skipped join: " << tC << "\n";
      //   while (!matrixC.joined)
      //   {
      //      usleep(50000); // Sleep 50 milliseconds
      //   }
      //}
      //}
      //{
      //std::lock_guard<std::mutex> lkD(matrixD.mMutex);
      //if (!matrixD.started && !matrixD.badMutexGrab)
      //{
      //   cerr << "Bad mutex grab D!!!" << endl;
      //   matrixD.started = true;
      //   matrixD.badMutexGrab = true;
      //   matrixD.mMutex.unlock();
      //}
      //else if (matrixD.badMutexGrab)
      //{
      //   cerr << "Bad mutex grab looping D!!!" << endl;
      //   matrixD.mMutex.unlock();
      //   while (!matrixD.joined)
      //   {
      //      usleep(50000); // Sleep 50 milliseconds
      //      std::lock_guard<std::mutex> lkD1(matrixD.mMutex);
      //   }
      //}
      //if (!matrixD.joined)
      //{
      //      t[tD].join();
      //      matrixD.joined = true;
      //}
      //if (t[tD].joinable())
      //{
      //   //std::cerr << "Passed wait: " << tD << "\n";
      //   try
      //   {
      //      t[tD].join();
      //      matrixD.joined = true;
      //   }
      //   catch (std::system_error &e)
      //   {
      //      std::cerr << "Caught std::system_error!\n";
      //   }
      //}
      //else
      //{
      //   if (!matrixD.joined)
      //   {
      //      std::cerr << "Passed join: D, " << tD << "\n";
      //   }
      //   //std::cerr << "Skipped join: " << tD << "\n";
      //   while (!matrixD.joined)
      //   {
      //      usleep(50000); // Sleep 50 milliseconds
      //   }
      //}
      //}
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
      matrixA.safetyJoin(&t[tA]);
      matrixB.safetyJoin(&t[tB]);
      //{
      //std::lock_guard<std::mutex> lkA(matrixA.mMutex);
      //if (!matrixA.started && !matrixA.badMutexGrab)
      //{
      //   cerr << "Bad mutex grab A!!!" << endl;
      //   matrixA.started = true;
      //   matrixA.badMutexGrab = true;
      //   matrixA.mMutex.unlock();
      //}
      //else if (matrixA.badMutexGrab)
      //{
      //   cerr << "Bad mutex grab looping A!!!" << endl;
      //   matrixA.mMutex.unlock();
      //   while (!matrixA.joined)
      //   {
      //      usleep(50000); // Sleep 50 milliseconds
      //      std::lock_guard<std::mutex> lkA1(matrixA.mMutex);
      //   }
      //}
      //if (!matrixA.joined)
      //{
      //      t[tA].join();
      //      matrixA.joined = true;
      //}
      //if (t[tA].joinable())
      //{
      //   //std::cerr << "Passed wait: " << tA << "\n";
      //   try
      //   {
      //      t[tA].join();
      //      matrixA.joined = true;
      //   }
      //   catch (std::system_error &e)
      //   {
      //      std::cerr << "Caught std::system_error!\n";
      //   }
      //}
      //else
      //{
      //   if (!matrixA.joined)
      //   {
      //      std::cerr << "Passed join: A, " << tA << "\n";
      //   }
      //   //std::cerr << "Skipped join: " << tA << "\n";
      //   while (!matrixA.joined)
      //   {
      //      usleep(50000); // Sleep 50 milliseconds
      //   }
      //}
      //}
      //{
      //std::lock_guard<std::mutex> lkB(matrixB.mMutex);
      //if (!matrixB.started && !matrixB.badMutexGrab)
      //{
      //   cerr << "Bad mutex grab B!!!" << endl;
      //   matrixB.started = true;
      //   matrixB.badMutexGrab = true;
      //   matrixB.mMutex.unlock();
      //}
      //else if (matrixB.badMutexGrab)
      //{
      //   cerr << "Bad mutex grab looping B!!!" << endl;
      //   matrixB.mMutex.unlock();
      //   while (!matrixB.joined)
      //   {
      //      usleep(50000); // Sleep 50 milliseconds
      //      std::lock_guard<std::mutex> lkB1(matrixB.mMutex);
      //   }
      //}
      //if (!matrixB.joined)
      //{
      //      t[tB].join();
      //      matrixB.joined = true;
      //}
      //if (t[tB].joinable())
      //{
      //   //std::cerr << "Passed wait: " << tB << "\n";
      //   try
      //   {
      //      t[tB].join();
      //      matrixB.joined = true;
      //   }
      //   catch (std::system_error &e)
      //   {
      //      std::cerr << "Caught std::system_error!\n";
      //   }
      //}
      //else
      //{
      //   if (!matrixB.joined)
      //   {
      //      std::cerr << "Passed join: B, " << tB << "\n";
      //   }
      //   //std::cerr << "Skipped join: " << tB << "\n";
      //   while (!matrixB.joined)
      //   {
      //      usleep(50000); // Sleep 50 milliseconds
      //   }
      //}
      //}
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
   
   // Starting point for thread creation
   static int thread_Start;
   // Stopping point for thread creation
   static int thread_Stop;
   // Limit the number of simultaneous threads
   static std::mutex threadLimiter[100];
   static int maxThreads;
   static int threadCount;
   
   /**************************************************************************
    * Wrapper to improve parallelization of Strassen's algorithm
    * 
    *************************************************************************/
   void mult_Fast(Matrix<T>* a0, Matrix<T>* a1, bool addA, Matrix<T>* b0, Matrix<T>* b1, bool addB)
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
      mult(matrixB, true);
   }
   
   /**************************************************************************
    * Matrix multiplication using Strassen's algorithm.
    * The input matrices must be equal in size and square, 
    *    and the size must be n x n, where n is a power of 2
    *************************************************************************/
   void mult(Matrix<T>& matrixB, bool wrapped = false)
   {
      // Take over the lock for this matrix.....
      std::mutex* pMutex = &mMutex;
      if (wrapped)
      {
         pMutex = &mWMutex;
      }
      std::lock_guard<std::mutex> lk(*pMutex);
      started = true;
      
      if (mSize < 512 && mSize > 1 && mSize <= thread_Stop)
      {
         // This is, oddly enough, about 1 second faster for 2048x2048...
         Matrix<T> mA(*this);
         multStandard(mA, matrixB);
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
         // m3 =  a00        * (b01 - b11);
         // m4 =  a11        * (b10 - b00);
         // m5 = (a00 + a01) *  b11;
         // m6 = (a10 - a00) * (b00 + b01);
         // m7 = (a01 - a11) * (b10 + b11);
         
         // Split for the thread number optimization
         if (mSize > thread_Start)
         {
            // Matrix is too large. Slow down to save memory...
            m1.mult_Fast(&a00, &a11 , true , &b00, &b11 , true );
            m2.mult_Fast(&a10, &a11 , true , &b00,  null, false);
            m3.mult_Fast(&a00,  null, false, &b01, &b11 , false);
            m4.mult_Fast(&a11,  null, false, &b10, &b00 , false);
            m5.mult_Fast(&a00, &a01 , true , &b11,  null, false);
            m6.mult_Fast(&a10, &a00 , false, &b00, &b01 , true );
            m7.mult_Fast(&a01, &a11 , false, &b10, &b11 , true );
         }
         else if (mSize > thread_Stop)
         {
            t[1] = std::thread(&Matrix<T>::mult_Fast, &m1, &a00, &a11 , true , &b00, &b11 , true );
            t[2] = std::thread(&Matrix<T>::mult_Fast, &m2, &a10, &a11 , true , &b00,  null, false);
            t[3] = std::thread(&Matrix<T>::mult_Fast, &m3, &a00,  null, false, &b01, &b11 , false);
            t[4] = std::thread(&Matrix<T>::mult_Fast, &m4, &a11,  null, false, &b10, &b00 , false);
            t[5] = std::thread(&Matrix<T>::mult_Fast, &m5, &a00, &a01 , true , &b11,  null, false);
            t[6] = std::thread(&Matrix<T>::mult_Fast, &m6, &a10, &a00 , false, &b00, &b01 , true );
            t[7] = std::thread(&Matrix<T>::mult_Fast, &m7, &a01, &a11 , false, &b10, &b11 , true );
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
            m1.mult_Fast(&a00, &a11 , true , &b00, &b11 , true );
            m2.mult_Fast(&a10, &a11 , true , &b00,  null, false);
            m3.mult_Fast(&a00,  null, false, &b01, &b11 , false);
            m4.mult_Fast(&a11,  null, false, &b10, &b00 , false);
            m5.mult_Fast(&a00, &a01 , true , &b11,  null, false);
            m6.mult_Fast(&a10, &a00 , false, &b00, &b01 , true );
            m7.mult_Fast(&a01, &a11 , false, &b10, &b11 , true );
         }
         else
         {
            // This is entered at one level, and it appears to work faster than a single standard multiplication that is larger
            m1.multStandard_Opt(&a00, &a11 , true , &b00, &b11 , true );
            m2.multStandard_Opt(&a10, &a11 , true , &b00,  null, false);
            m3.multStandard_Opt(&a00,  null, false, &b01, &b11 , false);
            m4.multStandard_Opt(&a11,  null, false, &b10, &b00 , false);
            m5.multStandard_Opt(&a00, &a01 , true , &b11,  null, false);
            m6.multStandard_Opt(&a10, &a00 , false, &b00, &b01 , true );
            m7.multStandard_Opt(&a01, &a11 , false, &b10, &b11 , true );
         }
         
         //// Use the 7 multiplication results to get the results for each quadrant
         //// Save on memory usage by reusing one set of quadrants
         ////a00 = m1 + m4 - m5 + m7;
         ////a01 = m3 + m5;
         ////a10 = m2 + m4;
         ////a11 = m1 + m3 - m2 + m6;
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
   }
   
   /**************************************************************************
    * Standard matrix multiplication, designed for use with Strassen's
    *    algorithm for matrix multiplication
    * Designed to be optimized for threading
    *************************************************************************/
   void multStandard_Opt(Matrix<T>* a0, Matrix<T>* a1, bool addA, Matrix<T>* b0, Matrix<T>* b1, bool addB)
   {
      // Take over the lock for this matrix.....
      std::lock_guard<std::mutex> lk(mMutex);
      started = true;
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
   }
   
   /*********************************************************************
   * Standard Matrix Multiplication
   *********************************************************************/
   void multStandard(Matrix<T>& matrixA, Matrix<T>& matrixB)
   {
      started = true;
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
};

// Initialize the static variables for Matrix
template <class T>
int Matrix<T>::thread_Start = 8192;
template <class T>
int Matrix<T>::thread_Stop = 0;
template <class T>
std::mutex Matrix<T>::threadLimiter[100];
template <class T>
int Matrix<T>::maxThreads = 100;
template <class T>
int Matrix<T>::threadCount = 0;

template <class T>
istream& operator>>(istream& is, const Matrix<T>& m)
{
   m.read(is);
   return is;
}

template <class T>
ostream& operator<< (ostream& os, const Matrix<T>& m)
{
   m.write(os);
   return os;
}

/****************************************************************************
* Read in a matrix from a file
****************************************************************************/
void readMatrix(Matrix<int>& matrix, string file, bool& error)
{
   error = false;
   ifstream inFile;
   inFile.open(file.c_str());
   
   if (inFile.is_open())
   {
      inFile >> matrix;
      inFile.close();
   }
   else 
   {
      cerr << "Unable to open " + file << endl;
      //return 1;
      error = true;
   }
}

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
   thread m1 = thread(readMatrix, std::ref(matrixA), file, std::ref(error1));
   thread m2 = thread(readMatrix, std::ref(matrixB), file2, std::ref(error2));
   m1.join();
   m2.join();
   if (error1 || error2)
   {
      return 1;
   }
   
   // Perform the multiplication
   matrixA.mult(matrixB);
   // Output
   cout << matrixA;

   return 0;
}
