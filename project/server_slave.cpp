/***************************************************************************
* Program:
*    server_leaf
* Author:
*    Bryson Gibbons
* Summary:
*    Senior Project - Matrix multiplication, distributed computing
*    This is the slave/server portion of the code
*    Everything meaningful is in matrix.h and connection.h
***************************************************************************/

#include <iostream>
#include "errorColors.h"
#include "matrix.h"
// Signal handling....
#include <signal.h>

using namespace std;

// Signal handlers...
void signal_callback_handler(int signum)
{
   // http://www.yolinux.com/TUTORIALS/C++Signals.html
   cerr << Red << "Caught signal '" << signum << "': SIGPIPE (13)" << RCol << "\n";
   // Cleanup and close up stuff here...
   
   // We want to catch and ignore, so we will just report.
   // Terminate the program....
   //exit(signum)
}
// End signal handlers.... (also a portion in main)

/****************************************************************************
* Parse the command line, and call the right function
****************************************************************************/
int main(int argc, char* argv[])
{
   // Signal Handler Setup
   signal(SIGPIPE, signal_callback_handler);
   int size = 32;
   string port;

   if (argc < 2)
   {
      port = "10021";
   }
   else
   {
      port = argv[1];
   }
   
   threadServer(port);

   return 0;
}
