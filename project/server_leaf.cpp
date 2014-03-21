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
#include "matrix.h"
// Signal handling....
#include <signal.h>
#include <errno.h>
// Nanosleep... http://linux.die.net/man/2/nanosleep
#include <time.h>

using namespace std;

// Error setting for broken pipe...
bool pipe_Broke = false;

// Signal handlers...
void signal_callback_handler(int signum)
{
   // http://www.yolinux.com/TUTORIALS/C++Signals.html
   cerr << Red << "Caught signal '" << signum << "': SIGPIPE (13)" << RCol << "\n";
   // Cleanup and close up stuff here...
   
   // We want to catch and ignore, so we will just report.
   // This has not been properly implemented in any way.
   pipe_Broke = true;
   // Terminate the program....
   //exit(signum)
}
// End signal handlers.... (also a portion in main)


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
// Declare the mutex....
std::mutex oneByOne;
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


/************************************************************************
* A function to allow threading from the connections
***********************************************************************/
//void threadedManager(Connection& net)
void threadedManager(int socket, unsigned int id)
{
   // Limit execution of this block of code....
   // Automatically releases when leaving scope
   //std::lock_guard<std::mutex> lock(oneByOne);
   cerr << Gre << "STARTING EXECUTION: THREAD " << id << RCol << "\n";
   Connection net(socket);
   int threadId = 0;
   int tSize[1] = {0};
   int close[5] = {0, 0, 0, 0, 0};
   // Receive threadId
   cerr << Gre << "RECEIVING THREAD ID..." << RCol << "\n";
   if (!net.receiveInt(&threadId, 4) || pipe_Broke)
   {
      cerr << Red << "Server closed connection\n";
      cerr << "ERROR: " << net.strError << RCol << endl;
   }
   // Receive size
   cerr << Gre << "RECEIVING SIZE..." << RCol << "\n";
   if (!net.receiveInt(tSize, 4) || pipe_Broke)
   {
      cerr << Red << "Server closed connection\n";
      cerr << "ERROR: " << net.strError << RCol << endl;
   }
   int size = tSize[0];
   cerr << Gre << "SIZE RECEIVED IS: " << size << RCol << endl;
   Matrix<int> matrixA(size);
   Matrix<int> matrixB(size);
   Matrix<int> result(size);
   matrixA.thread_Stop = size / 4;
   
   // Receive matrix A
   //matrixA.readNet(net);
   //cout << matrixA;
   /*for (int i = 0; i < mSize; ++i)
   {
      if (!net.receiveInt(this->mRows[i]))
      {
         cerr << "Server closed connection\n";
         break;
      }
   }*/
   // Receive matrix B
   //matrixB.readNet(net);
   //cout << matrixB;
   /*for (int i = 0; i < mSize; ++i)
   {
      if (!net.receiveInt(matrixB.mRows[i]))
      {
         cerr << "Server closed connection\n";
         break;
      }
   }*/
   
   // Receive matrix A and Receive matrix B
   if (matrixA.readNet(net) && matrixB.readNet(net))
   {
      cerr << Gre << "STATUS: Multiplying matrices!!!" << RCol << "\n";
      matrixA.mult_FarmSlave(matrixB, result);
      // Send Result
      result.writeNet(net);
   }
   else
   {
      cerr << Red << "ERROR: Failed to receive matrices!!!!" << RCol << "\n";
      net.closeComm();
      return;
   }
   //cout << result;
   /*for (int i = 0; i < mSize; ++i)
   {
      if (!net.sendInt(result.mRows[i]))
      {
         cerr << "Server closed connection\n";
         break;
      }
   }*/
   // Receive close or continue - close and exit for now.
   //net.receiveInt(close, 5 * 4);
   //cout << Gre << "CLOSE COMMAND RECEIVED: " << close[0] << RCol << endl;
   //cout << result;
   net.closeComm();
   cerr << Gre << "FINISHING EXECUTION: THREAD " << id << ", manager thread " << threadId << RCol << "\n";
}

int main(int argc, char* argv[])
{
   // Signal Handler Setup
   signal(SIGPIPE, signal_callback_handler);
   int size = 32;
   string port;

   if (argc < 2)
   {
      port = "10001";
   }
   else
   {
      port = argv[1];
   }
   
   //file = argv[1];
   //file2 = argv[2];
   //size = atoi(argv[3]);
   //thread_Stop = atoi(argv[4]);
   
   Connection net;
   //////////////////////////////////////////////////////////////////////////
   //////////////////////////////////////////////////////////////////////////
   ////// Thought: Make the management of the matrix multiplication threaded-
   ////// Listen for connections, and make a new thread for each one.
   ////// Will require some modification of connection.h
   //////////////////////////////////////////////////////////////////////////
   //////////////////////////////////////////////////////////////////////////
   if (net.serverSetup(port.c_str()))
   {
      cerr << Gre << "Net setup!!!" << RCol << "\n";
      // Handle this to close out socket and re-establish...
      pipe_Broke = false;
      //Connection child;
      timespec toSleep;
      toSleep.tv_sec = 0;
      toSleep.tv_nsec = 200000000;
      int newSocket = 0;
      unsigned int threadCounter = 0;
      //while (net.serverConnection(child) != 0)
      while (net.serverConnection(newSocket) != 0)
      {
         cerr << Gre << "Got a connection!" << RCol << "\n";
         //threadedManager(child);
         // To run threaded....
         //thread newCon = thread(threadedManager, std::ref(child));
         //thread newCon = thread(threadedManager, child);
         thread newCon = thread(threadedManager, newSocket, threadCounter++);
         newCon.detach(); // Detach the thread, so that we don't worry about it's cleanup
         //thread newCon = thread(threadedManager, std::ref(net));
         //newCon.join(); //For testing and debugging
         
         // Both sleep and usleep (microseconds, millionths) come from unistd.h
         //sleep(1); // Sleep 1 seconds
         /*/
         usleep(200000); // Sleep 200 milliseconds
         /*/
         usleep(50000); // Sleep 50 milliseconds
         /**/
         // Nanosleep requires time.h; uses struct timespec with tv_sec & tv_nsec
         //nanosleep(&toSleep, NULL);
         
         //int tSize[1];
         //int close[5];
         //// Receive size
         //if (!net.receiveInt(tSize, 4) || pipe_Broke)
         //{
         //   cerr << "Server closed connection\n";
         //}
         //size = tSize[0];
         //cerr << "SIZE RECEIVED IS: " << size << endl;
         //thread_Stop = size / 4;
         //Matrix<int> matrixA(size);
         //Matrix<int> matrixB(size);
         //Matrix<int> result(size);
         //
         //// Receive matrix A
         //matrixA.readNet(net);
         ////cout << matrixA;
         ///*for (int i = 0; i < mSize; ++i)
         //{
         //   if (!net.receiveInt(this->mRows[i]))
         //   {
         //      cerr << "Server closed connection\n";
         //      break;
         //   }
         //}*/
         //// Receive matrix B
         //matrixB.readNet(net);
         ////cout << matrixB;
         ///*for (int i = 0; i < mSize; ++i)
         //{
         //   if (!net.receiveInt(matrixB.mRows[i]))
         //   {
         //      cerr << "Server closed connection\n";
         //      break;
         //   }
         //}*/
         //matrixA.mult_FarmSlave(matrixB, result);
         //// Send Result
         //result.writeNet(net);
         ////cout << result;
         ///*for (int i = 0; i < mSize; ++i)
         //{
         //   if (!net.sendInt(result.mRows[i]))
         //   {
         //      cerr << "Server closed connection\n";
         //      break;
         //   }
         //}*/
         //// Receive close or continue - close and exit for now.
         //net.receiveInt(close, 5 * 4);
         //cout << "CLOSE COMMAND RECEIVED: " << close[0] << endl;
         ////cout << result;
      }
      net.closeServer();
   }
   else
   {
      cout << Red << "\nError: " << net.strError << RCol << "\n\n";
      return 1;
   }

   return 0;
}
