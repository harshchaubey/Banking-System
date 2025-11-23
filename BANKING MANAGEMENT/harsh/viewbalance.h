
#ifndef VIEWBALANCE_H
#define VIEWBALANCE_H
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<errno.h>
#include <fcntl.h>



int view_balance(int client_socket, int customer_id);

#endif
