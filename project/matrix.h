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

using namespace std;

template <class T>
class Matrix
{
private:
   T** mRows;
   int mSize;

public:
   Matrix<T>(int size)
   {
      mRows = new T*[size];
      for (int i = 0; i < size; i++)
      {
         mRows[i] = new T[size];
      }   
      mSize = size;
   }

   Matrix<T>(const Matrix<T>& matrixB)
   {
      mSize = matrixB.getSize();

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

   Matrix<T>& operator=(const Matrix<T>& matrixB)
   {
      if (mSize != matrixB.getSize())
      {
         for (int i = 0; i < mSize; i++)
         {
            delete [] mRows[i];
         }
         delete [] mRows;
         
         mSize = matrixB.getSize();
         
         mRows = new T*[mSize];
         for (int i = 0; i < mSize; i++)
         {
            mRows[i] = new T[mSize];
            for (int j = 0; j < mSize; ++j)
               mRows[i][j] = matrixB.mRows[i][j];
         }
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

   ~Matrix<T>()
   {
      for (int i = 0; i < mSize; i++)
      {
         delete [] mRows[i];
      }   
      delete [] mRows;
   }

   T* operator[](int row) const
   {
      return mRows[row];
   }

   int getSize() const
   {
      return mSize;
   }

   void read(istream& is) const
   {
      for (int i = 0; i < mSize; i++)
      {
         for (int j = 0; j < mSize; j++)
         {
            is >> mRows[i][j];
         }      
      }
   }

   void write(ostream& os) const
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
   
   /**************************************************************************
    * Constructor to build a matrix from quadrants
    *************************************************************************/
   Matrix<T>(const Matrix<T>& copy00, const Matrix<T>& copy01, const Matrix<T>& copy10, const Matrix<T>& copy11)
   {
      // Assume that all parameters matrices are equal in size
      mSize = copy00.getSize() * 2;
      
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
   }
   
   /**************************************************************************
   ***************************************************************************
   ** Can possibly increase speed here - make a constructor.                **
   ***************************************************************************
   **************************************************************************/
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
   
   // Output the entire matrix on error:
   static bool NetError;
   // Stopping point for thread creation
   static int thread_Stop;
   static int sysCounter;
   static std::mutex sysCounter_Mutex;
   
   /**************************************************************************
    * Matrix multiplication using Strassen's algorithm.
    * The input matrices must be equal in size and square, 
    *    and the size must be n x n, where n is a power of 2
    *************************************************************************/
   Matrix<T> mult_ThreadFarming(const Matrix<T>& matrixB, Matrix<T>& result, string computers[], int numComputers, string port) const
   {
      if (mSize > 1)
      {
         // Temporary Matrices to hold the 7 multiplication results
         Matrix<T> m1(mSize / 2);
         Matrix<T> m2(mSize / 2);
         Matrix<T> m3(mSize / 2);
         Matrix<T> m4(mSize / 2);
         Matrix<T> m5(mSize / 2);
         Matrix<T> m6(mSize / 2);
         Matrix<T> m7(mSize / 2);
         
         thread t[8];
         
         // Four quadrants for each matrix being multiplied
         Matrix<T> a00(getQuadrant(0, 0));
         Matrix<T> a01(getQuadrant(0, 1));
         Matrix<T> a10(getQuadrant(1, 0));
         Matrix<T> a11(getQuadrant(1, 1));
         {
         Matrix<T> b00(matrixB.getQuadrant(0, 0));
         Matrix<T> b01(matrixB.getQuadrant(0, 1));
         Matrix<T> b10(matrixB.getQuadrant(1, 0));
         Matrix<T> b11(matrixB.getQuadrant(1, 1));
         
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
            t[1] = thread(&Matrix<T>::mult_ThreadFarming, (a00 + a11), (b00 + b11), std::ref(m1), computers, numComputers, port);
            t[2] = thread(&Matrix<T>::mult_ThreadFarming, (a10 + a11), (b00)      , std::ref(m2), computers, numComputers, port);
            t[3] = thread(&Matrix<T>::mult_ThreadFarming,  a00       , (b01 - b11), std::ref(m3), computers, numComputers, port);
            t[4] = thread(&Matrix<T>::mult_ThreadFarming,  a11       , (b10 - b00), std::ref(m4), computers, numComputers, port);
            t[5] = thread(&Matrix<T>::mult_ThreadFarming, (a00 + a01), (b11)      , std::ref(m5), computers, numComputers, port);
            t[6] = thread(&Matrix<T>::mult_ThreadFarming, (a10 - a00), (b00 + b01), std::ref(m6), computers, numComputers, port);
            t[7] = thread(&Matrix<T>::mult_ThreadFarming, (a01 - a11), (b10 + b11), std::ref(m7), computers, numComputers, port);
         }
         else
         {
            // Mutex: control the access to the system counter (sysCounter) for even distribution
            std::lock_guard<std::mutex> lock(sysCounter_Mutex);
            t[1] = thread(&Matrix<T>::runParallel, (a00 + a11), (b00 + b11), std::ref(m1), computers[sysCounter % numComputers], port, sysCounter++);
            t[2] = thread(&Matrix<T>::runParallel, (a10 + a11), (b00)      , std::ref(m2), computers[sysCounter % numComputers], port, sysCounter++);
            t[3] = thread(&Matrix<T>::runParallel,  a00       , (b01 - b11), std::ref(m3), computers[sysCounter % numComputers], port, sysCounter++);
            t[4] = thread(&Matrix<T>::runParallel,  a11       , (b10 - b00), std::ref(m4), computers[sysCounter % numComputers], port, sysCounter++);
            t[5] = thread(&Matrix<T>::runParallel, (a00 + a01), (b11)      , std::ref(m5), computers[sysCounter % numComputers], port, sysCounter++);
            t[6] = thread(&Matrix<T>::runParallel, (a10 - a00), (b00 + b01), std::ref(m6), computers[sysCounter % numComputers], port, sysCounter++);
            t[7] = thread(&Matrix<T>::runParallel, (a01 - a11), (b10 + b11), std::ref(m7), computers[sysCounter % numComputers], port, sysCounter++);
         }   
         // End special scope for memory savings
         }
            
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
         a00 = m1 + m4 - m5 + m7;
         a01 = m3 + m5;
         a10 = m2 + m4;
         a11 = m1 + m3 - m2 + m6;
         
         // Reassemble the quadrants into a single whole
         result = Matrix(a00, a01, a10, a11);
      }
      else
      {
         // Assume a matrix of size 1
         result[0][0] = mRows[0][0] * matrixB[0][0];
      }
      return result;
   }
   
   /**************************************************************************
    * Matrix multiplication using Strassen's algorithm.
    * The input matrices must be equal in size and square, 
    *    and the size must be n x n, where n is a power of 2
    *************************************************************************/
   //Matrix operator*(const Matrix matrixB) const
   //Matrix<T> runParallel(const Matrix<T> matrixB, Matrix<T>& result, string computer, string port, Connection& net) const
   //Matrix<T> runParallel(const Matrix<T> matrixB, Matrix<T>& result, string computer, string port) const
   Matrix<T> runParallel(const Matrix<T> matrixB, Matrix<T>& result, string computer, string port, int id) const
   {
      //////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////
      ////// Thought: use a mutex to block other threads from using the
      ////// network connection while this one is still in process...
      //////////////////////////////////////////////////////////////////////////
      //////////////////////////////////////////////////////////////////////////
      //cerr << "Computer Check: '" << computer << "'\n";
      //Matrix result(mSize);
      if (mSize > 1)
      {
         cerr << Gre << "STARTING THREAD " << id << " FOR SYSTEM '" << computer << "'!!!" << RCol << "\n";
         Connection net;
         if (net.clientSetup(computer.c_str(), port.c_str()))
         {
            cerr << UPur << "THREAD " << id << " FOR SYSTEM '" << computer << "': Got Connection!!!" << RCol << "\n";
            bool failed = false;
            int size[1] = {mSize};
            int close[5] = {0, 0, 0, 0, 0};
            // All data must be sent as pointers or arrays!!!
            // SHOULD PROBABLY CHANGE TO THROWING AND CATCHING ERRORS!!!!!
            // Send thread id (for debugging purposes)
            cerr << Gre << "SENDING THEAD ID " << id << ": '" << computer << "'" << RCol << "\n";
            if (!net.sendData(&id, 4))
            {
               cerr << Red << "Server closed connection: " << computer << "\n";
               cerr << "ERROR (" << computer << "): " << net.strError << RCol << "\n";
               failed = true;
            }
            // Send size
            cerr << Gre << "SENDING SIZE: '" << computer << "'" << RCol << "\n";
            if (!net.sendData(size, 4))
            {
               cerr << Red << "Server closed connection: " << computer << "\n";
               cerr << "ERROR (" << computer << "): " << net.strError << RCol << "\n";
               failed = true;
            }
            cerr << Gre << "SENT SIZE: '" << computer << "'" << RCol << "\n";
            // Send matrix A
            //cerr << "SIZE CHECK!!!!!: " << sizeof(this->mRows[0]) << " should be " << mSize * 4 << endl;
            if (failed || !this->writeNet(net))
            {
               cerr << Red << "Server closed connection: " << computer << "\n";
               cerr << "ERROR (" << computer << "): " << net.strError << RCol << "\n";
               failed = true;
            }
            cerr << Gre << "SENT Matrix A: '" << computer << "'" << RCol << "\n";
            /*for (int i = 0; i < mSize; ++i)
            {
               if (!net.sendInt(this->mRows[i], mSize * (sizeof(T))))
               {
                  cerr << "Server closed connection\n";
                  break;
               }
            }*/
            // Send matrix B
            if (failed || !matrixB.writeNet(net))
            {
               cerr << Red << "Server closed connection: " << computer << "\n";
               cerr << "ERROR (" << computer << "): " << net.strError << RCol << "\n";
               failed = true;
            }
            cerr << Gre << "SENT Matrix B: '" << computer << "'" << RCol << "\n";
            /*for (int i = 0; i < mSize; ++i)
            {
               if (!net.sendInt(matrixB.mRows[i], mSize * (sizeof(T))))
               {
                  cerr << "Server closed connection\n";
                  break;
               }
            }*/
            // Receive Result
            if (failed || !result.readNet(net))
            {
               cerr << Red << "Server closed connection: " << computer << "\n";
               cerr << "ERROR (" << computer << "): " << net.strError << RCol << "\n";
               failed = true;
            }
            cerr << Gre << "Got Result: '" << computer << "'" << RCol << "\n";
            /*for (int i = 0; i < mSize; ++i)
            {
               if (!net.receiveInt(result.mRows[i], mSize * (sizeof(T))))
               {
                  cerr << "Server closed connection\n";
                  break;
               }
            }*/
            // Send close or continue - close and exit for now.
            //cerr << result;
            /*if (failed || !net.sendInt(close, 5 * 4))
            {
               cerr << Red << "Server closed connection: " << computer << "\n";
               cerr << "ERROR (" << computer << "): " << net.strError << RCol << "\n";
            }*/
            net.closeComm();
            if (failed)
            {
               NetError = true;
            }
         }
         else
         {
            // Report error....
            cerr << Red << "ERROR: Network connection failed!!!!: " << net.strError << RCol << "\n\n";
            NetError = true;
         }
      }
      else
      {
         // Assume a matrix of size 1
         result[0][0] = mRows[0][0] * matrixB[0][0];
      }
      cerr << Gre << "EXITING THREAD " << id << " FOR SYSTEM '" << computer << "'!!!" << RCol  << "\n";
      return result;
   }
   
   /**************************************************************************
    * Matrix multiplication using Strassen's algorithm.
    * The input matrices must be equal in size and square, 
    *    and the size must be n x n, where n is a power of 2
    *************************************************************************/
   //Matrix operator*(const Matrix matrixB) const
   Matrix<T> mult_FarmSlave(const Matrix<T>& matrixB, Matrix<T>& result) const
   {
      //Matrix result(mSize);
      if (mSize > 1)
      {
         // Temporary Matrices to hold the 7 multiplication results
         Matrix<T> m1(mSize / 2);
         Matrix<T> m2(mSize / 2);
         Matrix<T> m3(mSize / 2);
         Matrix<T> m4(mSize / 2);
         Matrix<T> m5(mSize / 2);
         Matrix<T> m6(mSize / 2);
         Matrix<T> m7(mSize / 2);
         
         thread t1;
         thread t2;
         thread t3;
         thread t4;
         thread t5;
         thread t6;
         thread t7;
         
         // Four quadrants for each matrix being multiplied
         Matrix<T> a00(getQuadrant(0, 0));
         Matrix<T> a01(getQuadrant(0, 1));
         Matrix<T> a10(getQuadrant(1, 0));
         Matrix<T> a11(getQuadrant(1, 1));
         // Scope change for memory purposes...
         {
         Matrix<T> b00(matrixB.getQuadrant(0, 0));
         Matrix<T> b01(matrixB.getQuadrant(0, 1));
         Matrix<T> b10(matrixB.getQuadrant(1, 0));
         Matrix<T> b11(matrixB.getQuadrant(1, 1));
         
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
            //thread t1 = thread(&Matrix<T>::mult, (a00 + a11), (b00 + b11), std::ref(m1));
            //thread t2 = thread(&Matrix<T>::mult, (a10 + a11), (b00)      , std::ref(m2));
            //thread t3 = thread(&Matrix<T>::mult,  a00       , (b01 - b11), std::ref(m3));
            //thread t4 = thread(&Matrix<T>::mult,  a11       , (b10 - b00), std::ref(m4));
            //thread t5 = thread(&Matrix<T>::mult, (a00 + a01), (b11)      , std::ref(m5));
            //thread t6 = thread(&Matrix<T>::mult, (a10 - a00), (b00 + b01), std::ref(m6));
            //thread t7 = thread(&Matrix<T>::mult, (a01 - a11), (b10 + b11), std::ref(m7));
            t1 = thread(&Matrix<T>::mult_FarmSlave, (a00 + a11), (b00 + b11), std::ref(m1));
            t2 = thread(&Matrix<T>::mult_FarmSlave, (a10 + a11), (b00)      , std::ref(m2));
            t3 = thread(&Matrix<T>::mult_FarmSlave, (a00)      , (b01 - b11), std::ref(m3));
            t4 = thread(&Matrix<T>::mult_FarmSlave, (a11)      , (b10 - b00), std::ref(m4));
            t5 = thread(&Matrix<T>::mult_FarmSlave, (a00 + a01), (b11)      , std::ref(m5));
            t6 = thread(&Matrix<T>::mult_FarmSlave, (a10 - a00), (b00 + b01), std::ref(m6));
            t7 = thread(&Matrix<T>::mult_FarmSlave, (a01 - a11), (b10 + b11), std::ref(m7));
   
            //t1.join();
            //t2.join();
            //t3.join();
            //t4.join();
            //t5.join();
            //t6.join();
            //t7.join();
         }
         else
         {
            //(a00 + a11).mult (b00 + b11, m1);
            //(a10 + a11).mult (b00      , m2);
            // a00       .mult (b01 - b11, m3);
            // a11       .mult (b10 - b00, m4);
            //(a00 + a01).mult (b11      , m5);
            //(a10 - a00).mult (b00 + b01, m6);
            //(a01 - a11).mult (b10 + b11, m7);
            
            (a00 + a11).multStandard (b00 + b11, m1);
            (a10 + a11).multStandard (b00      , m2);
            (a00)      .multStandard (b01 - b11, m3);
            (a11)      .multStandard (b10 - b00, m4);
            (a00 + a01).multStandard (b11      , m5);
            (a10 - a00).multStandard (b00 + b01, m6);
            (a01 - a11).multStandard (b10 + b11, m7);
         }
         //End scope change for memory purposes...
         }
         if (mSize > thread_Stop)
         {
            t1.join();
            t2.join();
            t3.join();
            t4.join();
            t5.join();
            t6.join();
            t7.join();
         }
         
         // Use the 7 multiplication results to get the results for each quadrant
         // Save on memory usage by reusing one set of quadrants
         a00 = m1 + m4 - m5 + m7;
         a01 = m3 + m5;
         a10 = m2 + m4;
         a11 = m1 + m3 - m2 + m6;
         
         // Reassemble the quadrants into a single whole
         result = Matrix(a00, a01, a10, a11);
      }
      else
      {
         // Assume a matrix of size 1
         result[0][0] = mRows[0][0] * matrixB[0][0];
      }
      return result;
   }
   
   /**************************************************************************
    * Matrix multiplication management
    * The input matrices must be equal in size and square, 
    *    and the size must be n x n, where n is a power of 2
    *************************************************************************/
   //Matrix operator*(const Matrix matrixB) const
   Matrix<T> multMan(const Matrix<T> matrixB, Matrix<T>& result) const
   {
      // Variables to store information about the threads
      int threadTotal = 0;
      thread* threadId;
      int threadCount = 0;
      int threadTracker = 0;
      int threadCounter = 0;
      /* We have to create M * N worker threads */
      if (mSize > 32)
      {
         threadTotal = mSize;
         threadId = new thread[threadTotal];
         for (int i = 0; i < mSize; ++i)
         {
            threadId[threadCount++] = thread(&Matrix<T>::multiplyRow, *this, i, std::ref(matrixB), std::ref(result)); 
            ++threadTracker;
            if (threadTracker > 100)
            {
               while (threadTracker > 50)
               {
                  threadId[threadCounter++].join();
                  --threadTracker;
               }
            }
         }
      }
      else
      {
         threadTotal = mSize * mSize;
         threadId = new thread[threadTotal];
         for (int i = 0; i < mSize; ++i)
         {
            for (int j = 0; j < mSize; ++j )
            {
               threadId[threadCount++] = thread(&Matrix<T>::multiply, *this, i, j, std::ref(matrixB), std::ref(result));
            }
         }
      }
      
      // Get and store the result from the threads
      for (int i = threadCounter; i < threadTotal; ++i)
      {
         // Get the data from the threads
         threadId[i].join();
      }
      delete [] threadId;
      return result;
   }
   
   /**************************************************************************
    * Matrix multiplication management
    * The input matrices must be equal in size and square, 
    *    and the size must be n x n, where n is a power of 2
    *************************************************************************/
   //Matrix operator*(const Matrix matrixB) const
   Matrix<T> multStandard(const Matrix<T> matrixB, Matrix<T>& result) const
   {
      for (int i = 0; i < mSize; ++i)
      {
         for (int j = 0; j < mSize; ++j )
         {
            this->multiply(i, j, matrixB, result);
         }
      }
      
      return result;
   }
   
   /************************************************************************
   * Low level matrix multiplier.....
   ***********************************************************************/
   void multiply(int i, int j, const Matrix<T>& matrixB, Matrix<T>& result) const
   {
      // Initialize to known value...
      result[i][j] = 0;
      for (int k = 0; k < mSize; ++k)
      {
         result[i][j] += (*this)[i][k] * matrixB[k][j];
      }
   }
   
   /************************************************************************
   * Low level matrix multiplier.....
   ***********************************************************************/
   void multiplyRow(int i, const Matrix<T>& matrixB, Matrix<T>& result) const
   {
      for (int j = 0; j < mSize; ++j)
      {
         // Initialize to known value...
         result[i][j] = 0;
         for (int k = 0; k < mSize; ++k)
         {
            result[i][j] += (*this)[i][k] * matrixB[k][j];
         }
      }
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
            cerr << Red << "Server closed connection\n";
            cerr << "ERROR: " << net.strError << RCol << endl;
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
               cerr << Red << "Server closed connection\n";
               cerr << "ERROR: " << net.strError << RCol << endl;
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
               cerr << Red << "Server closed connection\n";
               cerr << "ERROR: " << net.strError << RCol << endl;
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
            cerr << Red << "Server closed connection\n";
            cerr << "ERROR: " << net.strError << RCol << endl;
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
               cerr << Red << "Server closed connection\n";
               cerr << "ERROR: " << net.strError << RCol << endl;
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
               cerr << Red << "Server closed connection\n";
               cerr << "ERROR: " << net.strError << RCol << endl;
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

#endif //_MATRIX_H
