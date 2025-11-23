#ifndef LOAN_H
#define LOAN_H
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/socket.h>


void view_applied_loan_applications(int client_socket);
void assign_loan(int client_socket);
void approve_loan(int client_socket, int employee_id,int customer_id);
void apply_loan(int client_socket, int cust_id, int loan_amount);
void view_applied_loan_applications(int client_socket);





#endif