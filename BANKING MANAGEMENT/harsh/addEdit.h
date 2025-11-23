#ifndef ADDEDIT_H
#define ADDEDIT_H
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


void add_customer(int client_socket);
void edit_customer(int client_socket);
void edit_employee(int client_socket);
void add_employee(int client_socket);
void deactivate_customer(int client_socket, int customer_id);
void manage_user_role(int client_socket,int employee_id);


#endif



