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
      int sendChar(char* toSend, int numBytes);
      int receiveChar(char* received, int numBytes);
      int sendBool(bool* toSend, int numBytes);
      int receiveBool(bool* received, int numBytes);
      int sendShort(short* toSend, int numBytes);
      int receiveShort(short* received, int numBytes);
      int sendInt(int* toSend, int numBytes);
      int receiveInt(int* received, int numBytes);
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
      char* portStr;
      char* address;
      sockaddr_in stSockAddr;
      addrinfo hints;
      addrinfo *stSockAddr1;
      addrinfo *stSockAddr2;
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
   portStr = NULL;
   address = NULL;
   memset(&stSockAddr, '\0', sizeof stSockAddr);
   memset(&hints, 0, sizeof hints);
   stSockAddr1 = NULL;
}

/*****************************************************************************
* Closes connection and socket
*****************************************************************************/
Connection::~Connection()
{
   //cerr << "Closing socket for port " << port << endl;
   close(sockfdComm);
   if (isServer)
   {
      close(sockfdListen);
   }
   if (address != NULL)
      delete address;
   delete portStr;
   freeaddrinfo(stSockAddr1);
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
   portStr = new char[strlen(thePort)];
   strcpy(portStr, thePort);
   primarySockfd = &sockfdComm;
   
   int error = 1;
   error = inet_pton(AF_INET, address, &(stSockAddr.sin_addr));//.s_addr));
   if (error < 1)
   {
      strError = "address";
      return 0;
   }
   
   if (!setupManual())
      return 0;
   
   // bind not needed for a client
   //if (!(getSocketDesc() && reuseableSock() && getConnect()))
   //   return 0;
   for (stSockAddr2 = stSockAddr1; stSockAddr2 != NULL; stSockAddr2 = stSockAddr2->ai_next)
   {
      if (!getSocketDesc())
         continue; // Socket creation failed....
      if (!reuseableSock())
         ; // I don't know what I should do...
      if (getConnect())
         break; //Success!!! Go on to do other things.
      close(*primarySockfd); // try again
   }
   //if (*primarySockfd != NULL)
   //   cerr << "CONNECTION SUCCESS!!!!!\n";
   //else
   //   cerr << "CONNECTION FAILURE!!!!!\n";
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
   portStr = new char[strlen(thePort)];
   strcpy(portStr, thePort);
   address = NULL;
   primarySockfd = &sockfdListen;
   
   stSockAddr.sin_addr.s_addr = INADDR_ANY;
   hints.ai_flags = AI_PASSIVE; // Fill in my IP for me - only for server
   
   if (!setupManual())
      return 0;
   //if (!(getSocketDesc() && reuseableSock() && bindConn()))// && getConnect()))
   //   return 0;
   for (stSockAddr2 = stSockAddr1; stSockAddr2 != NULL; stSockAddr2 = stSockAddr2->ai_next)
   {
      if (!getSocketDesc())
         continue; // Socket creation failed....
      if (!reuseableSock())
         ; // I don't know what I should do...
      if (bindConn())
         break; //Success!!! Go on to do other things.
      close(*primarySockfd); // try again
   }
   //if (*primarySockfd != NULL)
   //   cerr << "BIND SUCCESS!!!!!\n";
   //else
   //   cerr << "BIND FAILURE!!!!!\n";
   
   if (!(doListen() && doAccept()))
      return 0;
   //if (*primarySockfd != NULL)
   //   cerr << "CONNECT SUCCESS!!!!!\n";
   //else
   //   cerr << "CONNECT FAILURE!!!!!\n";
   return 1;
}

/*****************************************************************************
* Sets the connection address and port
* returns 1 if successful.
*****************************************************************************/
int Connection::setupManual()
{
   //cerr << "setupManual()\n";
   sockfdComm = -1;
   sockfdListen = -1;
   
   hints.ai_family = AF_UNSPEC; // AF_INET for IPv4, AF_INET6 for IPv6, AF_UNSPEC for "I don't care which"
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_protocol = IPPROTO_TCP;
   hints.ai_canonname = NULL; // Initialize pointer to NULL
   hints.ai_addr = NULL; // Initialize pointer to NULL
   hints.ai_next = NULL; // Initialize pointer to NULL
   
   
   stSockAddr.sin_family = AF_INET;
   stSockAddr.sin_port = htons(port);
   //cerr << "Port is " << stSockAddr.sin_port << " " << port << endl;
   
   getaddrinfo(address, portStr, &hints, &stSockAddr1);
   
   /*int error = 1;
   if (isServer)
   {
      stSockAddr.sin_addr.s_addr = INADDR_ANY;
      hints.ai_flags = AI_PASSIVE; // Fill in my IP for me - only for server
   }
   else
      error = inet_pton(AF_INET, address, &(stSockAddr.sin_addr));//.s_addr));
   if (error < 1)
   {
      strError = "address";
      return 0;
   }*/
   
   //cerr << "servaddr: " << stSockAddr.sin_family << " " << stSockAddr.sin_port << " " << stSockAddr.sin_addr.s_addr << " " << stSockAddr.sin_zero << "\n";
   return 1;
}

/*****************************************************************************
* Get the socket descriptor
* returns 1 if successful.
*****************************************************************************/
int Connection::getSocketDesc()
{
   //*primarySockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
   *primarySockfd = socket(stSockAddr2->ai_family, stSockAddr2->ai_socktype, stSockAddr2->ai_protocol);
   
   strError = "getSocketDesc";
   //perror("Socket Status");
   //cerr << "Got socket descriptor with desc " << *primarySockfd << "\n";
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
   //perror("SetSockOpt Status");
   //cerr << "Reusable socket configured with code " << error << "\n";
   return (error >= 0);
}

/*****************************************************************************
* Binds the connection and socket
* returns 1 if successful.
*****************************************************************************/
int Connection::bindConn()
{
   int error = 1;
   //error = bind(sockfdListen, (sockaddr *) &stSockAddr, sizeof(stSockAddr));
   error = ::bind(sockfdListen, stSockAddr2->ai_addr, stSockAddr2->ai_addrlen);
   strError = "bind failed: ";
   strError += strerror(error);
   //cerr << "Bind exiting with code " << error << "\n";
   return (error >= 0);
   return true;
}

/*****************************************************************************
* Creates the connection
* returns 1 if successful.
*****************************************************************************/
int Connection::getConnect()
{
   int error = 1;
   //error = connect(sockfdComm, (sockaddr *) &stSockAddr, sizeof(stSockAddr));
   error = connect(sockfdComm, stSockAddr2->ai_addr, stSockAddr2->ai_addrlen);
   strError = "connect failed";
   //perror("Connect Status");
   //cerr << "Socket connection started with code " << error << "\n";
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
   //perror("Listen Status");
   //cerr << "Listener called with code " << error << "\n";
   return (error >= 0);
}

/*****************************************************************************
* Accepts 2 connections
* returns 1 if successful.
*****************************************************************************/
int Connection::doAccept()
{
   socklen_t addr_size = sizeof(in);
   //cerr << "Waiting for connection\n";
   sockfdComm = accept(sockfdListen, (sockaddr *)&in, &addr_size);
   //cerr << "Connection established.\n";
   strError = "accept failed";
   return (sockfdComm >= 0);
}

/*****************************************************************************
* Sends a character through socket number in whereTo, default is 0
* returns > 1 if successful.
*****************************************************************************/
int Connection::sendChar(char* toSend)
{
   int error = 1;
   strError = "send failed";
   error = send(sockfdComm, toSend, strlen(toSend), 0);
   //perror("Send Status");
   //cerr << "Sent |" << toSend << "|\n";
   return (error >= 0) ? error : 0;
}

/*****************************************************************************
* Receives a character through socket number in whereFrom, default is 0
* returns > 1 if successful.
*****************************************************************************/
int Connection::receiveChar(char* received)
{
   int error = 1;
   strError = "receive failed";
   error = recv(sockfdComm, received, strlen(received), 0);
   //perror("Receive Status");
   strError = error == 0 ? "socket closed" : "receive failed";
   //cerr << "Received |" << received << "|\n";
   return (error > 0) ? error : 0;
}

/*****************************************************************************
* Sends a character through socket number in whereTo, default is 0
* returns > 1 if successful.
*****************************************************************************/
int Connection::sendChar(char* toSend, int numBytes)
{
   int error = 1;
   strError = "send failed";
   //error = send(sockfdComm, toSend, strlen(toSend), 0);
   error = send(sockfdComm, toSend, numBytes, 0);
   //perror("Send Status");
   //cerr << "Sent |" << toSend << "|\n";
   return (error >= 0) ? error : 0;
}

/*****************************************************************************
* Receives a character through socket number in whereFrom, default is 0
* returns > 1 if successful.
*****************************************************************************/
int Connection::receiveChar(char* received, int numBytes)
{
   int error = 1;
   strError = "receive failed";
   //error = recv(sockfdComm, received, strlen(received), 0);
   error = recv(sockfdComm, received, numBytes, 0);
   //perror("Receive Status");
   strError = error == 0 ? "socket closed" : "receive failed";
   //cerr << "Received |" << received << "|\n";
   return (error > 0) ? error : 0;
}

/*****************************************************************************
* Sends a character through socket number in whereTo, default is 0
* returns > 1 if successful.
*****************************************************************************/
int Connection::sendBool(bool* toSend, int numBytes)
{
   int error = 1;
   strError = "send failed";
   //error = send(sockfdComm, toSend, sizeof(toSend), 0);
   error = send(sockfdComm, toSend, numBytes, 0);
   //perror("Send Status");
   //cerr << "Sent |" << toSend << "|\n";
   return (error >= 0) ? error : 0;
}

/*****************************************************************************
* Receives a character through socket number in whereFrom, default is 0
* returns > 1 if successful.
*****************************************************************************/
int Connection::receiveBool(bool* received, int numBytes)
{
   int error = 1;
   strError = "receive failed";
   //error = recv(sockfdComm, received, sizeof(received), 0);
   error = recv(sockfdComm, received, numBytes, 0);
   //perror("Receive Status");
   strError = error == 0 ? "socket closed" : "receive failed";
   //cerr << "Received |" << received << "|\n";
   return (error > 0) ? error : 0;
}

/*****************************************************************************
* Sends a character through socket number in whereTo, default is 0
* returns > 1 if successful.
*****************************************************************************/
int Connection::sendShort(short* toSend, int numBytes)
{
   int error = 1;
   strError = "send failed";
   //error = send(sockfdComm, toSend, sizeof(toSend), 0);
   error = send(sockfdComm, toSend, numBytes, 0);
   //perror("Send Status");
   //cerr << "Sent |" << toSend << "|\n";
   return (error >= 0) ? error : 0;
}

/*****************************************************************************
* Receives a character through socket number in whereFrom, default is 0
* returns > 1 if successful.
*****************************************************************************/
int Connection::receiveShort(short* received, int numBytes)
{
   int error = 1;
   strError = "receive failed";
   //error = recv(sockfdComm, received, sizeof(received), 0);
   error = recv(sockfdComm, received, numBytes, 0);
   //perror("Receive Status");
   strError = error == 0 ? "socket closed" : "receive failed";
   //cerr << "Received |" << received << "|\n";
   return (error > 0) ? error : 0;
}

/*****************************************************************************
* Sends a character through socket number in whereTo, default is 0
* returns > 1 if successful.
*****************************************************************************/
int Connection::sendInt(int* toSend, int numBytes)
{
   int error = 1;
   strError = "send failed";
   //error = send(sockfdComm, toSend, sizeof(toSend), 0);
   error = send(sockfdComm, toSend, numBytes, 0);
   //perror("Send Status");
   //cerr << "Sent |" << toSend << "| in size " << sizeof(toSend) << ", but sent only " << error << " bytes\n";
   return (error >= 0) ? error : 0;
}

/*****************************************************************************
* Receives a character through socket number in whereFrom, default is 0
* returns > 1 if successful.
*****************************************************************************/
int Connection::receiveInt(int* received, int numBytes)
{
   int error = 1;
   strError = "receive failed";
   //error = recv(sockfdComm, received, sizeof(received), 0);
   error = recv(sockfdComm, received, numBytes, 0);
   //perror("Receive Status");
   strError = error == 0 ? "socket closed" : "receive failed";
   //cerr << "Received |" << received << "| in size " << sizeof(received) << ", but sent only " << error << " bytes\n";
   return (error > 0) ? error : 0;
}