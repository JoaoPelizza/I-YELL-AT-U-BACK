/*tcp_server*/
#include <stdio.h>
#include <string.h>
#include <iostream>
#include "sock_init.hpp"
#include <ctype.h>

int main(int argc, char *argv[]){

    #if defined(_WIN32)
        WSADATA d;
        if(WSAStratup(MAKEWORD(2,2),&d)){
            fprintf(stderr, "Failed initializa.\n");
            return 1;
        }
    #endif

    printf("Configuring local address...\n");
    struct addrinfo hints;
    memset(&hints,0,sizeof(hints));
    hints.ai_family=AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo *bind_address;
    getaddrinfo(0,argv[0],&hints,&bind_address);

    printf("Creating socket...\n");
    SOCKET socket_listen;
    socket_listen = socket(bind_address->ai_family,
                    bind_address->ai_socktype,
                    bind_address->ai_protocol);
    
    if(!ISVALIDSOCKET(socket_listen)){
        std::cout << stderr << " Socket() failed err: " << GETSOCKETERRNO();
        return 1;
    }


    if(bind(socket_listen,
            bind_address->ai_addr,bind_address->ai_addrlen)){
                std::cout << stderr << " bind() failed err: " << GETSOCKETERRNO();
                return 1;
            }
    freeaddrinfo(bind_address);

    std::cout << "Listening\n";

    if (listen(socket_listen, 10) < 0) {
        fprintf(stderr, "listen() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    fd_set master, reads;
    FD_ZERO(&master);
    FD_SET(socket_listen,&master);
    SOCKET max_socket = socket_listen;

    std::cout << "Waiting for connections\n";

    while(true){
        reads= master;
        if(select(max_socket+1,&reads,0,0,0) < 0 ){
            std::cout << stderr << " select() failed err: " << GETSOCKETERRNO();
            return 1;
        }

        SOCKET i;

        for(i = 1; i<=max_socket; i++){
            if(FD_ISSET(i,&reads)){
                //handle socket
                if (i==socket_listen){
                    struct sockaddr_storage client_address;
                    socklen_t client_len = sizeof(client_address);
                    SOCKET socket_client = accept(socket_listen,(struct sockaddr*) &client_address, &client_len);
                    
                    if(!ISVALIDSOCKET(socket_listen)){
                        std::cout << stderr << " Socket() failed err: " << GETSOCKETERRNO();
                        return 1;
                    }
                    
                    FD_SET(socket_client, &master);
                    if(socket_client > max_socket){
                        max_socket = socket_client;
                    }
                        char address_buffer[100];
                        getnameinfo((struct sockaddr*)&client_address,
                                    client_len,
                                    address_buffer, sizeof(address_buffer),
                                    0,0,NI_NUMERICHOST);
                        std::cout << "new connection from: " << address_buffer << std::endl; 
                }else{
                    char read[1024];
                    int bytes_received = recv(i,read,1024,0);
                    if(bytes_received < 1){
                        FD_CLR(i,&master);
                        CLOSESOCKET(i);
                        continue;
                    }
                    int j;
                    for(j=0;j<bytes_received;j++){
                        read[j] = toupper(read[j]);
                    }
                    send(i,read,bytes_received,0);
                }//else

            }//FD SET
        }
    }//while true

    std::cout << "Closing listening socket\n";
    CLOSESOCKET(socket_listen);
    
    #if defined(_WIN32)
        WSACleanup();
    #endif

    std::cout << "Closing\n";
    return 0;

}//while true
