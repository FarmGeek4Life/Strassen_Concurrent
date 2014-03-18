/***************************************************************************
* Program:
*    Copied from strassen_int_opt
* Author:
*    Bryson Gibbons
* Summary:
*    Senior Project
*    
***************************************************************************/

#include <cmath>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <cstring>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "connection.h"
// Signal handling....
#include <signal.h>
#include <errno.h>

using namespace std;

// Signal handlers...
void signal_callback_handler(int signum)
{
   // http://www.yolinux.com/TUTORIALS/C++Signals.html
   cerr << "Caught signal '" << signum << "': SIGPIPE (13)\n";
   // Cleanup and close up stuff here...
   
   // We want to catch and ignore, so we will just report.
   // Terminate the program....
   //exit(signum)
}
// End signal handlers.... (also a portion in main)

// Stopping point for thread creation
int thread_Stop;

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
    * Matrix multiplication using Strassen's algorithm.
    * The input matrices must be equal in size and square, 
    *    and the size must be n x n, where n is a power of 2
    *************************************************************************/
   //Matrix operator*(const Matrix matrixB) const
   Matrix<T> mult(const Matrix<T>& matrixB, Matrix<T>& result, string computers[], int numComputers, string port) const
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
         
         thread t[8];
         Connection* net = new Connection[numComputers];
         //for (int i = 0; i < numComputers; ++i)
         //{
         //   cerr << "Opening net for '" << computers[i] << "'\n";
         //   if (!net[i].clientSetup(computers[i].c_str(), port.c_str()))
         //   {
         //      // Report error....
         //      cerr << "ERROR: Network connection failed for system '" << computers[i] << "'!!!!: " << net[i].strError << "\n\n";
         //   }
         //}
         
         // Four quadrants for each matrix being multiplied
         Matrix<T> a00(getQuadrant(0, 0));
         Matrix<T> a01(getQuadrant(0, 1));
         Matrix<T> a10(getQuadrant(1, 0));
         Matrix<T> a11(getQuadrant(1, 1));
         //{
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
         //if (mSize > thread_Stop)
         //{
            int i = 0;
            t[1] = thread(&Matrix<T>::runParallel, (a00 + a11), (b00 + b11), std::ref(m1), computers[i++], port, net[0]);
            t[2] = thread(&Matrix<T>::runParallel, (a10 + a11), (b00)      , std::ref(m2), computers[i++], port, net[1]);
            t[3] = thread(&Matrix<T>::runParallel,  a00       , (b01 - b11), std::ref(m3), computers[i++], port, net[2]);
            t[4] = thread(&Matrix<T>::runParallel,  a11       , (b10 - b00), std::ref(m4), computers[i++], port, net[3]);
            t[5] = thread(&Matrix<T>::runParallel, (a00 + a01), (b11)      , std::ref(m5), computers[i++], port, net[4]);
            t[6] = thread(&Matrix<T>::runParallel, (a10 - a00), (b00 + b01), std::ref(m6), computers[i++], port, net[5]);
            t[7] = thread(&Matrix<T>::runParallel, (a01 - a11), (b10 + b11), std::ref(m7), computers[i++], port, net[6]);
            
            //std::this_thread::sleep_for(chrono::seconds(1));
            //sleep(2);
            
         //}
            
            t[1].join();
            t[2].join();
            t[3].join();
            t[4].join();
            t[5].join();
            t[6].join();
            t[7].join();
         //}
         //else
         //{
         //   (a00 + a11).mult (b00 + b11, m1);
         //   (a10 + a11).mult (b00      , m2);
         //    a00       .mult (b01 - b11, m3);
         //    a11       .mult (b10 - b00, m4);
         //   (a00 + a01).mult (b11      , m5);
         //   (a10 - a00).mult (b00 + b01, m6);
         //   (a01 - a11).mult (b10 + b11, m7);
         //}
         
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
    * Matrix multiplication using Strassen's algorithm.
    * The input matrices must be equal in size and square, 
    *    and the size must be n x n, where n is a power of 2
    *************************************************************************/
   //Matrix operator*(const Matrix matrixB) const
   Matrix<T> runParallel(const Matrix<T> matrixB, Matrix<T>& result, string computer, string port, Connection& net) const
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
         cerr << "STARTING THREAD FOR SYSTEM '" << computer << "'!!!\n";
         //Connection net;
         if (net.clientSetup(computer.c_str(), port.c_str()))
         {
            bool failed = false;
            int size[1] = {mSize};
            int close[5] = {0, 0, 0, 0, 0};
            // All data must be sent as pointers or arrays!!!
            // SHOULD PROBABLY CHANGE TO THROWING AND CATCHING ERRORS!!!!!
            // Send size
            if (!net.sendInt(size, 4))
            {
               cerr << "Server closed connection: " << computer << "\n";
               cerr << "ERROR (" << computer << "): " << net.strError << "\n";
               failed = true;
            }
            // Send matrix A
            //cerr << "SIZE CHECK!!!!!: " << sizeof(this->mRows[0]) << " should be " << mSize * 4 << endl;
            if (failed || !this->writeNet(net))
            {
               cerr << "Server closed connection: " << computer << "\n";
               cerr << "ERROR (" << computer << "): " << net.strError << "\n";
               failed = true;
            }
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
               cerr << "Server closed connection: " << computer << "\n";
               cerr << "ERROR (" << computer << "): " << net.strError << "\n";
               failed = true;
            }
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
               cerr << "Server closed connection: " << computer << "\n";
               cerr << "ERROR (" << computer << "): " << net.strError << "\n";
               failed = true;
            }
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
            if (failed || !net.sendInt(close, 5 * 4))
            {
               cerr << "Server closed connection: " << computer << "\n";
               cerr << "ERROR (" << computer << "): " << net.strError << "\n";
            }
         }
         else
         {
            // Report error....
            cerr << "ERROR: Network connection failed!!!!: " << net.strError << "\n\n";
         }
      }
      else
      {
         // Assume a matrix of size 1
         result[0][0] = mRows[0][0] * matrixB[0][0];
      }
      cerr << "EXITING THREAD FOR SYSTEM '" << computer << "'!!!\n";
      return result;
   }
   
   /************************************************************************
   * Function to read data to fill a matrix from a network socket
   ***********************************************************************/
   bool readNet(Connection& net)
   {
      bool success = true;
      for (int i = 0; i < mSize; ++i)
      {
         if (!net.receiveInt(this->mRows[i], mSize * sizeof(T)))
         {
            //cerr << "Server closed connection\n";
            //cerr << "ERROR: " << net.strError << endl;
            success = false;
            break;
         }
      }
      return success;
   }
   
   /************************************************************************
   * Function to write data from entire matrix to a network socket
   ***********************************************************************/
   bool writeNet(Connection& net) const
   {
      bool success = true;
      for (int i = 0; i < mSize; ++i)
      {
         if (!net.sendInt(this->mRows[i], mSize * (sizeof(T))))
         {
            //cerr << "Server closed connection\n";
            //cerr << "ERROR: " << net.strError << endl;
            success = false;
            break;
         }
      }
      return success;
   }
};

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

void parseEnv(char* envVar, string output[], int& count)
{
   string var(envVar);
   int pos = 0;
   int next = var.find_first_of(":", pos);
   count = 0;
   while (next != string::npos)
   {
      output[count++] = var.substr(pos, next - pos);
      pos = next + 1;
      next = var.find_first_of(":", pos);
   }
   // Catch the last one....
   output[count++] = var.substr(pos, next - pos);
}

int main(int argc, char* argv[])
{
   // Signal Handler Setup
   signal(SIGPIPE, signal_callback_handler);
   int size = 32;
   ifstream inFile;
   ifstream inFile2;
   string file;
   string file2;
   char* compList;
   string port;
   // Avoid unknown errors when the environment variables aren't set...
   try
   {
      if (getenv("BG_COMPUTERS") == NULL)
      {
         throw 1;
      }
      else
      {
         compList = getenv("BG_COMPUTERS");
      }
   }
   catch (...)
   {
      cerr << "Please set environment variable 'BG_COMPUTERS'!\n";
      return 1;
   }
   try
   {
      if (getenv("BG_PORT") == NULL)
      {
         throw 2;
      }
      else
      {
         port = getenv("BG_PORT");
      }
   }
   catch (...)
   {
      cerr << "Please set environment variable 'BG_PORT'\n";
      port = "10021";
      cerr << "Using port number '10021' for this run.\n";
   }
   string computers[35];
   int totalComputers = 0;
   if (compList != NULL)
   {
      parseEnv(compList, computers, totalComputers);
   }
   
   bool error = false;
   //for (int i = 0; i < totalComputers; ++i)
   //{
   //   cerr << "Computer" << setw(3) << ": " << computers[i] << endl;
   //}
   
   if (port.find_first_not_of("0123456789") != string::npos)
   {
      cout << "Error! Port invalid: " << port << endl;
      error = true;
   }
   else if (atoi(port.c_str()) < 1024)
   {
      cout << "Error! Reserved ports (< 1024) cannot be used!\n";
      error = true;
   }
   else if (atoi(port.c_str()) > 65535)
   {
      cout << "Error! Maximum port number is 65535!\n";
      error = true;
   }
   if (error)
   {
      cout << "Error! Server port invalid: " << port << endl;
      cout << "Usage: " << argv[0] << " [file1] [file2] [size]\n";
      return 1;
   }
   
   thread_Stop = 0;
   
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
   else if (argc == 4)
   {
      file = argv[1];
      file2 = argv[2];
      size = atoi(argv[3]);
   }
   else if (argc == 5)
   {
      file = argv[1];
      file2 = argv[2];
      size = atoi(argv[3]);
      thread_Stop = atoi(argv[4]);
   }
   else if (argc > 5)
   {
      file = argv[1];
      file2 = argv[2];
      size = atoi(argv[3]);
      thread_Stop = atoi(argv[4]);
      /// Add something here to handle entering computers by command line...
   }
   else 
   {
      cout << "Usage: " << argv[0] << " [file1] [file2] [size]\n";
   }
   if (thread_Stop == 0)
   {
      // How to split it up....
      thread_Stop = size / 2;
   }

   Matrix<int> matrixA(size);
   Matrix<int> matrixB(size);

   inFile.open(file.c_str());
   
   if (inFile.is_open())
   {
      inFile >> matrixA;
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
      inFile2 >> matrixB;
      inFile2.close();
   }
   else 
   {
      cerr << "Unable to open " + file2;
      return 1;
   }
   //cout << (matrixA * matrixB);
   Matrix<int> result(size);
   if (totalComputers == 1)
   {
      //matrixA.runParallel(matrixB, result, computers[0], port);
      
      Connection net;
      if (net.clientSetup(computers[0].c_str(), port.c_str()))
      {
         thread t = thread(&Matrix<int>::runParallel, matrixA, matrixB, std::ref(result), computers[0], port, net);
         t.join();
      }
      else
      {
         // Report error....
         cerr << "ERROR: Network connection failed for system '" << computers[0] << "'!!!!: " << net.strError << "\n\n";
      }
   }
   else
   {
      matrixA.mult(matrixB, result, computers, totalComputers, port);
   }
   //cerr << result;
   cout << result;

   return 0;
}
