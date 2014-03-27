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

public:
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
   }
   
   Matrix<T>(const Matrix<T>& matrixA, const Matrix<T>& matrixB, bool add)
   {
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
   // Stopping point for thread creation
   static int thread_Stop;
   static int sysCounter;
   static std::mutex sysCounter_Mutex;
   // Limit the number of simultaneous threads
   static std::mutex threadLimiter[100];
   static int maxThreads;
   
   /**************************************************************************
    * Matrix multiplication using Strassen's algorithm.
    * The input matrices must be equal in size and square, 
    *    and the size must be n x n, where n is a power of 2
    *************************************************************************/
   void mult_ThreadFarming(Matrix<T>& matrixB, Matrix<T>* result, std::string computers[], int numComputers, std::string port) const
   {
      if (mSize > 1)
      {
         std::thread t[8];
         
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
         //Matrix<T> m1(mSize / 2);
         //Matrix<T> m2(mSize / 2);
         //Matrix<T> m3(mSize / 2);
         //Matrix<T> m4(mSize / 2);
         //Matrix<T> m5(mSize / 2);
         //Matrix<T> m6(mSize / 2);
         //Matrix<T> m7(mSize / 2);
         // Initialize to the left side of the multiplication...
         Matrix<T> m1(a00, a11, true ); // Create new object, adding 2nd to 1st
         Matrix<T> m2(a10, a11, true ); // Create new object, adding 2nd to 1st
         Matrix<T> m3(a00            ); // Make a copy...
         Matrix<T> m4(a11            ); // Make a copy...
         Matrix<T> m5(a00, a01, true ); // Create new object, adding 2nd to 1st
         Matrix<T> m6(a10, a00, false); // Create new object, subtracting 2nd from 1st
         Matrix<T> m7(a01, a11, false); // Create new object, subtracting 2nd from 1st
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
         // It makes no sense to split into smaller chunks for 7 computers
         if (mSize > thread_Stop && numComputers != 7 && numComputers != 1)
         {
            //t[1] = std::thread(&Matrix<T>::mult_ThreadFarming, (a00 + a11), (b00 + b11), &m1, computers, numComputers, port);
            //t[2] = std::thread(&Matrix<T>::mult_ThreadFarming, (a10 + a11), (b00)      , &m2, computers, numComputers, port);
            //t[3] = std::thread(&Matrix<T>::mult_ThreadFarming,  a00       , (b01 - b11), &m3, computers, numComputers, port);
            //t[4] = std::thread(&Matrix<T>::mult_ThreadFarming,  a11       , (b10 - b00), &m4, computers, numComputers, port);
            //t[5] = std::thread(&Matrix<T>::mult_ThreadFarming, (a00 + a01), (b11)      , &m5, computers, numComputers, port);
            //t[6] = std::thread(&Matrix<T>::mult_ThreadFarming, (a10 - a00), (b00 + b01), &m6, computers, numComputers, port);
            //t[7] = std::thread(&Matrix<T>::mult_ThreadFarming, (a01 - a11), (b10 + b11), &m7, computers, numComputers, port);
            t[1] = std::thread(&Matrix<T>::mult_ThreadFarming, &m1, (b00 + b11), null, computers, numComputers, port);
            t[2] = std::thread(&Matrix<T>::mult_ThreadFarming, &m2, (b00)      , null, computers, numComputers, port);
            t[3] = std::thread(&Matrix<T>::mult_ThreadFarming, &m3, (b01 - b11), null, computers, numComputers, port);
            t[4] = std::thread(&Matrix<T>::mult_ThreadFarming, &m4, (b10 - b00), null, computers, numComputers, port);
            t[5] = std::thread(&Matrix<T>::mult_ThreadFarming, &m5, (b11)      , null, computers, numComputers, port);
            t[6] = std::thread(&Matrix<T>::mult_ThreadFarming, &m6, (b00 + b01), null, computers, numComputers, port);
            t[7] = std::thread(&Matrix<T>::mult_ThreadFarming, &m7, (b10 + b11), null, computers, numComputers, port);
         }
         else
         {
            // Mutex: control the access to the system counter (sysCounter) for even distribution
            std::lock_guard<std::mutex> lock(sysCounter_Mutex);
            //t[1] = std::thread(&Matrix<T>::runParallel, (a00 + a11), (b00 + b11), std::ref(m1), computers[sysCounter % numComputers], port, sysCounter); ++sysCounter;
            //t[2] = std::thread(&Matrix<T>::runParallel, (a10 + a11), (b00)      , std::ref(m2), computers[sysCounter % numComputers], port, sysCounter); ++sysCounter;
            //t[3] = std::thread(&Matrix<T>::runParallel,  a00       , (b01 - b11), std::ref(m3), computers[sysCounter % numComputers], port, sysCounter); ++sysCounter;
            //t[4] = std::thread(&Matrix<T>::runParallel,  a11       , (b10 - b00), std::ref(m4), computers[sysCounter % numComputers], port, sysCounter); ++sysCounter;
            //t[5] = std::thread(&Matrix<T>::runParallel, (a00 + a01), (b11)      , std::ref(m5), computers[sysCounter % numComputers], port, sysCounter); ++sysCounter;
            //t[6] = std::thread(&Matrix<T>::runParallel, (a10 - a00), (b00 + b01), std::ref(m6), computers[sysCounter % numComputers], port, sysCounter); ++sysCounter;
            //t[7] = std::thread(&Matrix<T>::runParallel, (a01 - a11), (b10 + b11), std::ref(m7), computers[sysCounter % numComputers], port, sysCounter); ++sysCounter;
            t[1] = std::thread(&Matrix<T>::runParallel, &m1, (b00 + b11), null, computers[sysCounter % numComputers], port, sysCounter); ++sysCounter;
            t[2] = std::thread(&Matrix<T>::runParallel, &m2, (b00)      , null, computers[sysCounter % numComputers], port, sysCounter); ++sysCounter;
            t[3] = std::thread(&Matrix<T>::runParallel, &m3, (b01 - b11), null, computers[sysCounter % numComputers], port, sysCounter); ++sysCounter;
            t[4] = std::thread(&Matrix<T>::runParallel, &m4, (b10 - b00), null, computers[sysCounter % numComputers], port, sysCounter); ++sysCounter;
            t[5] = std::thread(&Matrix<T>::runParallel, &m5, (b11)      , null, computers[sysCounter % numComputers], port, sysCounter); ++sysCounter;
            t[6] = std::thread(&Matrix<T>::runParallel, &m6, (b00 + b01), null, computers[sysCounter % numComputers], port, sysCounter); ++sysCounter;
            t[7] = std::thread(&Matrix<T>::runParallel, &m7, (b10 + b11), null, computers[sysCounter % numComputers], port, sysCounter); ++sysCounter;
         }
         // Clear out allocated memory....
         b00.erase();
         b01.erase();
         b10.erase();
         b11.erase();
         // Also include matrixB
         matrixB.erase();
         
         t[1].join();
         t[2].join();
         t[3].join();
         t[4].join();
         t[5].join();
         t[6].join();
         t[7].join();
         
         // Use the 7 multiplication results to get the results for each quadrant
         // Save on memory usage by reusing one set of quadrants
         // Also saves on time - no de/reallocation
         //a00 = m1 + m4 - m5 + m7;
         //a01 = m3 + m5;
         //a10 = m2 + m4;
         //a11 = m1 + m3 - m2 + m6;
         a00.op00_11(m1, m4, m5, m7);
         a01.op01_10(m3, m5);
         a10.op01_10(m2, m4);
         a11.op00_11(m1, m3, m2, m6);
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
         if (result != NULL)
         {
            *result = Matrix(a00, a01, a10, a11);
         }
      }
      else
      {
         // Assume a matrix of size 1
         if (result != NULL)
         {
            *result[0][0] = mRows[0][0] * matrixB[0][0];
         }
         mRows[0][0] = mRows[0][0] * matrixB[0][0];
      }
   }
   
   /**************************************************************************
    * Matrix multiplication using Strassen's algorithm.
    * The input matrices must be equal in size and square, 
    *    and the size must be n x n, where n is a power of 2
    *************************************************************************/
   //void runParallel(Matrix<T> matrixB, Matrix<T>& result, std::string computer, std::string port, int id)
   void runParallel(Matrix<T> matrixB, Matrix<T>* result, std::string computer, std::string port, int id)
   {
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
            // Erase the allocated memory of this object. It's not needed anymore.
            this->erase();
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
            if (failed || (result == NULL ? !this->readNet(net) : !result->readNet(net)))
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
         if (result != NULL)
         {
            *result[0][0] = mRows[0][0] * matrixB[0][0];
         }
         mRows[0][0] = mRows[0][0] * matrixB[0][0];
      }
      std::cerr << Gre << "EXITING THREAD " << id << " FOR SYSTEM '" << computer << "'!!!" << RCol  << "\n";
   }
   
   /**************************************************************************
    * Matrix multiplication using Strassen's algorithm.
    * The input matrices must be equal in size and square, 
    *    and the size must be n x n, where n is a power of 2
    *************************************************************************/
   void mult_FarmSlave(Matrix<T>& matrixB, Matrix<T>* result)
   {
      if (mSize > 1)
      {
         std::thread t[8];
         
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
         //Matrix<T> m1(mSize / 2);
         //Matrix<T> m2(mSize / 2);
         //Matrix<T> m3(mSize / 2);
         //Matrix<T> m4(mSize / 2);
         //Matrix<T> m5(mSize / 2);
         //Matrix<T> m6(mSize / 2);
         //Matrix<T> m7(mSize / 2);
         // Initialize to the left side of the multiplication...
         Matrix<T> m1(a00, a11, true ); // Create new object, adding 2nd to 1st
         Matrix<T> m2(a10, a11, true ); // Create new object, adding 2nd to 1st
         Matrix<T> m3(a00            ); // Make a copy...
         Matrix<T> m4(a11            ); // Make a copy...
         Matrix<T> m5(a00, a01, true ); // Create new object, adding 2nd to 1st
         Matrix<T> m6(a10, a00, false); // Create new object, subtracting 2nd from 1st
         Matrix<T> m7(a01, a11, false); // Create new object, subtracting 2nd from 1st
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
         if (mSize > thread_Stop)
         {
            //t[1] = std::thread(&Matrix<T>::mult_FarmSlave, (a00 + a11), (b00 + b11), &m1);
            //t[2] = std::thread(&Matrix<T>::mult_FarmSlave, (a10 + a11), (b00)      , &m2);
            //t[3] = std::thread(&Matrix<T>::mult_FarmSlave, (a00)      , (b01 - b11), &m3);
            //t[4] = std::thread(&Matrix<T>::mult_FarmSlave, (a11)      , (b10 - b00), &m4);
            //t[5] = std::thread(&Matrix<T>::mult_FarmSlave, (a00 + a01), (b11)      , &m5);
            //t[6] = std::thread(&Matrix<T>::mult_FarmSlave, (a10 - a00), (b00 + b01), &m6);
            //t[7] = std::thread(&Matrix<T>::mult_FarmSlave, (a01 - a11), (b10 + b11), &m7);
            t[1] = std::thread(&Matrix<T>::mult_FarmSlave, &m1, (b00 + b11), null);
            t[2] = std::thread(&Matrix<T>::mult_FarmSlave, &m2, (b00)      , null);
            t[3] = std::thread(&Matrix<T>::mult_FarmSlave, &m3, (b01 - b11), null);
            t[4] = std::thread(&Matrix<T>::mult_FarmSlave, &m4, (b10 - b00), null);
            t[5] = std::thread(&Matrix<T>::mult_FarmSlave, &m5, (b11)      , null);
            t[6] = std::thread(&Matrix<T>::mult_FarmSlave, &m6, (b00 + b01), null);
            t[7] = std::thread(&Matrix<T>::mult_FarmSlave, &m7, (b10 + b11), null);
         }
         else if (mSize > 512)
         {
            /*/
            (a00 + a11).mult_wrapper (b00 + b11, &m1);
            (a10 + a11).mult_wrapper (b00      , &m2);
            (a00)      .mult_wrapper (b01 - b11, &m3);
            (a11)      .mult_wrapper (b10 - b00, &m4);
            (a00 + a01).mult_wrapper (b11      , &m5);
            (a10 - a00).mult_wrapper (b00 + b01, &m6);
            (a01 - a11).mult_wrapper (b10 + b11, &m7);
            /*/
            m1.mult_wrapper (b00 + b11, null);
            m2.mult_wrapper (b00      , null);
            m3.mult_wrapper (b01 - b11, null);
            m4.mult_wrapper (b10 - b00, null);
            m5.mult_wrapper (b11      , null);
            m6.mult_wrapper (b00 + b01, null);
            m7.mult_wrapper (b10 + b11, null);
            /**/
         }
         else
         {
            (a00 + a11).multStandard (b00 + b11, m1);
            (a10 + a11).multStandard (b00      , m2);
            (a00)      .multStandard (b01 - b11, m3);
            (a11)      .multStandard (b10 - b00, m4);
            (a00 + a01).multStandard (b11      , m5);
            (a10 - a00).multStandard (b00 + b01, m6);
            (a01 - a11).multStandard (b10 + b11, m7);
         }
         // Clear out allocated memory....
         b00.erase();
         b01.erase();
         b10.erase();
         b11.erase();
         // We don't need matrixB data anymore. Erase it.
         matrixB.erase();
         
         if (mSize > thread_Stop)
         {
            t[1].join();
            t[2].join();
            t[3].join();
            t[4].join();
            t[5].join();
            t[6].join();
            t[7].join();
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
         if (result != NULL)
         {
            *result = Matrix(a00, a01, a10, a11);
         }
      }
      else
      {
         // Assume a matrix of size 1
         if (result != NULL)
         {
            *result[0][0] = mRows[0][0] * matrixB[0][0];
         }
         mRows[0][0] = mRows[0][0] * matrixB[0][0];
      }
   }
   
   /**************************************************************************
    * Matrix multiplication using Strassen's algorithm.
    * The input matrices must be equal in size and square, 
    *    and the size must be n x n, where n is a power of 2
    *************************************************************************/
   void mult_wrapper(Matrix<T> matrixB, Matrix<T>* result)
   {
      this->mult_FarmSlave(matrixB, result);
      //this->mult(matrixB);
   }
   
   /**************************************************************************
    * Standard matrix multiplication
    * The input matrices must be equal in size and square, 
    *    and the size must be n x n, where n is a power of 2
    *************************************************************************/
   //Matrix operator*(const Matrix matrixB) const
   Matrix<T> multStandard(const Matrix<T>& matrixB, Matrix<T>& result) const
   {
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
      
      return result;
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
               std::cerr << Red << "Server closed connection\n";
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
               std::cerr << Red << "Server closed connection\n";
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
bool Matrix<T>::NetError = false;
template <class T>
int Matrix<T>::thread_Stop = 0;
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
      std::cerr << Red << "Server closed connection\n";
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
   matrixA.thread_Stop = size / 4;
   
   // Receive matrix A and Receive matrix B
   if (matrixA.readNet(net) && matrixB.readNet(net))
   {
      std::cerr << Gre << "STATUS: Multiplying matrices!!!" << RCol << "\n";
      matrixA.mult_FarmSlave(matrixB, NULL);
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
