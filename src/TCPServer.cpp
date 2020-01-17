#include "TCPServer.h"
#include "exceptions.h"
#include <arpa/inet.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
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

    if((this->_server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        throw socket_error("Socket failed\n");
    }

    _address.sin_family = AF_INET;
    if(inet_pton(AF_INET, ip_addr, &this->_address.sin_addr.s_addr) <= 0){
        throw socket_error("Invaliad address\n");
    }

    _address.sin_port = htons(port);
    if(bind(this->_server_fd, (struct sockaddr *)&this->_address, sizeof(this->_address)) < 0) {
        throw socket_error("Bind failed\n");
    }
    fcntl(this->_server_fd, F_SETFL | O_NONBLOCK); 
}

/**********************************************************************************************
 * listenSvr - Performs a loop to look for connections and create TCPConn objects to handle
 *             them. Also loops through the list of connections and handles data received and
 *             sending of data. 
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/
//https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/ was used for the below solution.
void TCPServer::listenSvr() {
    int  new_socket, activity ,valread ,sd, max_sd;   
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 1000;

    //set all client sockets to zero so they are not checked
    for (int i = 0; i < this->_max_clients; i++) {   
        this->_client_socket[i] = 0;   
    }   
    
    //try to specify maximum of 3 pending connections for the master socket  
    if (listen(this->_server_fd, 3) < 0) {   
        throw socket_error("Listen failure\n");
    }    
         
    while(true) {   
        //clear the socket set  
        FD_ZERO(&this->_readfds);   
     
        //add master socket to set  
        FD_SET(this->_server_fd, &this->_readfds);   
        max_sd = this->_server_fd;   
             
        //add child sockets to set  
        for (int i = 0 ; i < this->_max_clients ; i++) {   
            sd = this->_client_socket[i];
                 
            //if valid socket descriptor then add to read list  
            if(sd > 0)   
                FD_SET(sd, &this->_readfds);   
                 
            //highest file descriptor number, need it for the select function  
            if(sd > max_sd)   
                max_sd = sd;   
        }   
     
        //wait for an activity on one of the sockets
        activity = select(max_sd+1, &this->_readfds, NULL, NULL, &timeout);   
       
        if ((activity < 0) && (errno!=EINTR)) {   
            throw socket_error("Select error\n");
        }   
             
        //If something happened on the master socket, then its an incoming connection  
        if (FD_ISSET(this->_server_fd, &this->_readfds)) {   
            if ((new_socket = accept(this->_server_fd, (struct sockaddr *)&this->_address, (socklen_t*)&this->_addrlen))<0) { 
                throw socket_error("Accept failure\n");
            }   
            std::cout << "New connection!\n";
            sendCommands(new_socket); 
                 
            //add the new client
            for(int i = 0; i < this->_max_clients; i++) {   
                //if position is empty  
                if(this->_client_socket[i] == 0 ) {   
                    this->_client_socket[i] = new_socket;      
                    break;   
                }   
            }   
        }   
             
        //else its some IO operation on some other socket 
        for (int i = 0; i < this->_max_clients; i++) {   
            sd = this->_client_socket[i];   
                 
            if(FD_ISSET(sd , &this->_readfds)) {  
                char buffer[1025];  //data buffer of 1K   
                //Check if it was for closing, and also read the incoming message  
                if ((valread = read(sd, buffer, 1024)) == 0) {   
                    //Somebody disconnected, so close the socket and mark as 0 in list for reuse 
                    if(close(sd) == -1) {
                        throw socket_error("Error closing client socket\n");
                    } 
                    this->_client_socket[i] = 0;   
                    std::cout << "A client disconnected\n";
                } else {   
                    //Otherwise there is a request that needs to be handled
                    handleCommands(sd, buffer, valread, i);
                }   
            }   
        }   
    } 
}

/**********************************************************************************************
 * sendCommands - Sends the list of server commands to the clients.
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/
void TCPServer::sendCommands(int clientFd){
    char *commands = "Server commands:\n\thello\n\t1\n\t2\n\t3\n\t4\n\t5\n\tpasswd\n\texit\n\tmenu\n";
    if(send(clientFd, commands, strlen(commands), 0) != strlen(commands)){   
        throw socket_error("Send error\n");
    } 
}

/**********************************************************************************************
 * handleCommands - Processes command from client and sends the correct response.
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/
void TCPServer::handleCommands(int clientFd, char *buffer, int cmdSize, int clientSocketNum) {
    std::string cmd = "";

    //copying to string for comparison minus the newline
    for(int i = 0; i < cmdSize-1; i++) {
        cmd += buffer[i];      
    }

   if(cmd.compare("hello") == 0) {
        char *response = "Hello from server!\n\0";
        send(clientFd, response, strlen(response), 0);
   } else if(cmd.compare("1") == 0) {
        char *response = "Option 1\n\0";
        send(clientFd, response, strlen(response), 0);
   } else if(cmd.compare("2") == 0) {
        char *response = "Option 2\n\0";
        send(clientFd, response, strlen(response), 0);
   } else if(cmd.compare("3") == 0) {
        char *response = "Option 3\n\0";
        send(clientFd, response, strlen(response), 0);
   } else if(cmd.compare("4") == 0){
        char *response = "Option 4\n\0";
        send(clientFd, response, strlen(response), 0);
   } else if(cmd.compare("5") == 0) {
        char *response = "Option 5\n\0";
        send(clientFd, response, strlen(response), 0);
   } else if(cmd.compare("passwd") == 0) {
        char *response = "passwd functionality not yet implemented\n\0";
        send(clientFd, response, strlen(response), 0);
   } else if(cmd.compare("exit") == 0) {
        char *response = "Disconnecting client\n\0";
        send(clientFd, response, strlen(response), 0);
        
        if(close(clientFd) == -1) {
            throw socket_error("Error closing client socket\n");
        }
        this->_client_socket[clientSocketNum] = 0;
   } else if(cmd.compare("menu") == 0) {
        sendCommands(clientFd);
   } else {
        char *response = "Invalid selection\n\0";
        send(clientFd,  response, strlen(response), 0);  
   }
}

/**********************************************************************************************
 * shutdown - Cleanly closes the socket FD.
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPServer::shutdown() {
    if(close(this->_server_fd) == -1) {
        throw socket_error("Error closing socket\n");
    }
}
