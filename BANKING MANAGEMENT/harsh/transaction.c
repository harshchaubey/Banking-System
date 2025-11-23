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
#include "transactionHistory.h"
#include "transaction.h"

// NOTE: log_transaction must be updated in transactionHistory.h/c
// to have this signature:
// void log_transaction(int customer_id, int amount, char *type, int new_balance);


void deposit(int client_socket, int customer_id, int add_balance)
{
    int fd_read = open("customer_db.txt", O_RDWR);
    if (fd_read == -1) {
        perror("open customer_db.txt");
        send(client_socket, "Server file error.\n", strlen("Server file error.\n") + 1, 0);
        return;
    }

    struct customer temp_cust;

    while(read(fd_read, &temp_cust, sizeof(struct customer)) > 0)
    {
        if(customer_id == temp_cust.customer_id){
            // Found the record, now get the lock
            struct flock lk;
            lk.l_type = F_WRLCK;
            lk.l_whence = SEEK_SET; 
            lk.l_start = lseek(fd_read, 0, SEEK_CUR) - sizeof(struct customer);
            lk.l_len = sizeof(struct customer);

            if(fcntl(fd_read , F_SETLKW, &lk) == -1){
                perror("fcntl deposit lock");
                close(fd_read);
                return;
            }

           
            if (lseek(fd_read, lk.l_start, SEEK_SET) == -1) {
                 perror("lseek re-read"); close(fd_read); return;
            }
            if (read(fd_read, &temp_cust, sizeof(struct customer)) <= 0) {
                perror("read after lock"); close(fd_read); return;
            }
            

            int old_balance = temp_cust.balance;
            temp_cust.balance += add_balance;
            
            
            if(lseek(fd_read, lk.l_start, SEEK_SET) == -1){
                perror("lseek pre-write");
                return;
            }

            if(write(fd_read, &temp_cust, sizeof(struct customer)) <= 0){
                perror("write error");
                
            }

            lk.l_type = F_UNLCK;
            fcntl(fd_read, F_SETLK, &lk);

            
            log_transaction(customer_id, add_balance, "DEPOSIT", temp_cust.balance);

            char msg[256];
            sprintf(msg, "Balance updated. Old balance: %d. New balance: %d\n", old_balance, temp_cust.balance);
            
            
            send(client_socket, msg, strlen(msg) + 1, 0);
            close(fd_read);
            return;
        }
    }

    
    send(client_socket, "Could not find customer id in db.\n", strlen("Could not find customer id in db.\n") + 1, 0);
    close(fd_read);
}

void withdraw(int client_socket, int customer_id, int withdraw_amount)
{
    
    int fd_read = open("customer_db.txt", O_RDWR);
    if (fd_read == -1) {
        perror("open customer_db.txt");
        send(client_socket, "Server file error.\n", strlen("Server file error.\n") + 1, 0);
        return;
    }

    struct customer temp_cust;
    while(read(fd_read, &temp_cust, sizeof(struct customer)) > 0){
   
        if(customer_id == temp_cust.customer_id){
            struct flock lk;
            lk.l_type = F_WRLCK;
            lk.l_whence = SEEK_SET; 
            lk.l_start = lseek(fd_read, 0, SEEK_CUR) - sizeof(struct customer);
            lk.l_len = sizeof(struct customer);

            if(fcntl(fd_read, F_SETLKW, &lk ) == -1){
                perror("fcntl withdraw lock");
                close(fd_read);
                return;
            }

           
            if (lseek(fd_read, lk.l_start, SEEK_SET) == -1) {
                 perror("lseek re-read"); close(fd_read); return;
            }
            if (read(fd_read, &temp_cust, sizeof(struct customer)) <= 0) {
                perror("read after lock"); close(fd_read); return;
            }

            int old_balance = temp_cust.balance;
            if(old_balance < withdraw_amount){
                char msg[256];
                sprintf(msg, "Not enough balance. Current balance: %d\n", old_balance);
                
                send(client_socket, msg, strlen(msg) + 1, 0);
                
                lk.l_type = F_UNLCK;
                fcntl(fd_read, F_SETLK, &lk);
                close(fd_read);
                return;
            }

            temp_cust.balance = temp_cust.balance - withdraw_amount;

            if(lseek(fd_read, lk.l_start, SEEK_SET) == -1){
                perror("lseek pre-write");
                return;
            }

            if(write(fd_read, &temp_cust,sizeof(struct customer)) <= 0){
                perror("write error");
            }

            lk.l_type = F_UNLCK;
            fcntl(fd_read, F_SETLK, &lk);

            log_transaction(customer_id, withdraw_amount, "WITHDRAW", temp_cust.balance);

            char msg[256];
            sprintf(msg, "Balance updated. Old balance: %d. New balance: %d\n", old_balance, temp_cust.balance);
            
            send(client_socket, msg, strlen(msg) + 1, 0);
            close(fd_read);
            return;
        }
    }

    send(client_socket, "Could not find customer id in db.\n", strlen("Could not find customer id in db.\n") + 1, 0);
    close(fd_read);
}

void transfer_fund(int client_socket, int sender_customer_id, int receiver_customer_id, int transfer_amount)
{
    if (sender_customer_id == receiver_customer_id) {
        send(client_socket, "Cannot transfer to yourself.\n", strlen("Cannot transfer to yourself.\n") + 1, 0);
        return;
    }

    int fd_read = open("customer_db.txt", O_RDWR);
    if (fd_read == -1) {
        perror("open customer_db.txt");
        send(client_socket, "Server file error.\n", strlen("Server file error.\n") + 1, 0);
        return;
    }

    struct customer sender_data, receiver_data, temp;
    bool sender_found = false;
    bool receiver_found = false;
    off_t sender_offset = -1;
    off_t receiver_offset = -1;

    while(read(fd_read, &temp, sizeof(struct customer)) > 0)
    {
        if(temp.customer_id == sender_customer_id){
            sender_found = true;
            sender_offset = lseek(fd_read, 0, SEEK_CUR) - sizeof(struct customer);
        }
        if(temp.customer_id == receiver_customer_id)
        {
            receiver_found = true;
            receiver_offset = lseek(fd_read, 0, SEEK_CUR) - sizeof(struct customer);
        }
    }

    if (!sender_found || !receiver_found) {
        send(client_socket, "Sender or receiver not found.\n", strlen("Sender or receiver not found.\n") + 1, 0);
        close(fd_read);
        return;
    }

    
    struct flock lk1, lk2;
    off_t offset1, offset2;

    if (sender_offset < receiver_offset) {
        offset1 = sender_offset;
        offset2 = receiver_offset;
    } else {
        offset1 = receiver_offset;
        offset2 = sender_offset;
    }

    // Lock first record
    lk1.l_type = F_WRLCK; lk1.l_whence = SEEK_SET; lk1.l_start = offset1; lk1.l_len = sizeof(struct customer);
    if (fcntl(fd_read, F_SETLKW, &lk1) == -1) {
        perror("fcntl lock 1"); close(fd_read); return;
    }

    // Lock second record
    lk2.l_type = F_WRLCK; lk2.l_whence = SEEK_SET; lk2.l_start = offset2; lk2.l_len = sizeof(struct customer);
    if (fcntl(fd_read, F_SETLKW, &lk2) == -1) {
        perror("fcntl lock 2");
        lk1.l_type = F_UNLCK; fcntl(fd_read, F_SETLK, &lk1); // Unlock first
        close(fd_read); return;
    }

    // --- STAGE 3: Both locked. Re-read data, perform transaction. ---
    
    // Read sender data
    lseek(fd_read, sender_offset, SEEK_SET);
    read(fd_read, &sender_data, sizeof(struct customer));
    
    // Read receiver data
    lseek(fd_read, receiver_offset, SEEK_SET);
    read(fd_read, &receiver_data, sizeof(struct customer));

    if (sender_data.balance < transfer_amount) {
        char msg[256];
        sprintf(msg, "Not enough balance. Current balance: %d\n", sender_data.balance);
        send(client_socket, msg, strlen(msg) + 1, 0);
    } else {
        // Perform transaction
        sender_data.balance -= transfer_amount;
        receiver_data.balance += transfer_amount;

        // Write sender data
        lseek(fd_read, sender_offset, SEEK_SET);
        write(fd_read, &sender_data, sizeof(struct customer));

        // Write receiver data
        lseek(fd_read, receiver_offset, SEEK_SET);
        write(fd_read, &receiver_data, sizeof(struct customer));

        // Log transactions *after* write, *before* unlock
        char transaction_type[256];
        sprintf(transaction_type, "Transferred to %d", receiver_customer_id);
        log_transaction(sender_customer_id, transfer_amount, transaction_type, sender_data.balance);
        
        sprintf(transaction_type, "Received from %d", sender_customer_id);
        log_transaction(receiver_customer_id, transfer_amount, transaction_type, receiver_data.balance);

        send(client_socket, "Amount transferred successfully.\n", strlen("Amount transferred successfully.\n") + 1, 0);
    }

    // --- STAGE 4: Unlock both records ---
    lk1.l_type = F_UNLCK;
    fcntl(fd_read, F_SETLK, &lk1);
    
    lk2.l_type = F_UNLCK;
    fcntl(fd_read, F_SETLK, &lk2);

    close(fd_read);
}