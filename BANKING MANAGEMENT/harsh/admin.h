
#ifndef ADMIN_H
#define ADMIN_H
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



bool handle_admin(int client_socket);
void add_employee( int client_socket);
void edit_customer( int client_socket);
void edit_employee( int client_socket);
void manage_user_role( int client_socket,  int employee_id);
void edit_credentials_admin( int client_socket, int  user_id);
void admin_logout( int client_socket, int user_id);
bool authenticate_admin(int client_socket, int user_id, char* password);

#endif