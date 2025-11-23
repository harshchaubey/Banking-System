
#ifndef CREDENTIALS_H
#define CREDENTIALS_H


// other includes and declarations
#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>
#include <fcntl.h>      
#include <unistd.h>     
#include <sys/stat.h> 
#include <sys/socket.h>
#include <stdbool.h>
#include <openssl/evp.h>


void edit_credentials_employee(int client_socket,int user_id);
void edit_credentials_customer(int client_socket,int user_id);
void password_hash_to_hex(const unsigned char* hashed_pswd,char* hex_pswd);
void password_hash_to_hex(const unsigned char* hashed_pswd,char* hex_pswd);
void hash_password(const char* pswd, unsigned char* hashed_pswd) ;
void trim_leading_spaces(char *str);
void trim_trailing_spaces(char *str) ;
void edit_credentials_manager(int client_socket,int user_id);
void edit_credentials_admin(int client_socket,int user_id);
bool authenticate_customer(int client_socket,int user_id, char *pswd);
bool authenticate_employee(int client_socket,int user_id,char* pswd);
bool authenticate_manager(int client_socket,int user_id,char* pswd);
bool authenticate_admin(int client_socket,int user_id,char* pswd);
void customer_logout(int client_socket,int user_id);
void employee_logout(int client_socket,int user_id);
void manager_logout(int client_socket,int user_id);
void admin_logout(int client_socket,int user_id);


#endif






