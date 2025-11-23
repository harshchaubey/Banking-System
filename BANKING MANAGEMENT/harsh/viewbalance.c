#include <stdio.h>
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
#include "commonstruct.h"
#include "viewbalance.h"


int view_balance(int client_socket, int customer_id)
{
    int fd_read = open("customer_db.txt", O_RDONLY);
    if (fd_read == -1) {
        perror("open customer_db.txt");
        send(client_socket, "Server file error.\n", strlen("Server file error.\n") + 1, 0);
        return -1;
    }

    int current_balance = -1; // Use -1 to indicate not found
    struct customer temp_cust;
    
    while(read(fd_read, &temp_cust, sizeof(struct customer)) > 0)
    {
        if(customer_id == temp_cust.customer_id){
            struct flock lk;
            
            off_t record_start = lseek(fd_read, 0, SEEK_CUR) - sizeof(struct customer);

            lk.l_type = F_RDLCK;    // Read Lock
            lk.l_whence = SEEK_SET; 
            lk.l_start = record_start;
            lk.l_len = sizeof(struct customer);

            if(fcntl(fd_read, F_SETLKW, &lk) == -1){
                perror("fcntl read lock");
                close(fd_read);
                return -1;
            }

           
            lseek(fd_read, record_start, SEEK_SET);
            if (read(fd_read, &temp_cust, sizeof(struct customer)) <= 0) {
                perror("read after lock");
                lk.l_type = F_UNLCK;
                fcntl(fd_read, F_SETLK, &lk);
                close(fd_read);
                return -1;
            }

            current_balance = temp_cust.balance;

            lk.l_type = F_UNLCK;
            if(fcntl(fd_read, F_SETLK, &lk) == -1){
                perror("fcntl unlock");
                close(fd_read);
                return -1;
            }

            char msg[256];
            sprintf(msg, "Current balance: %d\n", current_balance);
            
            send(client_socket, msg, strlen(msg) + 1, 0);
            
            close(fd_read);
            return current_balance;
        }
    }

    send(client_socket, "Could not find customer.\n", strlen("Could not find customer.\n") + 1, 0);
    close(fd_read);
    return -1;
}