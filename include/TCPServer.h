#ifndef TCPSERVER_H
#define TCPSERVER_H

#include "Server.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector> 
#include <memory>

class TCPServer : public Server 
{
public:
   TCPServer();
   ~TCPServer();

   void bindSvr(const char *ip_addr, unsigned short port);
   void listenSvr();
   void shutdown();
   void sendCommands(int clientFd);
   void handleCommands(int clientFd, char *buffer, int cmdSize, int clientSocketNum);

private:
   std::vector<int> _connlist;
   int _client_socket[20];
   int _server_fd;
   int _max_clients = 20;
   struct sockaddr_in _address;
   int _addrlen = sizeof(_address);
   fd_set _readfds; 
};


#endif
