#ifndef TRANSACTION_H
#define TRANSACTION_H
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



 void log_transaction(int customer_id, int amount, char *type, int new_balance);
 void deposit(int client_socket,int customer_id, int add_balance);
  void withdraw(int client_socket, int customer_id, int withdraw_amount);
   void transfer_fund(int client_socket, int sender_customer_id, int receiver_customer, int transfer_amount);
   




 #endif
