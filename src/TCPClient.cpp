#include "TCPClient.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <fcntl.h>
#include "exceptions.h"

//geeksforgeeks.org/socket-programming-cc/

/**********************************************************************************************
 * TCPClient (constructor) - Creates a Stdin file descriptor to simplify handling of user input. 
 *
 **********************************************************************************************/

TCPClient::TCPClient() : Client() {

}

/**********************************************************************************************
 * TCPClient (destructor) - No cleanup right now
 *
 **********************************************************************************************/

TCPClient::~TCPClient() {

}

/**********************************************************************************************
 * connectTo - Opens a File Descriptor socket to the IP address and port given in the
 *             parameters using a TCP connection.
 *
 *    Throws: socket_error exception if failed. socket_error is a child class of runtime_error
 **********************************************************************************************/

void TCPClient::connectTo(const char *ip_addr, unsigned short port) {
    if((this->_connectionFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        throw socket_error("Socket creation error\n");
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if(inet_pton(AF_INET, ip_addr, &this->serv_addr.sin_addr) <= 0) {
        throw socket_error("Invalid address or address not supported\n");
    }

    if(connect(this->_connectionFd, (struct sockaddr *)&this->serv_addr, sizeof(this->serv_addr)) < 0) {
        throw socket_error("Connection error\n");
    }
}

/**********************************************************************************************
 * handleConnection - Performs a loop that checks if the connection is still open, then 
 *                    looks for user input and sends it if available. Finally, looks for data
 *                    on the socket and sends it.
 * 
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPClient::handleConnection() {
    
    while(true) {
        if(isOpen()) {
            char buffer[socket_bufsize] = {0};
            //looking for data from server
            read(this->_connectionFd, buffer, socket_bufsize);
            //output what was received
            std::cout << buffer;
            fflush(stdout);

            //read in a line from stdin and format it to send to server
            std::string cmd;
            std::getline(std::cin,cmd);
            int n = cmd.length();
            char cmdBuf[n+1];
            strncpy(cmdBuf, cmd.c_str(), n);
            cmdBuf[n+1] = '\0';

            //send command to server
            write(this->_connectionFd, cmdBuf, n+1);
        } else {
            close(this->_connectionFd);
            return;
        }
    }
}

/**********************************************************************************************
 * isOpen - Checks if the server connection is still open. Doesn't seem to work until 2 commands have been sent.
 *
 **********************************************************************************************/
bool TCPClient::isOpen() {
    if((fcntl(this->_connectionFd, F_GETFD) == -1) && errno == EBADF) {
        return false;
    } 
    return true;
}

/**********************************************************************************************
 * closeConnection - Closes the connection to the server.
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPClient::closeConn() {
    if(close(this->_connectionFd) == -1) {
        throw socket_error("Error closing socket\n");
    }
}


