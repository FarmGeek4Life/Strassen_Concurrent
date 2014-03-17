/***********************************************************************
* Connection class header file
******************************************************************************/
#include <iostream>
#include <iomanip>
#include <cmath>
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

using namespace std;

/*****************************************************************************
* class Connection: provides a basic TCP connection.
*****************************************************************************/
class Connection
{
   public:
      Connection();
      ~Connection();
      int clientSetup(const char* theAddress, const char* thePort);
      int serverSetup(const char* thePort);
      int sendChar(char* toSend);
      int receiveChar(char* received);
      int sendBool(bool* toSend);
      int receiveBool(bool* received);
      int sendShort(short* toSend);
      int receiveShort(short* received);
      int sendInt(int* toSend);
      int receiveInt(int* received);
      string strError;
      
   private:
      int setupManual();
      int getSocketDesc();
      int reuseableSock();
      int bindConn();
      int getConnect();
      int doListen();
      int doAccept();
      bool isServer;
      int sockfdComm; // Communication socket (to remote system)
      int sockfdListen; // Server Listening Socket (only local)
      sockaddr_storage in;
      int* primarySockfd;
      int port;
      char* address;
      sockaddr_in stSockAddr;
};

/*****************************************************************************
* Initializes the connection object with standard defaults
*****************************************************************************/
Connection::Connection()
{
   isServer = false;
   sockfdComm = -1;
   sockfdListen = -1;
   strError = "";
   primarySockfd = NULL;
   port = 0;
   address = NULL;
}

/*****************************************************************************
* Closes connection and socket
*****************************************************************************/
Connection::~Connection()
{
   cout << "Closing socket for port " << port << endl;
   close(sockfdComm);
   if (isServer)
   {
      close(sockfdListen);
   }
   delete address;
}

/*****************************************************************************
* Sets the connection up for a client
* returns 1 if successful.
*****************************************************************************/
int Connection::clientSetup(const char* theAddress, const char* thePort)
{
   port = atoi(thePort);
   address = new char[strlen(theAddress)];
   strcpy(address, theAddress);
   primarySockfd = &sockfdComm;
   
   if (!setupManual())
      return 0;
   
   // bind not needed for a client
   if (!(getSocketDesc() && reuseableSock() && getConnect()))
      return 0;
   return 1;
}

/*****************************************************************************
* Sets the connection up for a server
* returns 1 if successful.
*****************************************************************************/
int Connection::serverSetup(const char* thePort)
{
   isServer = true;
   port = atoi(thePort);
   address = INADDR_ANY;
   primarySockfd = &sockfdListen;

   if (!setupManual())
      return 0;
   if (!(getSocketDesc() && reuseableSock() && bindConn()))// && getConnect()))
      return 0;
   
   if (!(doListen() && doAccept()))
      return 0;
   return 1;
}

/*****************************************************************************
* Sets the connection address and port
* returns 1 if successful.
*****************************************************************************/
int Connection::setupManual()
{
   cerr << "setupManual()\n";
   sockfdComm = -1;
   sockfdListen = -1;
   
   memset(&stSockAddr, '\0', sizeof(stSockAddr));
   
   stSockAddr.sin_family = AF_INET;
   stSockAddr.sin_port = htons(port);
   cerr << "Port is " << stSockAddr.sin_port << " " << port << endl;
   
   int error = 1;
   if (isServer)
   {
      stSockAddr.sin_addr.s_addr = INADDR_ANY;
   }
   else
      error = inet_pton(AF_INET, address, &(stSockAddr.sin_addr));//.s_addr));
   if (error < 1)
   {
      strError = "address";
      return 0;
   }
   
   cerr << "servaddr: " << stSockAddr.sin_family << " " << stSockAddr.sin_port << " " << stSockAddr.sin_addr.s_addr << " " << stSockAddr.sin_zero << "\n";
   return 1;
}

/*****************************************************************************
* Get the socket descriptor
* returns 1 if successful.
*****************************************************************************/
int Connection::getSocketDesc()
{
   *primarySockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   
   strError = "getSocketDesc";
   perror("Socket Status");
   cout << "Got socket descriptor with desc " << *primarySockfd << "\n";
   return (*primarySockfd >= 0);
}

/*****************************************************************************
* Configures the socket so that the port is reuseable
* returns 1 if successful.
*****************************************************************************/
int Connection::reuseableSock()
{
   int yes = 1;
   int error = 1;
   error = setsockopt(*primarySockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int));
   strError = "setSockOpt";
   perror("SetSockOpt Status");
   cout << "Reusable socket configured with code " << error << "\n";
   return (error >= 0);
}

/*****************************************************************************
* Binds the connection and socket
* returns 1 if successful.
*****************************************************************************/
int Connection::bindConn()
{
   int error = 1;
   error = bind(sockfdListen, (sockaddr *) &stSockAddr, sizeof(stSockAddr));
   strError = "bind failed: ";
   strError += strerror(error);
   cout << "Bind exiting with code " << error << "\n";
   return (error >= 0);
}

/*****************************************************************************
* Creates the connection
* returns 1 if successful.
*****************************************************************************/
int Connection::getConnect()
{
   int error = 1;
   error = connect(sockfdComm, (sockaddr *) &stSockAddr, sizeof(stSockAddr));
   strError = "connect failed";
   perror("Connect Status");
   cout << "Socket connection started with code " << error << "\n";
   return (error >= 0);
}

/*****************************************************************************
* Listens for a connection
* returns 1 if successful.
*****************************************************************************/
int Connection::doListen()
{
   int backlog = 2;
   int error = 1;
   error = listen(sockfdListen, backlog);
   strError = "listen failed: ";
   strError += strerror(error);
   perror("Listen Status");
   cout << "Listener called with code " << error << "\n";
   return (error >= 0);
}

/*****************************************************************************
* Accepts 2 connections
* returns 1 if successful.
*****************************************************************************/
int Connection::doAccept()
{
   socklen_t addr_size = sizeof(in);
   cout << "Waiting for connection\n";
   sockfdComm = accept(sockfdListen, (sockaddr *)&in, &addr_size);
   cout << "Connection established.\n";
   strError = "accept failed";
   return (sockfdComm >= 0);
}

/*****************************************************************************
* Sends a character through socket number in whereTo, default is 0
* returns > 1 if successful.
*****************************************************************************/
int Connection::sendChar(char* toSend, int size)
{
   int error = 1;
   strError = "send failed";
   error = send(sockfdComm, toSend, strlen(toSend), 0);
   perror("Send Status");
   cerr << "Sent |" << toSend << "|\n";
   return (error >= 0) ? error : 0;
}

/*****************************************************************************
* Receives a character through socket number in whereFrom, default is 0
* returns > 1 if successful.
*****************************************************************************/
int Connection::receiveChar(char* received, int size)
{
   int error = 1;
   strError = "receive failed";
   error = recv(sockfdComm, received, strlen(received), 0);
   perror("Receive Status");
   strError = error == 0 ? "socket closed" : "receive failed";
   cerr << "Received |" << received << "|\n";
   return (error > 0) ? error : 0;
}

/*****************************************************************************
* Sends a character through socket number in whereTo, default is 0
* returns > 1 if successful.
*****************************************************************************/
int Connection::sendBool(bool* toSend, int size)
{
   int error = 1;
   strError = "send failed";
   error = send(sockfdComm, toSend, sizeof(toSend)/sizeof(bool), 0);
   perror("Send Status");
   cerr << "Sent |" << toSend << "|\n";
   return (error >= 0) ? error : 0;
}

/*****************************************************************************
* Receives a character through socket number in whereFrom, default is 0
* returns > 1 if successful.
*****************************************************************************/
int Connection::receiveBool(bool* received, int size)
{
   int error = 1;
   strError = "receive failed";
   error = recv(sockfdComm, received, sizeof(received)/sizeof(bool), 0);
   perror("Receive Status");
   strError = error == 0 ? "socket closed" : "receive failed";
   cerr << "Received |" << received << "|\n";
   return (error > 0) ? error : 0;
}

/*****************************************************************************
* Sends a character through socket number in whereTo, default is 0
* returns > 1 if successful.
*****************************************************************************/
int Connection::sendShort(short* toSend, int size)
{
   int error = 1;
   strError = "send failed";
   error = send(sockfdComm, toSend, sizeof(toSend)/sizeof(short), 0);
   perror("Send Status");
   cerr << "Sent |" << toSend << "|\n";
   return (error >= 0) ? error : 0;
}

/*****************************************************************************
* Receives a character through socket number in whereFrom, default is 0
* returns > 1 if successful.
*****************************************************************************/
int Connection::receiveShort(short* received, int size)
{
   int error = 1;
   strError = "receive failed";
   error = recv(sockfdComm, received, sizeof(received)/sizeof(short), 0);
   perror("Receive Status");
   strError = error == 0 ? "socket closed" : "receive failed";
   cerr << "Received |" << received << "|\n";
   return (error > 0) ? error : 0;
}

/*****************************************************************************
* Sends a character through socket number in whereTo, default is 0
* returns > 1 if successful.
*****************************************************************************/
int Connection::sendInt(int* toSend, int size)
{
   int error = 1;
   strError = "send failed";
   error = send(sockfdComm, toSend, sizeof(toSend)/sizeof(int), 0);
   perror("Send Status");
   cerr << "Sent |" << toSend << "|\n";
   return (error >= 0) ? error : 0;
}

/*****************************************************************************
* Receives a character through socket number in whereFrom, default is 0
* returns > 1 if successful.
*****************************************************************************/
int Connection::receiveInt(int* received, int size)
{
   int error = 1;
   strError = "receive failed";
   error = recv(sockfdComm, received, sizeof(received)/sizeof(int), 0);
   perror("Receive Status");
   strError = error == 0 ? "socket closed" : "receive failed";
   cerr << "Received |" << received << "|\n";
   return (error > 0) ? error : 0;
}
