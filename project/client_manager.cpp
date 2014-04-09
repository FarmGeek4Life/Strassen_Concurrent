/***************************************************************************
* Program:
*    Copied from strassen_int_opt
* Author:
*    Bryson Gibbons
* Summary:
*    Senior Project
*    
***************************************************************************/

//#include <cmath>
#include <iostream>
#include <fstream>
//#include <iomanip>
//#include <cstdlib>
#include <thread>
// Trying mutual exclusion....
//#include <mutex>
//#include <chrono>
//#include <cstring>
#include <string>
#include <unistd.h>
//#include "connection.h"
#include "errorColors.h"
#include "matrix.h"
// Signal handling....
#include <signal.h>
#include <errno.h>
// For timing....
#include <ctime>

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

/****************************************************************************
* Parse an environment variable (':' separated)
****************************************************************************/
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
      cerr << Red << "Unable to open " + file << RCol << endl;
      //return 1;
      error = true;
   }
}

/****************************************************************************
* Parse the command line and environment variables
* Read in the input matrices
* Start the distribution
****************************************************************************/
int main(int argc, char* argv[])
{
   // Signal Handler Setup
   signal(SIGPIPE, signal_callback_handler);
   int size = 32;
   ifstream inFile;
   ifstream inFile2;
   //ofstream fout;
   string file;
   string file2;
   string fileOut;
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
      cerr << Red << "Please set environment variable 'BG_COMPUTERS'!" << RCol << "\n";
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
      cerr << Red << "Please set environment variable 'BG_PORT'" << RCol << "\n";
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
   
   int thread_Stop = 0;
   
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
      fileOut = argv[3];
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
   
   
   
   time_t start;
   time_t readin;
   time_t calculate;
   time_t end;
   
   time(&start);
   
   //cerr << "\nStart Time: " << asctime(localtime(&start));
   //cerr << "\nEnd Time: " << asctime(localtime(&end)) << endl;
   double timed;
   int min;
   double sec;
   int times[3][2];
   
   
   
   
   
   Matrix<int> matrixA(size);
   Matrix<int> matrixB(size);
   
   //Tie this local variable to the matrix class static variable...
   bool* NetError = &(matrixA.NetError);
   
   //matrixA.thread_Stop = thread_Stop / 2;
   matrixA.thread_Stop = thread_Stop;
   //matrixA.maxThreads = 17 * 2;
   matrixA.maxThreads = 17 * 4;

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
   
   time(&readin);
   timed = difftime (readin, start);
   times[0][0] = static_cast<int>(timed) / 60;
   times[0][1] = timed - ( static_cast<double>(times[0][0]) * 60 );
   //min = static_cast<int>(timed) / 60;
   //sec = timed - ( static_cast<double>(min) * 60 );
   //cerr << "Time (read in): " << min << " minutes " << sec << " seconds.\n";
   
   if (totalComputers == 1)
   {
      /**/
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
      //   matrixA.thread_Stop = 0;
      //   matrixA.thread_Start = matrixA.thread_Stop * 4;
      matrixA.mult_FarmSlave(matrixB);
      /*/
      matrixA.runParallel(matrixB, computers[0], port, 0);
      /**/
   }
   else
   {
      matrixA.mult_ThreadFarming(matrixB, computers, totalComputers, port);
   }
   
   
   
   time(&calculate);
   timed = difftime (calculate, start);
   times[1][0] = static_cast<int>(timed) / 60;
   times[1][1] = timed - ( static_cast<double>(times[1][0]) * 60 );
   //min = static_cast<int>(timed) / 60;
   //sec = timed - ( static_cast<double>(min) * 60 );
   //cerr << "Time (compute): " << min << " minutes " << sec << " seconds.\n";
   
   
   
   
   if (*NetError && size < 128)
   {
      cerr << matrixA;
   }
   cerr << "Outputting result...\n";
   cout << matrixA;
   
   
   time(&end);
   timed = difftime (end, start);
   times[2][0] = static_cast<int>(timed) / 60;
   times[2][1] = timed - ( static_cast<double>(times[2][0]) * 60 );
   //min = static_cast<int>(timed) / 60;
   //sec = timed - ( static_cast<double>(min) * 60 );
   //cerr << "Time (output ): " << min << " minutes " << sec << " seconds.\n";
   
   cerr << "Time (read in): " << times[0][0] << " minutes " << times[0][1] << " seconds.\n";
   cerr << "Time (compute): " << times[1][0] << " minutes " << times[1][1] << " seconds.\n";
   cerr << "Time (output ): " << times[2][0] << " minutes " << times[2][1] << " seconds.\n";
   
   
   
   return 0;
}
