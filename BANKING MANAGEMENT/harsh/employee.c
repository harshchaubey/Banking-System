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
#include "employee.h"


bool handle_employee(int client_socket)
{
    while(1){
        printf("Inside employee\n");
        char username_buffer[1024];
        char password_buffer[1024];
        int userid_buffer;
        int menu_option_recv;
        int rec_client;

        // 1. Receive Username
        rec_client = recv(client_socket, username_buffer, sizeof(username_buffer), 0);
        if(rec_client <= 0) { perror("recv username error"); return false; }
       

        // 2. Receive Password
        rec_client = recv(client_socket, password_buffer, sizeof(password_buffer), 0);
        if (rec_client <= 0) { perror("recv password error"); return false; }
        
        printf("emp 49 %s\n", password_buffer);

        // 3. Receive User ID
        
        rec_client = recv(client_socket, &userid_buffer, sizeof(userid_buffer), 0);
        if (rec_client <= 0) { perror("recv userid error"); return false; }
        printf("emp 54 %d\n", userid_buffer);

        if(authenticate_employee(client_socket, userid_buffer, password_buffer))
        {
           printf("Inside emp auth 63\n");
            
            send(client_socket, "Authentication Successful", strlen("Authentication Successful") + 1, 0);

            while(1){
                
                
                
                int client_rcv = recv(client_socket, &menu_option_recv, sizeof(menu_option_recv), 0);
                if (client_rcv <= 0) { perror("recv menu choice error"); return false; }
                
                int customer_id; 

                switch(menu_option_recv){
                    case 1:
                        add_customer(client_socket);
                        break;
                    case 2:
                        edit_customer(client_socket);
                        break;
                    case 3: // Approve Loan
                    {
                        
                        rec_client = recv(client_socket, &customer_id, sizeof(customer_id), 0);
                        if (rec_client <= 0) {
                            perror("recv cust_id error");
                            return false;
                        }
                        
                        break;
                    }
                    case 4:
                        // view_assigned_loan_application(client_socket, userid_buffer);
                        break;
                    case 5:
                    {
                        
                        // rec_client = recv(client_socket, &customer_id, sizeof(customer_id), 0);
                        // show_transaction_history(client_socket, customer_id);
                        break;
                    }
                    case 6:
                        edit_credentials_employee(client_socket, userid_buffer);
                        break;
                    case 7: // Logout
                        employee_logout(client_socket, userid_buffer);
                        break;
                    case 8: // Exit
                        
                        send(client_socket, "Exiting..", strlen("Exiting..") + 1, 0);
                        close(client_socket);
                        exit(0);
                    default:
                       
                        send(client_socket, "Invalid response", strlen("Invalid response") + 1, 0);
                        break;
                }

                if(menu_option_recv == 7){
                    
                    break;
                }
            }
        } else {
            printf("auth failed emp 141\n");
           
            send(client_socket,"Authentication failed", strlen("Authentication failed") + 1, 0);
            return false; // Or break;
        }
    }
    return true; 
}