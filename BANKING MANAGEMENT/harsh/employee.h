#ifndef EMPLOYEE_H
#define EMPLOYEE_H
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
#include <stdbool.h>




bool authenticate_employee(int client_socket, int user_id,  char *password);
void edit_credentials_employee(int client_socket, int user_id);
void employee_logout(int client_socket, int user_id);
bool handle_employee(int client_socket);
void add_customer(int client_socket);
void edit_customer(int client_socket);
                   void edit_credentials_employee(int client_socket, int userid_buffer);
void employee_logout(int client_socket, int userid_buffer);

#endif