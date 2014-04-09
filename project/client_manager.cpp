/***************************************************************************
* Program:
*    Client Manager, Senior Project
* Author:
*    Bryson Gibbons
* Summary:
*    Senior Project
*    
***************************************************************************/

#include <iostream>
#include <thread>
#include <string>
#include <unistd.h>
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
   // We want to catch and ignore, so we will just report.
   cerr << "Caught signal '" << signum << "': SIGPIPE (13)\n";
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
* Parse the command line and environment variables
* Read in the input matrices
* Start the distribution
****************************************************************************/
int main(int argc, char* argv[])
{
   // Signal Handler Setup - We don't want SIGPIPE killing the program
   signal(SIGPIPE, signal_callback_handler);
   
   bool local = false;
   int size = 32;
   ifstream inFile;
   ifstream inFile2;
   string file;
   string file2;
   string fileOut;
   char* compList;
   string port;
   // Avoid unknown errors when the environment variables aren't set...
   try
   {
      if (getenv("BG_COMPUTERS") == NULL)
      // Run locally.
      local = true;
         //throw 1;
      else
         compList = getenv("BG_COMPUTERS");
   }
   catch (...)
   {
      cerr << Red << "Please set environment variable 'BG_COMPUTERS' to use server slaves!" << RCol << "\n";
      // Run locally.
      local = true;
   }
   try
   {
      if (getenv("BG_PORT") == NULL)
         throw 2;
      else
         port = getenv("BG_PORT");
   }
   catch (...)
   {
      if (!local)
      {
         cerr << Red << "Please set environment variable 'BG_PORT'" << RCol << "\n";
         port = "10021";
         cerr << "Using port number '10021' for this run.\n";
      }
   }
   string computers[35];
   int totalComputers = 0;
   // Read the list of computers into something easier to use
   if (compList != NULL)
   {
      parseEnv(compList, computers, totalComputers);
   }
   
   if (!local)
   {
      // Perform some checks on the port number...
      bool error = false;
      
      if (port.find_first_not_of("0123456789") != string::npos)
      {
         cerr << "Error! Port invalid: " << port << endl;
         error = true;
      }
      else if (atoi(port.c_str()) < 1024)
      {
         cerr << "Error! Reserved ports (< 1024) cannot be used!\n";
         error = true;
      }
      else if (atoi(port.c_str()) > 65535)
      {
         cerr << "Error! Maximum port number is 65535!\n";
         error = true;
      }
      if (error)
      {
         cerr << "Error! Server port invalid: " << port << endl;
         cerr << "Usage: " << argv[0] << " [file1] [file2] [size]\n";
         return 1;
      }
   }
   
   // Parse the command line
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
   }
   
   // Variable to 
   time_t start;
   time_t readin;
   time_t calculate;
   time_t end;
   
   // Capture the start time
   time(&start);
   
   // Declare the matrices
   Matrix<int> matrixA(size);
   Matrix<int> matrixB(size);
   
   //Tie this local variable to the matrix class static variable...
   bool* NetError = &(matrixA.NetError);
   
   matrixA.thread_Stop = size / 2;
   matrixA.maxThreads = 17 * 4;

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
   
   // Capture the read completion time
   time(&readin);
   
   if (local)
   {
      // Execute locally
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
      matrixA.mult_FarmSlave(matrixB);
   }
   else if (totalComputers == 1)
   {
      // Execute using a single slave server
      matrixA.runParallel(matrixB, computers[0], port, 0);
   }
   else
   {
      // Execute on a group of slave servers
      matrixA.mult_ThreadFarming(matrixB, computers, totalComputers, port);
   }
   
   // Capture the calculation completion time
   time(&calculate);
   
   if (*NetError && size < 128)
   {
      cerr << matrixA;
   }
   cerr << "Outputting result...\n";
   cout << matrixA;
   
   // Capture the output completion time
   time(&end);
   
   // Make some sense out the the capture times.
   double timed;
   int min;
   double sec;
   int times[3][2];
   timed = difftime (readin, start);
   times[0][0] = static_cast<int>(timed) / 60;
   times[0][1] = timed - ( static_cast<double>(times[0][0]) * 60 );
   timed = difftime (calculate, start);
   times[1][0] = static_cast<int>(timed) / 60;
   times[1][1] = timed - ( static_cast<double>(times[1][0]) * 60 );
   timed = difftime (end, start);
   times[2][0] = static_cast<int>(timed) / 60;
   times[2][1] = timed - ( static_cast<double>(times[2][0]) * 60 );
   
   cerr << "Time (read in): " << times[0][0] << " minutes " << times[0][1] << " seconds.\n";
   cerr << "Time (compute): " << times[1][0] << " minutes " << times[1][1] << " seconds.\n";
   cerr << "Time (output ): " << times[2][0] << " minutes " << times[2][1] << " seconds.\n";
   
   return 0;
}
