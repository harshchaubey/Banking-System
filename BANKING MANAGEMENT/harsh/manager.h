#ifndef MANAGER_H
#define MANAGER_H
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


void edit_credentials_manager(int client_socket, int user_id);

bool authenticate_manager(int client_socket, int user_id,  char *password);

bool handle_manager(int client_socket);
void deactivate_customer(int client_socket, int customer_id);
void deactivate_employee(int employee_id);
void deactivate_manager(int client_socket,int manager_id);
  void view_applied_loan_applications(int client_socket);
  void assign_loan(int client_socket);
  void edit_credentials_manager(int client_socket,int userid_buffer);

#endif