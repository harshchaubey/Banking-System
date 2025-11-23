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
#include "credentials.h"
#include "commonstruct.h"
#include "transaction.h"
#include "loan.h"
#include "feedback.h"
#include "transactionHistory.h"
#include "viewbalance.h"
#include "customer.h"


bool handle_customer(int client_socket)
{
    while(1){
        // first authenticate then proceed;
        printf("Inside customer\n");
        char username_buffer[128];
        char password_buffer[128];
        int userid_buffer;
        int menu_option_recv;

       
        
        // 1. Receive Username
        int rec_client = recv(client_socket, username_buffer, sizeof(username_buffer), 0);
        
        if(rec_client <= 0){
            perror("recv username error");
            return false;
        }
        username_buffer[rec_client] = '\0'; 
        printf("handle customer 28 |%s| \n", username_buffer);

      
         rec_client = recv(client_socket, password_buffer, sizeof(password_buffer), 0);
         if(rec_client <= 0){
            perror("recv password error");
            return false;
         }
         password_buffer[rec_client] = '\0'; 
         printf("handle_customer 39 |%s|\n",  password_buffer);
           
         
         rec_client = recv(client_socket, &userid_buffer, sizeof(userid_buffer), 0);
         if(rec_client <= 0){
            perror("recv userid error");
            return false;
         }
         printf("customer_handle 50 |%d|\n", userid_buffer);

         if(authenticate_customer(client_socket, userid_buffer, password_buffer)){
            
            
            send(client_socket,"Authentication successful", strlen("Authentication successful") + 1, 0);

            while(1){

               
               // char menu_options[] = "...";
               // send(client_socket, menu_options, strlen(menu_options),0 );
               
               int client_rcv = recv(client_socket, &menu_option_recv, sizeof(menu_option_recv), 0);
               if (client_rcv <= 0) { perror("recv menu choice error"); return false; }

               switch(menu_option_recv){
                
                  case 1: // View Balance
                     view_balance(client_socket, userid_buffer);
                     break;

                  case 2: // Deposit
                  {
                     int amount;
                    
                     rec_client = recv(client_socket, &amount, sizeof(amount), 0);
                     if(rec_client <= 0){
                        perror("recv deposit amount error");
                        return false;
                     }
                     printf("deposit amt 83 customer: %d\n", amount);
                     deposit(client_socket, userid_buffer, amount);
                     break;
                  }
                  case 3: // Withdraw
                  {
                     int withdraw_amount;
                     rec_client = recv(client_socket, &withdraw_amount, sizeof(withdraw_amount), 0);
                     if(rec_client <= 0){
                        perror("recv withdraw error");
                        return false;
                     }
                     printf("customer withdraw: %d\n", withdraw_amount);
                     withdraw(client_socket, userid_buffer, withdraw_amount);
                     break;
                  }
                  case 4: // Transfer
                  {
                     
                     int receiver_customer_id;
                     rec_client = recv(client_socket, &receiver_customer_id, sizeof(receiver_customer_id), 0);
                     if(rec_client <= 0){
                        perror("recv receiver_id error");
                        return false;
                     }

                     int transfer_amount;
                     rec_client = recv(client_socket, &transfer_amount, sizeof(transfer_amount), 0);
                     if(rec_client <= 0){
                        perror("recv transfer_amount error");
                        return false;
                     }
                     transfer_fund(client_socket, userid_buffer, receiver_customer_id, transfer_amount);
                     break;
                  }
                  case 5: // Apply Loan
                  {
                     
                     int loan_amount;
                     rec_client = recv(client_socket, &loan_amount, sizeof(loan_amount), 0);
                     if(rec_client <= 0){
                        perror("recv loan_amount error");
                        return false;
                     }
                     apply_loan(client_socket, userid_buffer, loan_amount);
                     break;
                  }
                  case 6: // Change Password
                     // edit_credential_customer(userid_buffer);
                     break;

                  case 7: // Add Feedback
                  {
                     char feedback[1024];
                     
                     rec_client = recv(client_socket, feedback, sizeof(feedback), 0);
                     if(rec_client <= 0){
                        perror("recv feedback error");
                        return false;
                     }
                     // add_feedback(client_socket, userid_buffer, feedback);
                     break;
                  }
                  case 8: // View History
                     // show_transaction_history(userid_buffer);
                     break;

                  case 10: // Exit
                     
                     send(client_socket, "Existing..", strlen("Existing..") + 1, 0);
                     close(client_socket);
                     exit(0);
                  
                  default:
                     
                     send(client_socket, "Invalid response", strlen("Invalid response") + 1, 0);
                     break;
               }

               if (menu_option_recv == 9){ 
                  return true; 
               } 
            }
         }
         else{
            // Authenticate failed
            
            send(client_socket, "Authentication failed", strlen("Authentication failed") + 1, 0);
            return false;
         }
      }
      return true; 
}