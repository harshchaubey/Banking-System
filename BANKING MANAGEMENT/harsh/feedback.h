#ifndef FEEDBACK_H
#define FEEDBACK_H
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h> 
#include <string.h>
#include <stdbool.h>


void add_feedback(int client_socket,int user_id,char* feedback);
void resolve_feedback(int client_socket,int user_id);

#endif

