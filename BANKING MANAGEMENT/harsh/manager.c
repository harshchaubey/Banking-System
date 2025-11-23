#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
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
#include "manager.h"


bool handle_manager(int client_socket){

    while(1){

        char username_buffer[128];
        char password_buffer[128];
        int userid_buffer;
        int menu_options_recv;

        int rec_client = recv(client_socket, username_buffer, sizeof(username_buffer), 0);
        if(rec_client <= 0) { perror("recv username error"); return false; }

       // Receive password
       rec_client = recv(client_socket, password_buffer, sizeof(password_buffer), 0);
       if(rec_client <= 0) { perror("recv password error"); return false; }

        rec_client = recv(client_socket, &userid_buffer, sizeof(userid_buffer), 0);
        if(rec_client <= 0) { perror("recv user_id error"); return false; }

        if(authenticate_manager(client_socket, userid_buffer, password_buffer)){

            send(client_socket, "Manager Authentication Successful", strlen("Manager Authentication Successful") + 1, 0);

            while(1){

                // char menu_option[] = "...";
                // send(client_socket,menu_options,strlen(menu_options),0);
                
                int client_rcv = recv(client_socket, &menu_options_recv, sizeof(menu_options_recv), 0);
                if (client_rcv <= 0) { perror("recv menu choice error"); return false; }

                int customer_id;

                switch(menu_options_recv){
                    case 1: // Deactivate Account
                    {
                        rec_client = recv(client_socket, &customer_id, sizeof(customer_id), 0);
                        if(rec_client <= 0) {
                            perror("recv customer_id error");
                            return false;
                        }
                        deactivate_customer(client_socket, customer_id);
                        break;
                    }
                    case 2:
                        // view_applied_loan_applications(client_socket);
                        break;

                    case 3:
                        // assign_loan(client_socket);
                        break;
                    case 4:
                        // This case is commented out, which is fine
                        break;

                    case 5:
                         edit_credentials_manager(client_socket, userid_buffer);
                         break;

                    case 6: // Logout
                         manager_logout(client_socket, userid_buffer);
                         break;

                    case 7: // Exit
                         send(client_socket, "Existing..", strlen("Existing..") + 1, 0);
                         close(client_socket);
                         exit(0); // Exit the child process
                    
                    default:
                         send(client_socket, "Invalid input", strlen("Invalid input") + 1, 0);
                         break;
                }
                
                if(menu_options_recv == 6)
                {
                    break;
                }  
            }
    
        } else {
            send(client_socket, "Authenticate failed", strlen("Authenticate failed") + 1, 0);
            return false; 
        }
    }         
}