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

    int opt = 1;   
    int  new_socket, activity, valread;
    std::unique_ptr<int> sd;   
    int max_sd;   
    struct sockaddr_in address;   
         
    char buffer[1025];  //data buffer of 1K  
         
    //set of socket descriptors  
      
         
    //a message  
    char *message = "ECHO Daemon v1.0 \r\n";   
     
    //initialise all client_socket[] to 0 so not checked  
    for (int i = 0; i < this->_max_clients; i++)   
    {   
        this->_connlist.push_back(std::make_unique<int>(0));
 
    }   
           
         
    //try to specify maximum of 3 pending connections for the master socket  
    if (listen(this->_server_fd, 3) < 0)   
    {   
        perror("listen");   
        exit(EXIT_FAILURE);   
    }   
         
    //accept the incoming connection  
    //addrlen = sizeof(address);   
    puts("Waiting for connections ...");   
         
    while(1)   
    {   
        //clear the socket set  
        FD_ZERO(&this->_readfds);   
     
        //add master socket to set  
        FD_SET(this->_server_fd, &this->_readfds);   
        max_sd = this->_server_fd;   
             
        //add child sockets to set  
        for (int i = 0 ; i < this->_max_clients ; i++)   
        {   
            //socket descriptor  
            sd = std::move(this->_connlist.at(i)); 
      
            //if valid socket descriptor then add to read list  
            if(*sd > 0)   
                FD_SET( *sd , &this->_readfds);   
                 
            //highest file descriptor number, need it for the select function  
            if(*sd > max_sd)   
                max_sd = *sd;   
        }   
     
        //wait for an activity on one of the sockets , timeout is NULL ,  
        //so wait indefinitely  
        activity = select( max_sd + 1 , &this->_readfds , NULL , NULL , NULL);   
       
        if ((activity < 0) && (errno!=EINTR))   
        {   
            printf("select error");   
        }   
             
        //If something happened on the master socket ,  
        //then its an incoming connection  
        if (FD_ISSET(this->_server_fd, &this->_readfds))   
        {   
            if ((new_socket = accept(this->_server_fd,  
                    (struct sockaddr *)&this->_address, (socklen_t*)&this->addrlen))<0)   
            {   
                perror("accept");   
                exit(EXIT_FAILURE);   
            }   
             
            //inform user of socket number - used in send and receive commands  
            printf("New connection , socket fd is %d , ip is : %s , port : %d \n" , new_socket , inet_ntoa(address.sin_addr) , ntohs 
                  (address.sin_port));   
           
            //send new connection greeting message  
            if( send(new_socket, message, strlen(message), 0) != strlen(message) )   
            {   
                perror("send");   
            }   
                 
            puts("Welcome message sent successfully");   
                 
            //add new socket to array of sockets  
            this->_connlist.push_back(std::make_unique<int>(new_socket));
            //for (int i = 0; i < this->_max_clients; i++)   
            //{   
                //if position is empty  
               // if( client_socket[i] == 0 )   
               // {   
                //    client_socket[i] = new_socket;   
                //    printf("Adding to list of sockets as %d\n" , i);   
                         
                //    break;   
                //}   
            //}   
        }   
             
        //else its some IO operation on some other socket 
        for (int i = 0; i < this->_max_clients; i++)   
        {   
std::cout << "right here\n";
            sd = std::move(this->_connlist.at(i));   
std::cout << "right after\n";
            if (FD_ISSET( *sd , &this->_readfds))   
            {   
                //Check if it was for closing , and also read the  
                //incoming message  
                if ((valread = read( *sd , buffer, 1024)) == 0)   
                {   
                    //Somebody disconnected , get his details and print  
                    getpeername(*sd , (struct sockaddr*)&address , \ 
                        (socklen_t*)&addrlen);   
                    printf("Host disconnected , ip %s , port %d \n" ,  
                          inet_ntoa(address.sin_addr) , ntohs(address.sin_port));   
                         
                    //Close the socket and mark as 0 in list for reuse  
                    close( *sd );   
                    this->_connlist.at(i) = 0;   
                }   
                     
                //Echo back the message that came in  
                else 
                {   
                    //set the string terminating NULL byte on the end  
                    //of the data read  
                    buffer[valread] = '\0';   
                    send(*sd , buffer , strlen(buffer) , 0 );   
                }   
            }   
            std::cout << "after for\n";
        }   
    }   



    // fd_set readfds;
    // int max_sd;

    // if(listen(this->_server_fd, 3) < 0) {
    //     throw socket_error("Listen failure\n");
    // }
    // while(true) {
    //     FD_ZERO(&readfds);
    //     max_sd = this->_server_fd;
    //     int new_socket;
    //     if((new_socket = accept(this->_server_fd, (struct sockaddr *)&this->_address, (socklen_t*)&this->addrlen)) < 0){
    //         throw socket_error("Accept failure\n");
    //     }
        
    //     this->_connlist.push_back(std::make_unique<int>(new_socket));

    //     std::cout << "new connection " << new_socket << "\n";

    //     char buffer[1024] = {0};
    //     read(new_socket, buffer, 1024);
    //     std::cout << "stuff read on server side " << buffer << "\n";

    //     char* hello = "Hello from server\n\nPossible commands:\n1\n2\n3\n4\n5\npasswd\nexit\nmenu\n";
    //     write(new_socket, hello, strlen(hello));

    //     read(new_socket, buffer, 1024);
    //     std::cout << "more came in " << buffer << "\n";

    // }
    
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
