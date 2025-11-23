#include <stdio.h>
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
#include "commonstruct.h"
#include "loan.h"





//void view_applied_loan_applications(int client_socket);
//void assign_loan(int client_socket);



void assign_loan(int client_socket)
{
    // this funcction will be used by manager to assign lon;
    int employee_id;
    char emp_id_buff[1024];
    //send(client_socket,"Enter employee id to whom you want to assign loan application: ",strlen("Enter employee id to whom you want to assign loan appplication: "),0);
    int rec_emp = recv(client_socket, &emp_id_buff, sizeof(emp_id_buff), 0);
    employee_id = atoi(emp_id_buff);

    if(rec_emp <=0){

        perror("recv error");
        return ;
    }

    //send(client_socket,"Enter customer id whose loan you want to assign: ",strlen("Enter customer id whose loan you want to assign: "),0);
    int cust_id;
    char cust_id_buff[1024];
    int rec_cust = recv(client_socket, &cust_id_buff, sizeof(cust_id_buff), 0);
    cust_id = atoi(cust_id_buff);

    if(rec_cust <=0){

        perror("rev error");
        return ;
    }

   // find emp in loan_db.text and customer in cudtomer_db.text and update loan related entries.
   int fd_loan = open("loan_db.txt", O_RDWR);
   if(fd_loan == -1){
    perror("open error");
    return;
   }

   struct loan temp_loan;
   while(read(fd_loan, & temp_loan, sizeof(struct loan)) > 0){

    if(temp_loan.customer_id == cust_id){

        // assign loan_id to the employee;

        temp_loan.emp_id = employee_id;
        
        // write to db;
        struct flock lk;
        lk.l_type = F_WRLCK;
        lk.l_start = lseek(fd_loan, 0, SEEK_CUR) - sizeof(struct loan);
        lk.l_len = sizeof(temp_loan);
        lk.l_whence = SEEK_SET;

        if(fcntl(fd_loan, F_SETLKW, &lk) == -1 ){

            perror("Locking error in feedback resolve fun");
            close(fd_loan);
            return ;
            
        }

          if (lseek(fd_loan, -sizeof(temp_loan), SEEK_CUR) == -1)
            {
                perror("lseek error");
                close(fd_loan);
                return;
            }

         if(write(fd_loan, &temp_loan, sizeof(temp_loan))<=0){

            perror("write error in assign loan");
            close(fd_loan);
            return;
         }   

         lk.l_type = F_UNLCK;
         if(fcntl(fd_loan, F_SETLK, &lk) == -1){

            perror("unlock error");
            close(fd_loan);
            return ;
         }
          char msg[256];
          sprintf(msg, "loan application of customer_id %d , assigned to emp-id: %d", cust_id,employee_id);
          send(client_socket, msg, strlen(msg), 0);
          break;
    }
   }
    close(fd_loan);

   // close(client_socket);

}

void approve_loan(int client_socket, int employee_id,int customer_id)
{

    // this function is used by employee to approve or decline loan proposal.
    // in customer db make update and in loan db update status.

    int fd_emp = open("employee_db.txt",O_RDWR);
    if(fd_emp==-1){
        perror("open error");
        return;
    }

    int fd_loan = open("loan_db.txt", O_RDWR);
    int fd_cust = open("customer_db.txt", O_RDWR);

    if(fd_cust == -1 || fd_loan ==-1){

        perror("open error");
        return;
    }
     struct customer temp_cust;
     struct loan temp_loan;
     struct flock lkl;

     // update in customer db.
     
     // Loop through all records in the customer DB
     while(read(fd_cust, &temp_cust, sizeof(struct customer)) > 0)
     {
         if(temp_cust.customer_id == customer_id && temp_cust.need_loan == true) 
         {
            lkl.l_type = F_WRLCK;
            lkl.l_whence = SEEK_CUR;
            lkl.l_start = lseek(fd_cust, 0, SEEK_CUR) - sizeof(struct customer);
            lkl.l_len = sizeof(struct customer);
            if(fcntl(fd_cust, F_SETLKW, &lkl) == -1)
            {
                perror("fcntl error");
                close(fd_cust);
                close(fd_loan);
                return ;

            }
            temp_cust.loan_approved = true;
            char msg[256];
            sprintf(msg, "loan of customer_id: %d, loan amount: %d  approved by emp_id: %d", customer_id, temp_cust.loan_amount, employee_id);
            send(client_socket, msg, strlen(msg), 0);
            break;
         }   
     }
     // before unlocking write to customer db.
     if(lseek(fd_cust, -sizeof(struct customer), SEEK_CUR) == -1)
     {
        perror("lseek error");
        close(fd_cust);
        close(fd_loan);
        return;
     }
     if(write(fd_cust, &temp_cust, sizeof(struct customer))<=0){

        perror("write error");
        close(fd_cust);
        close(fd_loan);
        return;
     }

     lkl.l_type = F_UNLCK;

     if(fcntl(fd_cust, F_SETLK,&lkl) == -1){
        perror("fcntl error");
        close(fd_cust);
        close(fd_loan);
        return;
     }

     // update in loan db.
     // match both emp_id and customer_id in loan db;

     while(read(fd_loan, &temp_loan, sizeof(struct loan))>0)
     {
        if(temp_loan.emp_id == employee_id && temp_loan.customer_id == customer_id)
        {
            // found record in loan db
            strcpy(temp_loan.status, "approved");
            if(lseek(fd_loan, -sizeof( struct loan), SEEK_CUR) == -1){
                perror("lseek error");
                close(fd_cust);
                close(fd_loan);
                return;
            }

            if(write(fd_loan ,&temp_loan, sizeof(struct loan)) <=0 ){
                  perror("write error");
                  close(fd_cust);
                  close(fd_loan);
                  return;
            }
            break;
        }
     }

     close(fd_cust);
     close(fd_loan);

}

void apply_loan(int client_socket, int cust_id, int loan_amount)
{
 // this function will we used by cutomer to apply for loan .
 // first find customer in customer_db and then update required fiels.
 // in loan_db also add new entry for customer, keep other entries empty.
 
 int fd_cust = open("customer_db.txt", O_RDWR);
 int fd_loan = open("loan_db.txt", O_RDWR);

 if(fd_cust == -1 || fd_loan == -1){

    perror("open error");
    return;
 }

 struct customer temp_cust;
 struct loan temp_loan;
 temp_loan.customer_id = cust_id;
 temp_loan.emp_id = 0;
 temp_loan.loan_amount = loan_amount;
 strcpy(temp_loan.status, "applied");
 /// now write this entry into loan_db;
 struct flock lkl;
 lkl.l_type = F_WRLCK;
 lkl.l_start = 0;
 lkl.l_whence = SEEK_END;
 lkl.l_len = sizeof(struct loan);
 if(fcntl(fd_loan, F_SETLKW, &lkl) == -1)
{
    perror("fcntl error");
    close(fd_cust);
    close(fd_loan);
    return ;
}

 if(lseek(fd_loan,0,SEEK_END) == -1)
    {
        perror("lseek error");
        close(fd_cust);
        close(fd_loan);
        return;
    }
    if((write(fd_loan,&temp_loan,sizeof(struct loan))) <= 0)
    {
        perror("write error");
        close(fd_cust);
        close(fd_loan);
        return;
    }
    //remove lock from loan db.
    lkl.l_type = F_UNLCK;
    if(fcntl(fd_loan,F_SETLK,&lkl) == -1)
    {
        perror("fcntl error");
        close(fd_cust);
        close(fd_loan);
        return;
    }
    // now write to customer_db . take advisory lockthen update entries
    while(read(fd_cust, &temp_cust,sizeof(struct customer) ) >0)
    {
        if(temp_cust.customer_id == cust_id){
            // found record in db and take advisory lock;

            lkl.l_type = F_WRLCK;
            lkl.l_whence = SEEK_CUR;
            lkl.l_start = lseek(fd_cust,0, SEEK_CUR);
            lkl.l_len = sizeof(struct customer);

            if(fcntl(fd_cust, F_SETLKW, &lkl) == -1){

                perror("fcntl error");
                close(fd_cust);
                close(fd_loan);
                return;
            }
            temp_cust.need_loan = true;
            temp_cust.loan_amount  = loan_amount;
            // write to cust db before unlocking.

            if(lseek(fd_cust, -sizeof(struct customer), SEEK_CUR) == -1)
            {
                perror("lseek error");
                close(fd_cust);
                close(fd_loan);
                return;

            }
            if(write(fd_cust, &temp_cust, sizeof(struct customer)) <=0)
            {

                perror("write error");
                close(fd_cust);
                close(fd_loan);
                return;
            }

            //release lock from customer db;
            lkl.l_type = F_UNLCK;
            if(fcntl(fd_cust, F_SETLK, &lkl) == -1)
            {
                perror("fcntl error");
                close(fd_cust);
                close(fd_loan);
                return;
            }

            char msg[256];
            sprintf(msg, "Successfully applied for loan of amount: %d", temp_cust.loan_amount);
            send(client_socket, msg, strlen(msg) ,0);
            break;
        }
    }
    close(fd_cust);
    close(fd_loan);

}

void view_assigned_loan_application(int client_socket, int employee_id)
{
   // used by employee to see how many loan application has been asigned.
   // search using employee id in loand db and show all loan assigned to this amp_id.
   
   int fd_loan = open("loan_db.txt", O_RDONLY);
   if(fd_loan == -1){

    perror("open error");
    return;
   }

   struct loan temp_loan;
   while(read(fd_loan , &temp_loan, sizeof(struct loan)) >0)
   {
    if(temp_loan.emp_id == employee_id)
    {
        // don't break as there can be multiple for single emp in loan db;
        char msg[256];
        sprintf(msg, "cutomer_id: %d, loan amount: %d, status of loan application: %s", temp_loan.customer_id, temp_loan.loan_amount,temp_loan.status);
        send(client_socket, msg, strlen(msg), 0);

    }
   } 
   close(fd_loan);

}


void view_applied_loan_applications(int client_socket)
{
    //used by manager to view all applications that he needs to assign to employees.
    int fd_loan = open("loan_db.txt",O_RDONLY);
    if(fd_loan == -1)
    {
        perror("open error");
        return;
    }
    struct loan temp_loan;
    while(read(fd_loan,&temp_loan,sizeof(struct loan)) > 0)
    {
        if(strcmp(temp_loan.status,"applied") == 0)
        {
            //don't break as there can be multiple entries for single emp in loan db.
            char msg[512];
            sprintf(msg,"Customer_id: %d,loan amount: %d,status of loan application: %s",temp_loan.customer_id,temp_loan.loan_amount,temp_loan.status);
            send(client_socket,msg,strlen(msg),0);
        }
    }
    close(fd_loan);
}
