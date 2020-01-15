#ifndef TCPSERVER_H
#define TCPSERVER_H

#include "Server.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <list> 
#include <memory>

class TCPServer : public Server 
{
public:
   TCPServer();
   ~TCPServer();

   void bindSvr(const char *ip_addr, unsigned short port);
   void listenSvr();
   void shutdown();

private:
   std::list<std::unique_ptr<int>> _connlist;

   int _server_fd, _new_socket, _valread;
   struct sockaddr_in _address;
   int addrlen = sizeof(_address);
 


};


#endif
