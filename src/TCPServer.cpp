#include "TCPServer.h"
#include "exceptions.h"
#include <arpa/inet.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>


//geeksforgeeks.org/socket-programming-cc/
//for testing
#include <iostream>

TCPServer::TCPServer() : Server() {
}


TCPServer::~TCPServer() {

}

/**********************************************************************************************
 * bindSvr - Creates a network socket and sets it nonblocking so we can loop through looking for
 *           data. Then binds it to the ip address and port
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPServer::bindSvr(const char *ip_addr, short unsigned int port) {

    if((_server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        throw socket_error("Socket failed\n");
    }

    _address.sin_family = AF_INET;
    if(inet_pton(AF_INET, ip_addr, &_address.sin_addr.s_addr) <= 0){
        throw socket_error("Invaliad address\n");
    }
    _address.sin_port = htons(port);

    int flags;
    //flags = fcntl(_server_fd, F_GETFL);
    //if(fcntl(_server_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
      //  throw socket_error("fcntl failed\n");
    //}
    if(ioctl(_server_fd, FIONBIO, &flags) == -1 ){
        throw socket_error("ioctl failed\n");
    }

    if(bind(_server_fd, (struct sockaddr *)&_address, sizeof(_address)) < 0) {
        throw socket_error("Bind failed\n");
    }
   
}

/**********************************************************************************************
 * listenSvr - Performs a loop to look for connections and create TCPConn objects to handle
 *             them. Also loops through the list of connections and handles data received and
 *             sending of data. 
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPServer::listenSvr() {

    while(true) {
        if(listen(_server_fd, 20) < 0) {
            throw socket_error("Listen failure\n");
        }
        
        if((_new_socket = accept(_server_fd, (struct sockaddr *)&_address, (socklen_t*)&addrlen)) < 0){
            throw socket_error("Accept failure\n");
        }
        
        _connlist.push_back(std::make_unique<int>(_new_socket));

        std::cout << "new connection " << _new_socket << "\n";

        char buffer[1024] = {0};
        read(_new_socket, buffer, 1024);
        std::cout << "stuff read on server side " << buffer << "\n";

        char* hello = "Hello from server\n\nPossible commands:\n1\n2\n3\n4\n5\npasswd\nexit\menu\n";
        write(_new_socket, hello, strlen(hello));

        read(_new_socket, buffer, 1024);
        std::cout << "more came in " << buffer << "\n";

    }
    
}

/**********************************************************************************************
 * shutdown - Cleanly closes the socket FD.
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPServer::shutdown() {
    if(close(_server_fd) == -1) {
        throw socket_error("Error closing socket\n");
    }
}
