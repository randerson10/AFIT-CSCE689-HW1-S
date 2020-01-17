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

void TCPServer::listenSvr() {

//https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/
  
    int  new_socket, activity ,valread ,sd, max_sd;   
  
    //set of socket descriptors  
    fd_set readfds;   
         
    //a message  
    char *message = "ECHO Daemon v1.0 \r\n";   
     
    //initialise all client_socket[] to 0 so not checked  
    for (int i = 0; i < this->_max_clients; i++) {   
        this->_connlist.push_back(0);
        this->_client_socket[i] = 0;   
    }   
         

    //try to specify maximum of 3 pending connections for the master socket  
    if (listen(this->_server_fd, 3) < 0) {   
        throw socket_error("Listen failure\n");
    }   
         
    //accept the incoming connection  
    //addrlen = sizeof(address);   
    puts("Waiting for connections ...");   
         
    while(true) {   
        //clear the socket set  
        FD_ZERO(&readfds);   
     
        //add master socket to set  
        FD_SET(this->_server_fd, &readfds);   
        max_sd = this->_server_fd;   
             
        //add child sockets to set  
        for (int i = 0 ; i < this->_max_clients ; i++) {   
            //socket descriptor  
            //sd = this->_connlist.at(i); 
            sd = this->_client_socket[i];
                 
            //if valid socket descriptor then add to read list  
            if(sd > 0)   
                FD_SET( sd , &readfds);   
                 
            //highest file descriptor number, need it for the select function  
            if(sd > max_sd)   
                max_sd = sd;   
        }   
     
        //wait for an activity on one of the sockets , timeout is NULL ,  
        //so wait indefinitely  
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 1000;
        activity = select( max_sd + 1 , &readfds , NULL , NULL , &timeout);   
       
        if ((activity < 0) && (errno!=EINTR)) {   
            throw socket_error("Select error\n");
        }   
             
        //If something happened on the master socket ,  
        //then its an incoming connection  
        if (FD_ISSET(this->_server_fd, &readfds)) {   
            if ((new_socket = accept(this->_server_fd, (struct sockaddr *)&this->_address, (socklen_t*)&this->_addrlen))<0) { 
                throw socket_error("Accept failure\n");
            }   
             
            //inform user of socket number - used in send and receive commands  
            printf("New connection , socket fd is %d , ip is : %s , port : %d  \n" , new_socket , inet_ntoa(this->_address.sin_addr) , ntohs(this->_address.sin_port));   
           
            //send new connection greeting message  
            //if( send(new_socket, message, strlen(message), 0) != strlen(message) )   
            //{   
            //    perror("send");   
            //}   
            TCPServer::sendCommands(new_socket);
                 
            puts("Welcome message sent successfully");   
                 
            //add new socket to array of sockets  
            for(int i = 0; i < this->_max_clients; i++) {   
                //if position is empty  
                if(this->_client_socket[i] == 0 ) {   
                    this->_client_socket[i] = new_socket;   
                    printf("Adding to list of sockets as %d\n" , i);     
                    break;   
                }   
            }   
        }   
             
        //else its some IO operation on some other socket 
        for (int i = 0; i < this->_max_clients; i++) {   
            sd = this->_client_socket[i];   
                 
            if(FD_ISSET( sd , &readfds)) {  
                char buffer[1025];  //data buffer of 1K   
                //Check if it was for closing , and also read the  
                //incoming message  
                if ((valread = read( sd , buffer, 1024)) == 0) {   
                    //Somebody disconnected , get his details and print  
                    getpeername(sd , (struct sockaddr*)&this->_address , (socklen_t*)&this->_addrlen);   
                    printf("Host disconnected , ip %s , port %d \n" , inet_ntoa(this->_address.sin_addr) , ntohs(this->_address.sin_port));   
                         
                    //Close the socket and mark as 0 in list for reuse  
                    close(sd);   
                    this->_client_socket[i] = 0;   
                }   
                     
                //Echo back the message that came in  
                else {   
                    //set the string terminating NULL byte on the end  
                    //of the data read  
                    //buffer[valread] = '\0';   
                    //send(sd , buffer , strlen(buffer) , 0 );   
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
    char *message = "ECHO Daemon v1.0 \r\n"; 
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

    for(int i = 0; i < cmdSize-1; i++) {
        cmd += buffer[i];      
    }

   if(cmd.compare("hello") == 0) {
        std::cout << "hello selected\n";
        char *response = "Hello from server\n\0";
        send(clientFd, response, strlen(response), 0);
   } else if(cmd.compare("1") == 0) {
        std::cout << "1 selected\n";
        char *response = "Option 1\n\0";
        send(clientFd, response, strlen(response), 0);
   } else if(cmd.compare("2") == 0) {
        std::cout << "2 selected\n";
        char *response = "Option 2\n\0";
        send(clientFd, response, strlen(response), 0);
   } else if(cmd.compare("3") == 0) {
        std::cout << "3 selected\n";
        char *response = "Option 3\n\0";
        send(clientFd, response, strlen(response), 0);
   } else if(cmd.compare("4") == 0){
        std::cout << "4 selected\n";
        char *response = "Option 4\n\0";
        send(clientFd, response, strlen(response), 0);
   } else if(cmd.compare("5") == 0) {
        std::cout << "5 selected\n";
        char *response = "Option 5\n\0";
        send(clientFd, response, strlen(response), 0);
   } else if(cmd.compare("passwd") == 0) {
        std::cout << "passwd selected\n";
        char *response = "passwd functionality not yet implemented\n\0";
        send(clientFd, response, strlen(response), 0);
   } else if(cmd.compare("exit") == 0) {
        std::cout << "exit selected\n";
        char *response = "Disconneting client\n\0";
        send(clientFd, response, strlen(response), 0);
        
        if(close(clientFd) == -1) {
            throw socket_error("Error closing client socket\n");
        }
        this->_client_socket[clientSocketNum] = 0;
   } else if(cmd.compare("menu") == 0) {
        std::cout << "menu selected\n";
        sendCommands(clientFd);
   } else {
        std::cout << "invalid\n"; 
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

// bool TCPServer::hasData(long ms_timeout) {
//     fd_set read_fds
//     timeout_tv_sec = 0;
//     timeout.tv_usec = ms_timout;
//     select()
// }
