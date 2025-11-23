#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h> // Included for clarity

int main() {
    char *server_address = "127.0.0.1";
    int clientsocket = socket(AF_INET, SOCK_STREAM, 0);

    if (clientsocket == -1) {
        perror("socket error");
        exit(1);
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    inet_pton(AF_INET, server_address, &serverAddr.sin_addr);

    if (connect(clientsocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("connection failed");
        close(clientsocket);
        exit(1);
    }

    printf("Connection to the server successful\n");

    while (1) {
        printf("Select Role(enter number corresponding desired role: )\n1)customer\n2)Employee\n3)Manager\n4)Admin\n");

        int menu_choice;
        char username[1024];
        char password[1024];
        int userid;
        scanf("%d", &menu_choice);

        send(clientsocket, &menu_choice, sizeof(menu_choice), 0);

        switch (menu_choice) {
        case 1: // Customer
        {
            printf("Enter username: \n");
            scanf("%1023s", username); 
            send(clientsocket, username, strlen(username) + 1, 0); 

            printf("Enter password: \n");
            scanf("%1023s", password); 
            send(clientsocket, password, strlen(password) + 1, 0); 
            printf("Enter user_id: \n");
            scanf("%d", &userid);
            send(clientsocket, &userid, sizeof(userid), 0);

            char auth_buffer[1024];
           
            int auth_rcv = recv(clientsocket, auth_buffer, sizeof(auth_buffer) - 1, 0); 
            if (auth_rcv > 0) {
                auth_buffer[auth_rcv] = '\0';
                printf("%s\n", auth_buffer);

                if (strcmp("Authentication successful", auth_buffer) == 0) {
                    while (1) {
                        printf("Select action to perform (select number corresponding to desired action): \n1) View Account Balance\n2) Deposit\n3)Withdraw\n4)Transfer Funds\n5)Apply for loan\n6)Change Password\n7)Adding Feedback\n8)View Transaction History\n9)Logout\n10)Exit\n");
                        
                        int user_choice;
                        scanf("%d", &user_choice);
                        send(clientsocket, &user_choice, sizeof(user_choice), 0);

                        if (user_choice == 9) { // Logout
                            char logout_buff[1024];
                            int rec_logout = recv(clientsocket, logout_buff, sizeof(logout_buff) - 1, 0);
                            if(rec_logout > 0) logout_buff[rec_logout] = '\0';
                            printf("%s\n", logout_buff);
                            break; 
                        }
                        if (user_choice == 10) { 
                            char exit_buff[256];
                            int rec_exit = recv(clientsocket, exit_buff, sizeof(exit_buff) - 1, 0);
                            if(rec_exit > 0) exit_buff[rec_exit] = '\0';
                            printf("%s\n", exit_buff);
                            close(clientsocket);
                            exit(0); 
                        }

                        switch (user_choice) {
                            case 1: // View Balance
                            {
                                char view_buff[1024];
                                int rec_view = recv(clientsocket, view_buff, sizeof(view_buff) - 1, 0);
                                if (rec_view > 0) {
                                    view_buff[rec_view] = '\0';
                                    printf("%s\n", view_buff);
                                }
                                break;
                            }
                            case 2: // Deposit
                            {
                                printf("Enter amount you want to deposit: \n");
                                int deposit;
                                char deposit_buf[1024];
                                scanf("%d", &deposit);
                                send(clientsocket, &deposit, sizeof(deposit), 0);
                                int rec_deposit = recv(clientsocket, deposit_buf, sizeof(deposit_buf) - 1, 0);
                                if (rec_deposit > 0) {
                                    deposit_buf[rec_deposit] = '\0';
                                    printf("%s \n", deposit_buf);
                                }
                                break;
                            }
                            case 3: // Withdraw
                            {
                                printf("Enter amount to withdraw: \n");
                                int withdraw;
                                char withdraw_buff[1024];
                                scanf("%d", &withdraw);
                                send(clientsocket, &withdraw, sizeof(withdraw), 0);
                                int rec_withdraw = recv(clientsocket, withdraw_buff, sizeof(withdraw_buff) - 1, 0);
                                if (rec_withdraw > 0) {
                                    withdraw_buff[rec_withdraw] = '\0';
                                    printf("%s\n", withdraw_buff);
                                }
                                break;
                            }
                            case 4: // Transfer Funds
                            {
                                printf("Enter receiver's id: \n");
                                int rec_id;
                                scanf("%d", &rec_id);
                                send(clientsocket, &rec_id, sizeof(rec_id), 0);
                                printf("Enter amount to transfer: \n");
                                int transfer_amount;
                                scanf("%d", &transfer_amount);
                                send(clientsocket, &transfer_amount, sizeof(transfer_amount), 0);
                                char transfer_buff[1024];
                               
                                int rec_tr = recv(clientsocket, transfer_buff, sizeof(transfer_buff) - 1, 0);
                                if (rec_tr > 0) {
                                    transfer_buff[rec_tr] = '\0';
                                    printf("%s\n", transfer_buff);
                                }
                                break;
                            }
                            case 5: // Apply for loan
                            {
                                printf("Enter loan amount: \n");
                                int loan_amount;
                                char loan_buff[1024];
                                scanf("%d", &loan_amount);
                                send(clientsocket, &loan_amount, sizeof(loan_amount), 0);
                                int rec_loan = recv(clientsocket, loan_buff, sizeof(loan_buff) - 1, 0);
                                if (rec_loan > 0) {
                                    loan_buff[rec_loan] = '\0';
                                    printf("%s\n", loan_buff);
                                }
                                break;
                            }
                            case 6: 
                                //edit_credentials_customer(userid);
                                break;
                            case 7: // Add Feedback
                            {
                                printf("Enter feedback: \n");
                                char feed[256];
                                scanf("%255s", feed); 
                                send(clientsocket, feed, strlen(feed) + 1, 0); 
                                char feed_buff[1024];
                                int rec_feed = recv(clientsocket, feed_buff, sizeof(feed_buff) - 1, 0);
                                if (rec_feed > 0) {
                                    feed_buff[rec_feed] = '\0';
                                    printf("%s\n", feed_buff);
                                }
                                break;
                            }
                            case 8: // View Transaction History 
                                //show_transaction_history(userid);
                                break;
                            default:
                                printf("Invalid response\n");
                                break;
                        } 
                    } 
                } 
            } else {
                printf("Failed to receive auth response from server.\n");
            }
            break;
        } 
        case 2: // Employee
        {
            printf("Enter username: \n");
            char username_buffer[128];
            char password_buffer[128];
            scanf("%127s", username_buffer); 
            send(clientsocket, username_buffer, strlen(username_buffer) + 1, 0); 

            printf("Enter password: \n");
            scanf("%127s", password_buffer); 
            send(clientsocket, password_buffer, strlen(password_buffer) + 1, 0); 
            
            printf("Enter user_id: \n");
            int userid_buffer;
            scanf("%d", &userid_buffer);
            send(clientsocket, &userid_buffer, sizeof(userid_buffer), 0);
            
            char auth[1000];
            int rcv_auth = recv(clientsocket, auth, sizeof(auth) - 1, 0);
            if (rcv_auth > 0) {
                auth[rcv_auth] = '\0';
                printf("Server: %s\n", auth);

                if (strcmp(auth, "Authentication Successful") == 0) {
                    printf("Authentication Successful.\n");
                    while (1) {
                        printf("Select action to perform:\n1)Add new customer\n2)Modify customer detail\n3)Apply for loan\n4)View Assinged Loan Application\n5)View Customer Transaction(passbook)\n6)Change Password\n7)Logout\n8)Exit\n");
                        int menu;
                        scanf("%d", &menu);
                        send(clientsocket, &menu, sizeof(menu), 0);

                        if (menu == 7) { // Logout
                            char buffer[1024];
                            int rec_ = recv(clientsocket, buffer, sizeof(buffer) - 1, 0);
                            if(rec_ > 0) buffer[rec_] = '\0';
                            printf("%s\n", buffer);
                            break; 
                        }
                        if (menu == 8) { 
                            char buffer_[1024];
                            int rec1 = recv(clientsocket, buffer_, sizeof(buffer_) - 1, 0);
                            if(rec1 > 0) buffer_[rec1] = '\0';
                            printf("%s\n", buffer_);
                            close(clientsocket);
                            exit(0);
                        }

                        switch (menu) {
                            case 1: // Add new customer
                            {
                                printf("Enter user name: \n");
                                char user_name[100];
                                scanf("%99s", user_name);
                                send(clientsocket, user_name, strlen(user_name) + 1, 0); 

                                printf("Enter cusomerID: \n");
                                int id;
                                scanf("%d", &id);
                                send(clientsocket, &id, sizeof(id), 0);

                                printf("Enter balance: \n");
                                int balance;
                                scanf("%d", &balance);
                                send(clientsocket, &balance, sizeof(balance), 0);

                                printf("Enter Account number: \n");
                                int accon;
                                scanf("%d", &accon);
                                send(clientsocket, &accon, sizeof(accon), 0);

                                printf("Enter password for new customer: \n");
                                char password[100];
                                scanf("%99s", password); 
                                send(clientsocket, password, strlen(password) + 1, 0); 

                                printf("Enter address for new customer: \n");
                                char address[100];
                                scanf("%99s", address); 
                                send(clientsocket, address, strlen(address) + 1, 0); 

                                int a;
                                recv(clientsocket, &a, sizeof(a), 0);
                                if (a == 0) {
                                    printf("Customer id or account number already exists.\n");
                                } else {
                                    printf("Customer added successfully.\n");
                                }
                                break;
                            }
                            case 2: // Modify customer
                            {
                                printf("Enter cutomer id that you want to modify: \n");
                                int id_;
                                scanf("%d", &id_);
                                send(clientsocket, &id_, sizeof(id_), 0);

                                printf("Enter key to update the following characteristics: \n1 for name, \n2 for contact, \n3 for address\n");
                                int choice;
                                scanf("%d", &choice);
                                send(clientsocket, &choice, sizeof(choice), 0);
                                
                                switch (choice) {
                                    case 1:
                                        printf("Enter new customer name: \n");
                                        char name[256];
                                        scanf("%255s", name);
                                        send(clientsocket, name, strlen(name) + 1, 0); 
                                        break;
                                    case 2:
                                        printf("enter new contact: \n");
                                        int contact;
                                        scanf("%d", &contact);
                                        send(clientsocket, &contact, sizeof(contact), 0);
                                        break;
                                    case 3:
                                        printf("enter new address for constomer: \n");
                                        char addresss[100]; 
                                        scanf("%99s", addresss); 
                                        send(clientsocket, addresss, strlen(addresss) + 1, 0); 
                                        break;
                                    default:
                                        printf("Invalid input\n");
                                        break;
                                }

                                char s[200];
                                int rec_s = recv(clientsocket, s, sizeof(s) - 1, 0);
                                if (rec_s > 0) {
                                    s[rec_s] = '\0';
                                    if (strcmp(s, "record updated.") == 0) {
                                        printf("record updated.\n");
                                    } else {
                                        printf("user not updated.\n");
                                    }
                                }
                                break;
                            }
                            case 3: // Loan approve
                            {
                                printf("Enter customer id whose loan you want to approve\n");
                                int user;
                                scanf("%d", &user);
                                send(clientsocket, &user, sizeof(user), 0);

                                char buffer[1024];
                                int rec = recv(clientsocket, buffer, sizeof(buffer) - 1, 0);
                                if (rec > 0) {
                                    buffer[rec] = '\0';
                                    printf("%s\n", buffer);
                                }
                                break;
                            }
                            case 4: // View assigned loan
                                // ...
                                break;
                            case 5: // View transaction history
                                printf("Enter customer id: \n");
                                int c_id;
                                scanf("%d", &c_id);
                                send(clientsocket, &c_id, sizeof(c_id), 0);
                                // ...
                                break;
                            case 6: // Change password
                            {
                                printf("Enter new password: \n");
                                char buff[256];
                                scanf("%255s", buff); // FIX
                                send(clientsocket, buff, strlen(buff) + 1, 0); // FIX
                                
                                char s_[200];
                                // FIX: Receive into s_, not buff. Use sizeof(s_).
                                int rcv = recv(clientsocket, s_, sizeof(s_) - 1, 0); 
                                if (rcv > 0) {
                                    s_[rcv] = '\0';
                                    printf("%s\n", s_);
                                }
                                break;
                            }
                            default:
                                break;
                        } 
                    } 
                } 
            } else {
                printf("Failed to receive auth response from server.\n");
            }
            break;
        } 

        case 3: // Manager
        {
            printf("Enter username: \n");
            scanf("%1023s", username); 
            send(clientsocket, username, strlen(username) + 1, 0); 

            printf("Enter password: \n");
            scanf("%1023s", password);
            send(clientsocket, password, strlen(password) + 1, 0); 
            
            printf("Enter user_id: \n");
            scanf("%d", &userid);
            send(clientsocket, &userid, sizeof(userid), 0);
            
            char auth_buff[1024];
            int auth_rcve = recv(clientsocket, auth_buff, sizeof(auth_buff) - 1, 0); 
            
            if (auth_rcve > 0) {
                auth_buff[auth_rcve] = '\0';
                printf("Server: %s\n", auth_buff);

                if (strcmp(auth_buff, "Manager Authentication Successful") == 0) { 
                    printf("Authentication successful.\n");
                    while (1) {
                        printf("select action to perform:\n1)Deactivate account\n2)Show all applied loan application\n3)Assign loan application to employee\n4)Review Feedback\n5)change password\n6)logout\n7)exit\n");
                        int choice;
                        scanf("%d", &choice);
                        send(clientsocket, &choice, sizeof(choice), 0);

                        if(choice == 6) { // Logout
                            char logout_manager_buff[1024];
                            int rec_logout = recv(clientsocket, logout_manager_buff, sizeof(logout_manager_buff) - 1, 0);
                            if(rec_logout > 0) logout_manager_buff[rec_logout] = '\0';
                            printf("%s\n", logout_manager_buff);
                            break; // Exit manager loop
                        }
                        if(choice == 7) { 
                            // Server should send something before exit, or just close
                            close(clientsocket);
                            exit(0);
                        }

                        switch (choice) {
                            case 1: // Deactivate
                                printf("Enter customer id that you want to deactivate: \n");
                                int cust_id;
                                scanf("%d", &cust_id);
                                send(clientsocket, &cust_id, sizeof(cust_id), 0);
                                break;
                            case 2: // Show all loans
                               
                                break;
                            case 3: // Assign loan
                                printf("Employee id to whom you want to assign loan application: \n");
                                int emp_id_loan;
                                scanf("%d", &emp_id_loan);
                                send(clientsocket, &emp_id_loan, sizeof(emp_id_loan), 0);
                                printf("Enter customer id whose loan you want to assign: \n");
                                int cust_id_loan;
                                scanf("%d", &cust_id_loan);
                                send(clientsocket, &cust_id_loan, sizeof(cust_id_loan), 0);
                                
                                break;
                            case 4: // Review feedback
                                printf("Enter customer id whose feedback you want to resolve.\n");
                                int cust_id_feed;
                                scanf("%d", &cust_id_feed);
                                send(clientsocket, &cust_id_feed, sizeof(cust_id_feed), 0);
                                // ... recv response
                                break;
                            case 5: // Change password
                                printf("Enter new password: \n");
                                char new_pass[256];
                                scanf("%255s", new_pass); // FIX
                                send(clientsocket, new_pass, strlen(new_pass) + 1, 0); // FIX
                                char pass_buff[1024];
                                int rec_new_pass = recv(clientsocket, pass_buff, sizeof(pass_buff) - 1, 0);
                                if(rec_new_pass > 0) {
                                    pass_buff[rec_new_pass] = '\0';
                                    printf("%s\n", pass_buff);
                                }
                                break;
                            default:
                                char rec_def_buff[1024];
                                int rec_def = recv(clientsocket, rec_def_buff, sizeof(rec_def_buff) - 1, 0);
                                if(rec_def > 0) {
                                    rec_def_buff[rec_def] = '\0';
                                    printf("%s\n", rec_def_buff);
                                }
                                break;
                        }
                    } 
                } 
            } else {
                 printf("Failed to receive auth response from server.\n");
            }
            break;
        } 

        case 4: // Admin
        {
            printf("Enter username: \n");
            char name[256];
            scanf("%255s", name); 
            send(clientsocket, name, strlen(name) + 1, 0); 

            printf("Enter password: \n");
            char pass[256];
            scanf("%255s", pass); 
            send(clientsocket, pass, strlen(pass) + 1, 0); 

            printf("Enter user_id: \n");
            int id;
            scanf("%d", &id);
            send(clientsocket, &id, sizeof(id), 0);

            char auth_buf[1024];
            int rec_auth_buf = recv(clientsocket, auth_buf, sizeof(auth_buf) - 1, 0);
            
            if (rec_auth_buf == -1) {
                perror("recv error admin 510");
                return 1;
            }

            auth_buf[rec_auth_buf] = '\0';
            printf("Server: %s\n", auth_buf);

            if (strcmp("admin authenticated", auth_buf) == 0) {
                printf("Authenticated successful\n");
                while (1) {
                    printf("Select action to perform:\n1)Add New Bank Employee\n2)Modify Customer\n3)Modify Employee\n4)Manage User Roles\n5)Change password\n6)logout\n7)exit\n");
                    int choice;
                    scanf("%d", &choice);
                    printf("selected choice: %d\n", choice); 
                    send(clientsocket, &choice, sizeof(choice), 0);

                    if (choice == 6) { 
                        char s4[200];
                        int rcv7 = recv(clientsocket, s4, sizeof(s4) - 1, 0);
                        if(rcv7 > 0) s4[rcv7] = '\0';
                        printf("%s\n", s4);
                        break; 
                    }
                    if (choice == 7) { 
                        char s8[200];
                        int rcv8 = recv(clientsocket, s8, sizeof(s8) - 1, 0);
                        if(rcv8 > 0) s8[rcv8] = '\0';
                        printf("%s\n", s8);
                        close(clientsocket);
                        exit(0);
                    }

                    switch (choice) {
                        case 1: 
                        {
                            printf("Enter user user name: \n");
                            char emp_name[256];
                            scanf("%255s", emp_name); 
                            send(clientsocket, emp_name, strlen(emp_name) + 1, 0); 

                            printf("Enter password for new employee: \n");
                            char emp_pass[256];
                            scanf("%255s", emp_pass); 
                            send(clientsocket, emp_pass, strlen(emp_pass) + 1, 0); 

                            printf("enter employee id: \n");
                            int emp_id;
                            scanf("%d", &emp_id);
                            send(clientsocket, &emp_id, sizeof(emp_id), 0);

                            char buff[256];
                            int rcv = recv(clientsocket, buff, sizeof(buff) - 1, 0);
                            if (rcv > 0) {
                                buff[rcv] = '\0';
                                if (strcmp(buff, "employee already exists.\n") == 0) {
                                    printf("Employee already exists. \n");
                                } else {
                                    printf("Employee added.\n");
                                }
                            }
                            break;
                        }
                        case 2: // Modify Customer
                        {
                            printf("Enter customer id that you want to modify: \n");
                            int id2;
                            scanf("%d", &id2);
                            send(clientsocket, &id2, sizeof(id2), 0);
                            printf("Enter key to update:\n1 for name \n2 for contact \n3 for address \n");
                            int mod_choice;
                            scanf("%d", &mod_choice);
                            send(clientsocket, &mod_choice, sizeof(mod_choice), 0);
                            
                            switch (mod_choice) {
                                case 1:
                                    printf("Enter new customer name: \n");
                                    char mod_name[256];
                                    scanf("%255s", mod_name); 
                                    send(clientsocket, mod_name, strlen(mod_name) + 1, 0); 
                                    break;
                                case 2:
                                    printf("Enter new contact: \n");
                                    int contact;
                                    scanf("%d", &contact);
                                    send(clientsocket, &contact, sizeof(contact), 0);
                                    break;
                                case 3:
                                    printf("Enter new address for customer: \n");
                                    char address[100];
                                    scanf("%99s", address); 
                                    send(clientsocket, address, strlen(address) + 1, 0); 
                                    break;
                                default:
                                    printf("Invalid input\n");
                                    break;
                            }

                            char s[200];
                            int rcv2 = recv(clientsocket, s, sizeof(s) - 1, 0);
                            if(rcv2 > 0) {
                                s[rcv2] = '\0';
                                if (strcmp(s, "record updated. \n") == 0) {
                                    printf("record updated.\n");
                                } else {
                                    printf("user not updated.\n");
                                }
                            }
                            break;
                        }
                        case 3: // Modify Employee
                        {
                            printf("Enter employee id that you want to modify:\n");
                            int id3;
                            scanf("%d", &id3);
                            send(clientsocket, &id3, sizeof(id3), 0);

                            printf("Enter new employee name: \n");
                            char buff2[256];
                            scanf("%255s", buff2); 
                            send(clientsocket, buff2, strlen(buff2) + 1, 0); 
                            break;
                        }
                        case 4: // Manage User Roles
                        {
                            printf("Enter employee id that you want to make manager, \n");
                            int id5;
                            scanf("%d", &id5);
                            send(clientsocket, &id5, sizeof(id5), 0);

                            char s2[200];
                            int rcv5 = recv(clientsocket, s2, sizeof(s2) - 1, 0);
                            if(rcv5 > 0) {
                                s2[rcv5] = '\0';
                                printf("%s\n", s2);
                            }
                            break;
                        }
                        case 5: // Change Password
                        {
                            printf("Enter new password:\n");
                            char buff3[256];
                            scanf("%255s", buff3); 
                            send(clientsocket, buff3, strlen(buff3) + 1, 0); 
                            char s3[200];
                            int rcv = recv(clientsocket, s3, sizeof(s3) - 1, 0);
                            if(rcv > 0) {
                                s3[rcv] = '\0';
                                printf("%s\n", s3);
                            }
                            break;
                        }
                        default:
                            printf("enter valid argument \n");
                            break;
                    } 
                } 
            } 
            else {
                printf("%s\n", auth_buf);
            }
            break;
        }

        default:
            printf("Invalid role selection.\n");
            break;
        } 
    } 

    close(clientsocket);
    return 0;
}