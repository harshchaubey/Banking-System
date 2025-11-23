#ifndef TRANSACTION_HISTORY_H
#define TRANSACTION_HISTORY_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>  // For open, O_CREAT, O_WRONLY
#include <unistd.h> // For close, write, read
#include <time.h> 




int view_balance(int client_socket, int customer_id);
void get_current_time(char *buffer, size_t buffer_size);
void log_transaction(int client_socket, int customer_id, int amount, char *transaction_type);
void show_transaction_history(int cust_id);


#endif