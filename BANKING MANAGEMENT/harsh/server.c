#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include<stdbool.h>
#include<string.h>
#include "customer.h"
#include "employee.h"
#include "manager.h"
#include "admin.h"



#define PORT 8080
#define MAX_CLIENTS 6
#define BUFFER_SIZE 1024

void handle_client(int client_socket);

int main(){
    
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
     if(server_fd == -1){
         perror("socket error");
         return 1;
     }

     struct sockaddr_in addr_server, addr_client;
     socklen_t addr_client_len = sizeof(addr_client);
     addr_server.sin_family = AF_INET;
     addr_server.sin_port = htons(PORT);
     addr_server.sin_addr.s_addr = INADDR_ANY;

     int opt=1;
    

     // bind socket;

     if(bind(server_fd,(struct sockaddr*)&addr_server,sizeof(addr_server)) == -1)
     {
         perror("bind error");
         return 1;
     }

     if(listen(server_fd,MAX_CLIENTS) ==-1){

        perror("listen error");
        return 1;
     }

     while(1)
     {
        // loop for accepting incoming connection request;

        int new_client = accept(server_fd,(struct sockaddr*) &addr_client, &addr_client_len);

        if(new_client == -1){
            perror("accept error");
            close(server_fd);
            return 1;
        }

        pid_t pid = fork();
        if(pid == 0 ){

            // child process will handle each new client.
            // child process does not need server_fd;
            close(server_fd);
            handle_client(new_client);

        }

        else if(pid > 0){

            //we are in parent process.
            //parent will keep on accepting new connection.
            close(new_client);
        }
        else{
            perror("fork error");
            return 1;
        }
     }
}

void handle_client(int client_socket){

    printf("Inside server handle_client\n");
    while(1){

        int menu_choice;
        printf("server side before recv 92\n");
        int rec_client = recv(client_socket, &menu_choice, sizeof(menu_choice),0);
        printf("serve side menu choice %d\n", menu_choice);

        switch(menu_choice){

            case 1:
            handle_customer(client_socket);
            break;

            case 2:
            handle_employee(client_socket);
            break;

            case 3:
            handle_manager(client_socket);
            break;

            case 4:
            handle_admin(client_socket);
            break;

            default:
            break;
        }
    }
}