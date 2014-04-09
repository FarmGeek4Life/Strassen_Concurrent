/***********************************************************************
* Connection class header file
******************************************************************************/

#ifndef _CONNECTION_H
#define _CONNECTION_H

//#include <iostream> // Includes <string> in the c++11/0x library version
#include <cstring> // Only needed by sendChar/receiveChar (strlen)
#include <string> //?????? /// I'm using it, but it is getting included in another header?? (c++11/0x only) (it is included in the c++11/0x <iostream> header
//#include <sys/types.h> //?????? /// These must be getting included in netdb.h and arpa/inet.h (all)
//#include <sys/socket.h> //??????
//#include <netinet/in.h> //??????
#include <netdb.h>
#include <arpa/inet.h>
#include <stdlib.h> // Needed with c++99; for some reason included in other library for c++11/0x
#include <unistd.h> // Needed for close()

/*****************************************************************************
* class Connection: provides a basic TCP connection.
*****************************************************************************/
class Connection
{
public:
   std::string strError;
   
   /*****************************************************************************
   * Initializes the connection object with standard defaults
   *****************************************************************************/
   Connection()
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
      stSockAddr2 = NULL;
   }
   
   /*****************************************************************************
   * Initialize and copy a connection; this only copies the data necessary
   * to work with an already established connection
   *****************************************************************************/
   Connection(const Connection& toCopy)
   {
      sockfdComm = toCopy.sockfdComm;
      isServer = false; // Don't copy server capabilities to an individual connection.
                        // This will prevent a connection from closing the listening socket.
      sockfdListen = -1; // Don't copy server capabilities to an individual connection.
      in = toCopy.in;
      primarySockfd = &sockfdComm;
      port = toCopy.port;
      portStr = NULL;
      address = NULL;
      stSockAddr = toCopy.stSockAddr;
      hints = toCopy.hints;
      stSockAddr1 = NULL; // These should not be needed.
      stSockAddr2 = NULL; // These should not be needed.
   }
   
   /*****************************************************************************
   * Initialize a connection object using a socket descriptor.
   * This is made to only use sockets that already are connected
   * This was built to dump server connections into individual threads
   *****************************************************************************/
   Connection(int comSocket)
   {
      isServer = false;
      sockfdComm = comSocket;
      sockfdListen = -1;
      strError = "";
      primarySockfd = &sockfdComm;
      port = 0;
      portStr = NULL;
      address = NULL;
      memset(&stSockAddr, '\0', sizeof stSockAddr);
      memset(&hints, 0, sizeof hints);
      stSockAddr1 = NULL;
      stSockAddr2 = NULL;
   }
   
   /*****************************************************************************
   * Copy a connection; this only copies the data necessary to work with an
   * already established connection
   *****************************************************************************/
   Connection& operator=(const Connection& toCopy)
   {
      // Close an existing socket descriptor...
      close(sockfdComm);
      sockfdComm = toCopy.sockfdComm;
      isServer = false; // Don't copy server capabilities to an individual connection.
                        // This will prevent a connection from closing the listening socket.
      sockfdListen = -1; // Don't copy server capabilities to an individual connection.
      in = toCopy.in;
      primarySockfd = &sockfdComm;
      port = toCopy.port;
      if (address != NULL)
         delete address;
      if (portStr != NULL)
         delete portStr;
      if (stSockAddr1 != NULL)
         freeaddrinfo(stSockAddr1);
      portStr = NULL;
      address = NULL;
      stSockAddr = toCopy.stSockAddr;
      hints = toCopy.hints;
      stSockAddr1 = NULL; // These should not be needed.
      stSockAddr2 = NULL; // These should not be needed.
      return *this;
   }
   
   /*****************************************************************************
   * Closes connection and socket
   *****************************************************************************/
   ~Connection()
   {
      close(sockfdComm);
      if (isServer)
      {
         close(sockfdListen);
      }
      if (address != NULL)
         delete address;
      if (portStr != NULL)
         delete portStr;
      if (stSockAddr1 != NULL)
         freeaddrinfo(stSockAddr1);
   }
   
   /*****************************************************************************
   * Closes communication socket. For good cleanup possibility
   *****************************************************************************/
   int closeComm()
   {
      int status = 0;
      // Shutdown interacts with the network layer, and doesn't just disconnect the socket descriptor.
      status = shutdown(sockfdComm, 2);
      // 0: Stop receiving data for this socket. If further data arrives, reject it.
      // 1: Stop trying to transmit data from this socket. Discard any data waiting to be sent. Stop looking for acknowledgement of data already sent; don't retransmit it if it is lost.
      // 2: Stop both reception and transmission. 
      return (status == 0);
   }
   /*****************************************************************************
   * Closes server socket. For good cleanup possibility
   *****************************************************************************/
   int closeServer()
   {
      int status = 0;
      if (isServer)
      {
         // Shutdown interacts with the network layer, and doesn't just disconnect the socket descriptor.
         status = shutdown(sockfdListen, 2);
         // 0: Stop receiving data for this socket. If further data arrives, reject it.
         // 1: Stop trying to transmit data from this socket. Discard any data waiting to be sent. Stop looking for acknowledgement of data already sent; don't retransmit it if it is lost.
         // 2: Stop both reception and transmission. 
      }
      return (status == 0);
   }
   
   /*****************************************************************************
   * Sets the connection up for a client
   * returns 1 if successful.
   *****************************************************************************/
   int clientSetup(const char* theAddress, const char* thePort)
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
      
      // Test the possible addresses, and use the one that works.
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
      return 1;
   }
   
   /*****************************************************************************
   * Sets the connection up for a server
   * returns 1 if successful.
   *****************************************************************************/
   int serverSetup(const char* thePort)
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
      
      // Test the possible addresses, and use the one that works.
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
      return 1;
   }
   
   /*****************************************************************************
   * Gets and returns a new connection socket
   * returns 1 if successful.
   *****************************************************************************/
   int serverConnection(Connection& newNet)
   {
      if (!isServer)
      {
         return 0;
      }
      if (!(doListen() && doAccept()))
      {
         return 0;
      }
      newNet = *this; //Copy the connection off to the argument object
      sockfdComm = -1; // Clear out this copy of the socket so the server doesn't accidentally close it.
      return 1;
   }
   
   /*****************************************************************************
   * Gets and returns a new connection socket
   * returns 1 if successful.
   *****************************************************************************/
   int serverConnection(int& newSocket)
   {
      if (!isServer)
      {
         return 0;
      }
      if (!(doListen() && doAccept()))
      {
         return 0;
      }
      newSocket = sockfdComm; //Copy the connection off to the argument object
      sockfdComm = -1; // Clear out this copy of the socket so the server doesn't accidentally close it.
      return 1;
   }

   /*****************************************************************************
   * Sends character(s) through communication socket
   * returns > 1 if successful.
   *****************************************************************************/
   int sendChar(char* toSend)
   {
      int error = 1;
      strError = "send failed";
      error = send(sockfdComm, toSend, strlen(toSend), 0);
      return (error >= 0) ? error : 0;
   }
   
   /*****************************************************************************
   * Receives character(s) through the communication socket
   * returns > 1 if successful.
   *****************************************************************************/
   int receiveChar(char* received)
   {
      int error = 1;
      strError = "receive failed";
      error = recv(sockfdComm, received, strlen(received), 0);
      strError = error == 0 ? "socket closed" : "receive failed";
      return (error > 0) ? error : 0;
   }
   
   /*****************************************************************************
   * Sends data through the communication socket
   * returns > 1 if successful.
   *****************************************************************************/
   template <class T>
   int sendData(T* toSend, unsigned int numBytes)
   {
      int error = 1;
      strError = "send failed";
      // Send the data from the supplied locations
      error = send(sockfdComm, toSend, numBytes, 0);
      return (error >= 0) ? error : 0;
   }
   
   /*****************************************************************************
   * Receives data through the communication socket
   * returns > 1 if successful.
   *****************************************************************************/
   template <class T>
   int receiveData(T* received, unsigned int numBytes)
   {
      int error = 1;
      strError = "receive failed";
      // Wait for data to come, and read it into the provided locations
      error = recv(sockfdComm, received, numBytes, 0);
      strError = error == 0 ? "socket closed" : "receive failed";
      return (error > 0) ? error : 0;
   }
   
private:
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
   
   /*****************************************************************************
   * Sets the connection address and port
   * returns 1 if successful.
   *****************************************************************************/
   int setupManual()
   {
      sockfdComm = -1;
      sockfdListen = -1;
      
      // Set up the hints for socket data - what settings do we need for the connection?
      hints.ai_family = AF_UNSPEC; // AF_INET for IPv4, AF_INET6 for IPv6, AF_UNSPEC for "I don't care which"
      hints.ai_socktype = SOCK_STREAM;
      hints.ai_protocol = IPPROTO_TCP;
      hints.ai_canonname = NULL; // Initialize pointer to NULL
      hints.ai_addr = NULL; // Initialize pointer to NULL
      hints.ai_next = NULL; // Initialize pointer to NULL
      
      stSockAddr.sin_family = AF_INET;
      stSockAddr.sin_port = htons(port);
      
      // Socket function call - get all possible combinations of data based on the hints
      getaddrinfo(address, portStr, &hints, &stSockAddr1);
      
      return 1;
   }
   
   /*****************************************************************************
   * Get the socket descriptor
   * returns 1 if successful.
   *****************************************************************************/
   int getSocketDesc()
   {
      // Request a socket descriptor
      *primarySockfd = socket(stSockAddr2->ai_family, stSockAddr2->ai_socktype, stSockAddr2->ai_protocol);
      strError = "getSocketDesc";
      return (*primarySockfd >= 0);
   }
   
   /*****************************************************************************
   * Configures the socket so that the port is reuseable
   * returns 1 if successful.
   *****************************************************************************/
   int reuseableSock()
   {
      int yes = 1;
      int error = 1;
      // Set socket option: allow the address (the port and address combination) to be reused
      error = setsockopt(*primarySockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int));
      // Set socket option: allow the port to be reused no matter what
      error = setsockopt(*primarySockfd,SOL_SOCKET,SO_REUSEPORT,&yes,sizeof(int));
      strError = "setSockOpt";
      return (error >= 0);
   }
   
   /*****************************************************************************
   * Binds the connection and socket
   * returns 1 if successful.
   *****************************************************************************/
   int bindConn()
   {
      int error = 1;
      // Bind the program to a specific socket and address that can get incoming connections
      error = ::bind(sockfdListen, stSockAddr2->ai_addr, stSockAddr2->ai_addrlen);
      strError = "bind failed: ";
      strError += strerror(error);
      return (error >= 0);
      return true;
   }
   
   /*****************************************************************************
   * Creates the connection
   * returns 1 if successful.
   *****************************************************************************/
   int getConnect()
   {
      int error = 1;
      // Try to establish a connection to another socket
      error = connect(sockfdComm, stSockAddr2->ai_addr, stSockAddr2->ai_addrlen);
      strError = "connect failed";
      return (error >= 0);
   }
   
   /*****************************************************************************
   * Listens for a connection
   * returns 1 if successful.
   *****************************************************************************/
   int doListen()
   {
      int backlog = 2;
      int error = 1;
      // Wait for an incoming connection
      error = listen(sockfdListen, backlog);
      strError = "listen failed: ";
      strError += strerror(error);
      return (error >= 0);
   }
   
   /*****************************************************************************
   * Accepts 2 connections
   * returns 1 if successful.
   *****************************************************************************/
   int doAccept()
   {
      socklen_t addr_size = sizeof(in);
      // Accept an incoming connection
      sockfdComm = accept(sockfdListen, (sockaddr *)&in, &addr_size);
      strError = "accept failed";
      return (sockfdComm >= 0);
   }
};

#endif //_CONNECTION_H
