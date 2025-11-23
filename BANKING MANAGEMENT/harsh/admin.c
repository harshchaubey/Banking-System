#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<stdbool.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<errno.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/socket.h>
#include "commonstruct.h"
#include "transaction.h"
#include "credentials.h"
#include "loan.h"
#include "feedback.h"
#include "transactionHistory.h"
#include "viewbalance.h"
#include "addEdit.h"
#include "admin.h"


//void edit_credentials_admin(int client_socket, int user_id);
//void admin_logout(int client_socket, int user_id);


//bool authenticate_admin(int client_socket, int user_id,  char *password);

bool handle_admin(int client_socket)
{
    while(1)
    {
        char username_buffer[128];
        char password_buffer[128];
        int userid_buffer;
        int menu_options_recv;

        // 1. Receive Username (as string)
        int rec_client = recv(client_socket, username_buffer, sizeof(username_buffer), 0);
        if(rec_client <= 0) { perror("Client disconnected"); return false; }
        username_buffer[rec_client] = '\0';

        // 2. Receive Password (as string)
        rec_client = recv(client_socket, password_buffer, sizeof(password_buffer), 0);
        if(rec_client <= 0) { perror("Client disconnected"); return false; }
        password_buffer[rec_client] = '\0';

        // 3. Receive User ID (as binary int)
        rec_client = recv(client_socket, &userid_buffer, sizeof(userid_buffer), 0); 
        if(rec_client <= 0) { perror("Client disconnected"); return false; }

        printf("[Server-Debug] Received user_id: %d\n", userid_buffer);
        printf("[Server-Debug] Received password: '%s'\n", password_buffer);

        if(authenticate_admin(client_socket, userid_buffer, password_buffer))
        {
            send(client_socket, "admin authenticated", strlen("admin authenticated") + 1, 0);

            while(1)
            {
                // char menu_options[] = "...";
                // send(client_socket, menu_options, strlen(menu_options), 0);

                int client_rcv = recv(client_socket, &menu_options_recv, sizeof(menu_options_recv), 0);
                if(client_rcv <= 0) { perror("Client disconnected"); return false; }

                switch(menu_options_recv)
                {
                    case 1:
                        add_employee(client_socket);
                        break;
                    case 2:
                        edit_customer(client_socket);
                        break;
                    case 3:
                        edit_employee(client_socket);
                        break;
                    case 4:
                    { 
                        // send(client_socket,"Enter employee id that you want to make manager.", ...);
                        
                        int employee_id;
                        int rcv_emp_id = recv(client_socket, &employee_id, sizeof(employee_id), 0);
                        if(rcv_emp_id <= 0) { perror("Client disconnected"); return false; }

                        manage_user_role(client_socket, employee_id);
                        break;
                    }
                    case 5:
                        edit_credentials_admin(client_socket, userid_buffer);
                        break;
                    case 6:
                        admin_logout(client_socket, userid_buffer);
                        break;
                    case 7:
                        send(client_socket,"Exiting..", strlen("Exiting..") + 1, 0);
                        close(client_socket);
                        exit(0);
                    default:
                        send(client_socket,"Invalid input...\n", strlen("Invalid input...\n") + 1, 0);
                        break;
                }
                
                if(menu_options_recv == 6 || menu_options_recv == 7)
                {
                    // If choice was 6 (logout), this will break and go to the login while(1) loop
                    // If choice was 7, the process exited anyway.
                    break;
                }
            }
        }
        else
        {
            send(client_socket,"Authentication failed", strlen("Authentication failed") + 1, 0);
            return false; 
        }
    }
}


 // ADMIN_H