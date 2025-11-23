#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>  // For open, O_CREAT, O_WRONLY
#include <unistd.h> // For close, write, read
#include <time.h>   // For time functions
#include "commonstruct.h"
#include <sys/socket.h>
#include "viewbalance.h"
#include "transactionHistory.h"


//int view_balance(int client_socket, int customer_id);


// function to get current date and timeas a string;
void get_current_time(char *buffer, size_t buffer_size)
{

    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);                                           // get the curent time;
    timeinfo = localtime(&rawtime);                           // convert to local time;
    strftime(buffer, buffer_size, "%Y-%m-%d %H:%M:%S", timeinfo); // format;
    
}

void log_transaction(int client_socket, int customer_id, int amount, char *transaction_type){

    //store transaction after taking advisory lock.
    //we're taking lock on byte equal to sizeof(struct transact) which will be written on the file.

    struct transact new_transaction;
    new_transaction.user_id = customer_id;
    char temp_buffer[128];
    int current_balance = view_balance(client_socket, customer_id);
    new_transaction.current_balance = current_balance; 
    sprintf(temp_buffer, "%s:%d", transaction_type, amount);
    strcpy(new_transaction.transaction, temp_buffer);
    get_current_time(new_transaction.date_time, sizeof(new_transaction.date_time));

    int fd_transact = open("transaction_db.text", O_RDWR);
    if(fd_transact ==-1){
        perror("opensystem call error");
        return ;
    }

    struct flock lk_tr;
    lk_tr.l_type = F_WRLCK;
    lk_tr.l_whence = SEEK_END;
    lk_tr.l_start = 0;
    lk_tr.l_len = sizeof(struct transact);
    if(fcntl(fd_transact, F_SETLKW, &lk_tr) == -1){
        perror("error fcntl");
        close(fd_transact);
        return ;

    }

    if(lseek(fd_transact, 0 , SEEK_END) == -1){

        perror("write error");
        return ;
    }

    if(write(fd_transact, &new_transaction, sizeof(struct transact)) == -1)
    { 
        perror("write error");
        close(fd_transact);
        return ;

    }

    lk_tr.l_type = F_UNLCK;

    if(fcntl(fd_transact, F_SETLK, &lk_tr) == -1){

        perror("error fcntl");
        close(fd_transact);
        return ;
    }

}

void show_transaction_history(int cust_id){

    // find cusstomer id in transaction_db and show his transaction.
    // there ca be multiple struct for single user so scan entire file.
    int fd_transact = open("transaction_db.ext", O_RDONLY);
    if(fd_transact == -1){
        perror("error open");
        return ;
    
    }

    struct transact temp_trans;

    while(read(fd_transact, &temp_trans, sizeof(temp_trans))){

        if(temp_trans.user_id == cust_id){
            // char buffer[512];

            printf("user id: %d%s %s %d", temp_trans.user_id, temp_trans.transaction, temp_trans.date_time, temp_trans.current_balance);
            //send(client_-socket, buffer, strlen(buffer), 0);

        }
    }
    close(fd_transact);
}
