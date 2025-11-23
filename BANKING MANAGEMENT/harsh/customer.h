#ifndef CUSTOMER_H
#define CUSTOMER_H

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

bool handle_customer(int client_socket);

bool authenticate_customer(int client_socket, int user_id,  char* pswd);
int  view_balance(int client_socket, int userid_buffer);
void deposit(int client_socket, int userid_buffer, int amount);
void withdraw(int client_socket, int userid_buffer, int withdraw_amount);
void transfer_fund(int client_socket, int userid_buffer, int receiver_customer_id,int transfer_amount);

#endif