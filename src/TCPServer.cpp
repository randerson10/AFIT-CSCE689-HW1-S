#include "TCPServer.h"
#include "exceptions.h"
#include <arpa/inet.h>
//#include <sys/socket.h>

#include <unistd.h>

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
    //int server_fd, new_socket, valread;
    //struct sockaddr_in address;
    //int opt = 1;
    //int addrlen = sizeof(address);
    //char buffer[1024] = {0};

    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        throw socket_error("Socket failed\n");
    }

    address.sin_family = AF_INET;
    if(inet_pton(AF_INET, ip_addr, &address.sin_addr.s_addr) <= 0){
        throw socket_error("Invaliad address\n");
    }
    address.sin_port = htons(port);

    if(bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
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
    if(listen(server_fd, 3) < 0) {
        throw socket_error("Listen failure\n");
    }

    if((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)sizeof(address))) < 0){
        throw socket_error("Accet failure\n");
    }

    char buffer[1024] = {0};
    read(new_socket, buffer, 1024);
    std::cout << buffer << "\n";
}

/**********************************************************************************************
 * shutdown - Cleanly closes the socket FD.
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPServer::shutdown() {
    if(close(server_fd) == -1) {
        throw socket_error("Error closing socket\n");
    }
}
