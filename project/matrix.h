/***************************************************************************
* Program:
*    Matrix class, Senior Project
* Author:
*    Bryson Gibbons
* Summary:
*    Provide matrix multiplication functionality and support functions
*    for performing distributed matrix multiplication
*    
*    Makes use of both standard matrix multiplication and
*       Strassen's algorithm.
*    
***************************************************************************/

#ifndef _MATRIX_H
#define _MATRIX_H

#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <unistd.h>
#include <fstream>
#include "connection.h"
#include "errorColors.h"


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
   // Variables that cannot be made private and still function as needed
   std::mutex mMutex;
   std::mutex mWMutex;
   std::thread mThread;
   
   // Output the entire matrix on error:
   static bool NetError;
   // Starting point for thread creation
   static int thread_Start;
   // Stopping point for thread creation
   static int thread_Stop;
   // Allow for thread numbering - Used by the client/manager portion of code
   static int sysCounter;
   static std::mutex sysCounter_Mutex;
   // Limit the number of simultaneous threads - Used by server/slave portion of code
   static std::mutex threadLimiter[100];
   static int maxThreads;
   
   /*********************************************************************
   * Constructor: Allow the creation of the matrix without allocating memory
   * This is primarily for reducing looping
   *********************************************************************/
   Matrix<T>(int size, bool alloc)
   {
      // If we want memory allocated
      if (alloc)
      {
         // Allocate
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
   
   /*********************************************************************
   * Constructor: Initialize and allocate memory for the object
   *********************************************************************/
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
      badMutexGrab = false;
      joined = false;
   }
   
   /*********************************************************************
   * Copy Constructor
   *********************************************************************/
   Matrix<T>(const Matrix<T>& matrixB)
   {
      mSize = matrixB.mSize;
      colAlloc = true;
      rowAlloc = true;
      finished = false;
      started = false;
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
      // If allocated at all
      if (colAlloc || rowAlloc)
      {
         // exit
         return;
      }
      mSize = matrixB.getSize();
      colAlloc = true;
      rowAlloc = true;
      
      mRows = new T*[mSize];
      // Reduce branch misprediction by copying code...
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
   void allocMath(const Matrix<T>& matrixA, const Matrix<T>& matrixB, bool add)
   {
      if (colAlloc || rowAlloc)
      {
         return;
      }
      mSize = matrixB.mSize;
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
   
   /********************************************************************************************
   * Bracket operator: allow use of matrix object as a 2-dimensional array
   * 'const' does not prevent manipulation of data, just prevent manipulation of array pointers
   ********************************************************************************************/
   T* operator[](int row) const
   {
      return mRows[row];
   }
   
   int getSize() const
   {
      return mSize;
   }
   
   /********************************************************************************************
   * Read from an input stream
   ********************************************************************************************/
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
   
   /********************************************************************************************
   * Write to an output stream
   ********************************************************************************************/
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
   
   /********************************************************************************************
   * Deadlock-avoiding code for preventing duplicate attempts to join a thread
   *    Also prevents code from continuing while a thread has not been joined
   *    Unfortunately, there appears to be some sort of race condition with this that
   *    prevents the use of the opportunistic math with the standard multiplication.
   ********************************************************************************************/
   void safetyJoin()
   {
      // Logic to prevent continuation until the specified threads complete
      // It looks bad, but it works (unlike everything else I tried)
      // The ugly logic is necessary when the processor is loaded
      // Grab the lock. Hopefully the thread grabbed it first.
      std::lock_guard<std::mutex> lk(mMutex);
      if (!started && !badMutexGrab)
      {
         // Uh oh, we got the lock before the thread did.
         std::cerr << Red << "Bad mutex grab A!!!" << RCol << std::endl;
         // change variables to reflect the error
         started = true;
         badMutexGrab = true;
         // Drop the mutex like it's hot to prevent deadlock (on thread join)
         mMutex.unlock();
      }
      else if (badMutexGrab)
      {
         // We managed to get the lock twice before the thread did.
         std::cerr << Red << "Bad mutex grab looping A!!!" << RCol << std::endl;
         // Release the lock and let the thread take it.
         mMutex.unlock();
         // Don't quite busy loop; this will force other threads to go.
         while (!joined)
         {
            usleep(50000); // Sleep 50 milliseconds
         }
      }
      // This is true if a thread is joinable
      // a thread is not joinable if join() has already been called on it
      if (mThread.joinable())
      {
         try
         {
            mThread.join();
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
   Matrix<T>& op00_11_con(Matrix<T>& matrixA, Matrix<T>& matrixB, Matrix<T>& matrixC, Matrix<T>& matrixD)
   {
      matrixA.safetyJoin();
      matrixB.safetyJoin();
      matrixC.safetyJoin();
      matrixD.safetyJoin();
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
   Matrix<T>& op01_10_con(Matrix<T>& matrixA, Matrix<T>& matrixB)
   {
      matrixA.safetyJoin();
      matrixB.safetyJoin();
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
   
   /********************************************************************************************
   * Perform the addition/subtraction of the 4 matrices for results. (Strassen's algorithm)
   ********************************************************************************************/
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
   
   /********************************************************************************************
   * Perform the addition of the 2 matrices for results. (Strassen's algorithm)
   ********************************************************************************************/
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
   
   /**************************************************************************
    * Wrapper to improve parallelization of Strassen's algorithm
    * This one is for the client/manager side
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
    * Matrix multiplication using Strassen's algorithm, distributed
    * Used by the client/manager side
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
         // m1 = (a00 + a11) * (b00 + b11)
         // m2 = (a10 + a11) *  b00
         // m3 =  a00 *        (b01 - b11)
         // m4 =  a11 *        (b10 - b00)
         // m5 = (a00 + a01) *  b11
         // m6 = (a10 - a00) * (b00 + b01)
         // m7 = (a01 - a11) * (b10 + b11)
         
         // It makes no sense to split into smaller chunks for 7 computers
         if (mSize > thread_Stop && numComputers != 7 && numComputers != 1)
         {
            m1.mThread = std::thread(&Matrix<T>::mult_Fast_Farm, &m1, &a00, &a11 , true , &b00, &b11 , true , computers, numComputers, port);
            m2.mThread = std::thread(&Matrix<T>::mult_Fast_Farm, &m2, &a10, &a11 , true , &b00,  null, false, computers, numComputers, port);
            m3.mThread = std::thread(&Matrix<T>::mult_Fast_Farm, &m3, &a00,  null, false, &b01, &b11 , false, computers, numComputers, port);
            m4.mThread = std::thread(&Matrix<T>::mult_Fast_Farm, &m4, &a11,  null, false, &b10, &b00 , false, computers, numComputers, port);
            m5.mThread = std::thread(&Matrix<T>::mult_Fast_Farm, &m5, &a00, &a01 , true , &b11,  null, false, computers, numComputers, port);
            m6.mThread = std::thread(&Matrix<T>::mult_Fast_Farm, &m6, &a10, &a00 , false, &b00, &b01 , true , computers, numComputers, port);
            m7.mThread = std::thread(&Matrix<T>::mult_Fast_Farm, &m7, &a01, &a11 , false, &b10, &b11 , true , computers, numComputers, port);
         }
         else
         {
            // Mutex: control the access to the system counter (sysCounter) for even distribution
            std::lock_guard<std::mutex> lock(sysCounter_Mutex);
            // sysCounter incrementation is outside of function calls to guarantee consistency
            m1.mThread = std::thread(&Matrix<T>::runParallel_Fast, &m1, &a00, &a11 , true , &b00, &b11 , true , computers[sysCounter % numComputers], port, sysCounter);
            ++sysCounter;
            m2.mThread = std::thread(&Matrix<T>::runParallel_Fast, &m2, &a10, &a11 , true , &b00,  null, false, computers[sysCounter % numComputers], port, sysCounter);
            ++sysCounter;
            m3.mThread = std::thread(&Matrix<T>::runParallel_Fast, &m3, &a00,  null, false, &b01, &b11 , false, computers[sysCounter % numComputers], port, sysCounter);
            ++sysCounter;
            m4.mThread = std::thread(&Matrix<T>::runParallel_Fast, &m4, &a11,  null, false, &b10, &b00 , false, computers[sysCounter % numComputers], port, sysCounter);
            ++sysCounter;
            m5.mThread = std::thread(&Matrix<T>::runParallel_Fast, &m5, &a00, &a01 , true , &b11,  null, false, computers[sysCounter % numComputers], port, sysCounter);
            ++sysCounter;
            m6.mThread = std::thread(&Matrix<T>::runParallel_Fast, &m6, &a10, &a00 , false, &b00, &b01 , true , computers[sysCounter % numComputers], port, sysCounter);
            ++sysCounter;
            m7.mThread = std::thread(&Matrix<T>::runParallel_Fast, &m7, &a01, &a11 , false, &b10, &b11 , true , computers[sysCounter % numComputers], port, sysCounter);
            ++sysCounter;
         }
         
         // Use the 7 multiplication results to get the results for each quadrant
         // Save on memory usage by reusing one set of quadrants
         // Also saves on time - no de/reallocation
         // a00 = m1 + m4 - m5 + m7
         // a01 = m3 + m5
         // a10 = m2 + m4
         // a11 = m1 + m3 - m2 + m6
         // These functions write to axx quadrants, which will overwrite the data in *this
         std::thread r00 = std::thread(&Matrix<T>::op00_11_con, &a00, std::ref(m1), std::ref(m4), std::ref(m5), std::ref(m7));
         std::thread r01 = std::thread(&Matrix<T>::op01_10_con, &a01, std::ref(m3), std::ref(m5));
         std::thread r10 = std::thread(&Matrix<T>::op01_10_con, &a10, std::ref(m2), std::ref(m4));
         std::thread r11 = std::thread(&Matrix<T>::op00_11_con, &a11, std::ref(m1), std::ref(m3), std::ref(m2), std::ref(m6));
         r00.join();
         r01.join();
         r10.join();
         r11.join();
      }
      else
      {
         mRows[0][0] = mRows[0][0] * matrixB[0][0];
      }
      finished = true;
   }
   
   /**************************************************************************
    * Wrapper to improve parallelization of Strassen's algorithm - distribution
    * This is for the client/manager side
    *************************************************************************/
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
    * Network communication function for distributing Strassen's algorithm
    * client/manager side
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
            // Send control data (thread id and matrix size, right now)
            if (!net.sendData(&controlData, 4 * 5))
            {
               std::cerr << Red << "Server closed connection: " << computer << "\n";
               std::cerr << "ERROR (" << computer << "): " << net.strError << RCol << "\n";
               failed = true;
            }
            // Send matrix A
            // Since the input is a char, and some math is done, send as a short to save on time
            if (failed || !this->writeNet(net, true))
            {
               std::cerr << Red << "Server closed connection: " << computer << "\n";
               std::cerr << "ERROR (" << computer << "): " << net.strError << RCol << "\n";
               failed = true;
            }
            // Send matrix B
            // Since the input is a char, and some math is done, send as a short to save on time
            if (failed || !matrixB.writeNet(net, true))
            {
               std::cerr << Red << "Server closed connection: " << computer << "\n";
               std::cerr << "ERROR (" << computer << "): " << net.strError << RCol << "\n";
               failed = true;
            }
            // Erase the allocated memory of matrixB. It's not needed any more.
            matrixB.erase();
            // Receive Result
            // Read as an int - it will likely be bigger than a short
            if (failed || !this->readNet(net))
            {
               std::cerr << Red << "Server closed connection: " << computer << "\n";
               std::cerr << "ERROR (" << computer << "): " << net.strError << RCol << "\n";
               failed = true;
            }
            // Close the connection cleanly
            net.closeComm();
            // If we had a problem, set the static variable for reporting purposes
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
    * Wrapper to improve parallelization of Strassen's algorithm
    * This is for the server/slave side, and local computation
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
    * When the matrix is small enough, standard matrix multiplication is used
    * It is significantly faster than Strassen's algorithm for small matrices
    * This is used by the server/slave side, and for local computation
    *************************************************************************/
   void mult_FarmSlave(Matrix<T>& matrixB, bool wrapped = false)
   {
      if (mSize < 512 && mSize > 1 && mSize <= thread_Stop)
      {
         // This is, oddly enough, about 1 second faster for 2048x2048...
         Matrix<T> mA(*this);
         multStandard(mA, matrixB);
      }
      else if (mSize > 1)
      {
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
         // m1 = (a00 + a11) * (b00 + b11)
         // m2 = (a10 + a11) *  b00
         // m3 =  a00 *        (b01 - b11)
         // m4 =  a11 *        (b10 - b00)
         // m5 = (a00 + a01) *  b11
         // m6 = (a10 - a00) * (b00 + b01)
         // m7 = (a01 - a11) * (b10 + b11)
         
         // Split for the thread number optimization
         if (mSize > thread_Start)
         {
            // Matrix is too large. Slow down to save memory...
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
            // Run threaded
            m1.mThread = std::thread(&Matrix<T>::mult_Fast_Slave, &m1, &a00, &a11 , true , &b00, &b11 , true );
            m2.mThread = std::thread(&Matrix<T>::mult_Fast_Slave, &m2, &a10, &a11 , true , &b00,  null, false);
            m3.mThread = std::thread(&Matrix<T>::mult_Fast_Slave, &m3, &a00,  null, false, &b01, &b11 , false);
            m4.mThread = std::thread(&Matrix<T>::mult_Fast_Slave, &m4, &a11,  null, false, &b10, &b00 , false);
            m5.mThread = std::thread(&Matrix<T>::mult_Fast_Slave, &m5, &a00, &a01 , true , &b11,  null, false);
            m6.mThread = std::thread(&Matrix<T>::mult_Fast_Slave, &m6, &a10, &a00 , false, &b00, &b01 , true );
            m7.mThread = std::thread(&Matrix<T>::mult_Fast_Slave, &m7, &a01, &a11 , false, &b10, &b11 , true );
            // Join with the threads
            m1.mThread.join();
            m2.mThread.join();
            m3.mThread.join();
            m4.mThread.join();
            m5.mThread.join();
            m6.mThread.join();
            m7.mThread.join();
         }
         else if (mSize > 512)
         {
            // Strassen's algorithm is faster for any matrix above 512
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
            // I tried to thread this, but it just didn't agree - race condition problems
            // Also, the improvement is inconsistent
            m1.multStandard_Opt(&a00, &a11 , true , &b00, &b11 , true );
            m2.multStandard_Opt(&a10, &a11 , true , &b00,  null, false);
            m3.multStandard_Opt(&a00,  null, false, &b01, &b11 , false);
            m4.multStandard_Opt(&a11,  null, false, &b10, &b00 , false);
            m5.multStandard_Opt(&a00, &a01 , true , &b11,  null, false);
            m6.multStandard_Opt(&a10, &a00 , false, &b00, &b01 , true );
            m7.multStandard_Opt(&a01, &a11 , false, &b10, &b11 , true );
         }
         
         // Use the 7 multiplication results to get the results for each quadrant
         // Save on memory usage by reusing one set of quadrants
         //a00 = m1 + m4 - m5 + m7
         //a01 = m3 + m5
         //a10 = m2 + m4
         //a11 = m1 + m3 - m2 + m6
         // These functions write to axx quadrants, which will overwrite the data in *this
         a00.op00_11(m1, m4, m5, m7);
         a01.op01_10(m3, m5);
         a10.op01_10(m2, m4);
         a11.op00_11(m1, m3, m2, m6);
      }
      else
      {
         // Assume a matrix of size 1
         mRows[0][0] = mRows[0][0] * matrixB[0][0];
      }
      finished = true;
   }
   
   /**************************************************************************
    * Standard matrix multiplication, designed for use with Strassen's
    *    algorithm for matrix multiplication
    * Designed to be optimized for threading
    *************************************************************************/
   void multStandard_Opt(Matrix<T>* a0, Matrix<T>* a1, bool addA, Matrix<T>* b0, Matrix<T>* b1, bool addB)
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
      // Perform the matrix multiplication
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
   * Standard Matrix Multiplication
   *********************************************************************/
   void multStandard(const Matrix<T>& matrixA, const Matrix<T>& matrixB)
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
   bool readNet(Connection& net, bool reduceSize = false)
   {
      bool success = true;
      // It may be duplication, but it will prevent bigger branch misprediction problems
      if (reduceSize)
      {
         // Read the data as a short, instead of an int....
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
         // Read the data as an int
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
   bool writeNet(Connection& net, bool reduceSize = false) const
   {
      bool success = true;
      // It may be duplication, but it will prevent bigger branch misprediction problems
      if (reduceSize)
      {
         // Send the data as a short, instead of an int....
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
         // Send the data as an int
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

/*********************************************************************
* Overloaded input operator for Matrix
*********************************************************************/
template <class T>
std::istream& operator>>(std::istream& is, const Matrix<T>& m)
{
   m.read(is);
   return is;
}

/*********************************************************************
* Overloaded output operator for Matrix
*********************************************************************/
template <class T>
std::ostream& operator<< (std::ostream& os, const Matrix<T>& m)
{
   m.write(os);
   return os;
}

/****************************************************************************
* Read in a matrix from a file
****************************************************************************/
template <class T>
void readMatrix(Matrix<T>& matrix, std::string file, bool& error)
{
   error = false;
   std::ifstream inFile;
   inFile.open(file.c_str());
   
   if (inFile.is_open())
   {
      inFile >> matrix;
      inFile.close();
   }
   else 
   {
      std::cerr << Red << "Unable to open " + file << RCol << std::endl;
      error = true;
   }
}

// Mutexes used by the following functions for the server/slave side
std::mutex byMultiples[10];
std::mutex oneAtATime;

/************************************************************************
* Network communication manager for the server/slave side
* Several of these will run in parallel
***********************************************************************/
template <class T>
void threadedManager(int socket, unsigned int id)
{
   std::cerr << Gre << "STARTING EXECUTION: THREAD " << id << RCol << "\n";
   Connection net(socket);
   int threadId = 0;
   int controlData[5] = {0, 0, 0, 0, 0};
   int close[5] = {0, 0, 0, 0, 0};
   // Read control data - thread id, matrix size
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
   // Set limits on when threading starts and stops
   // The benefits are negligible on small matrices, but will greatly decrease memory
   //    usage on large matrices, with time benefits as well.
   // big advantage first seen with 8192x8192
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
   // Receive them as shorts, not as ints.
   if (matrixA.readNet(net, true) && matrixB.readNet(net, true))
   {
      std::cerr << Gre << "STATUS: Multiplying matrices!!!" << RCol << "\n";
      matrixA.mult_FarmSlave(matrixB);
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

/************************************************************************
* Server/slave management function.
* Accepts incoming connections and sets them up in a thread.
***********************************************************************/
int threadServer(std::string port)
{
   Connection net;
   if (net.serverSetup(port.c_str()))
   {
      // Handle this to close out socket and re-establish...
      int newSocket = 0;
      unsigned int threadCounter = 0;
      // Accept all connections that come in...
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
