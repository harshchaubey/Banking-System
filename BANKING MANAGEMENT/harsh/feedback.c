#include <fcntl.h>
#include <unistd.h>
#include <stdio.h> 
#include <string.h>
#include <stdbool.h>
#include "commonstruct.h"
#include "feedback.h"
#include <sys/socket.h>
//gcc *.c -o server -lssl -lcrypto



void add_feedback(int client_socket,int user_id,char* feedback)
{
    //this fucntion will be used by customer to add feedback.
    int fd = open("feedback_db.txt",O_RDWR);
    if(fd == -1)
    {
        perror("open error");
        return;
    }
    struct feedback new_feedback;
    new_feedback.user_id = user_id;
    strcpy(new_feedback.feed_back,feedback);
    new_feedback.resolved = false;
    //take lock and add feedback.
    struct flock lk;
    lk.l_type = F_WRLCK;
    lk.l_start = 0;
    lk.l_whence = SEEK_END;
    lk.l_len = sizeof(struct feedback);
    if(fcntl(fd,F_SETLKW,&lk) == -1)
    {
        perror("fcntl error");
        close(fd);
        return;
    }
    if(lseek(fd,0,SEEK_END) == -1)
    {
        perror("lseek error");
        close(fd);
        return;
    }
    if((write(fd,&new_feedback,sizeof(struct feedback))) <= 0)
    {
        perror("write error");
        close(fd);
        return;
    }
    lk.l_type = F_UNLCK;
    if(fcntl(fd,F_SETLK,&lk) == -1)
    {
        perror("fcntl error");
        close(fd);
        return;
    }
    send(client_socket,"Feedback added successfully",strlen("Feedback added successfully"),0);
}

void resolve_feedback(int client_socket,int user_id)
{
    //this fucntion will be used by manager to both view and resolve feedback.
    int fd = open("feedback_db.txt",O_RDWR);
    struct feedback temp_feedback;
    while(read(fd,&temp_feedback,sizeof(struct feedback)) > 0)
    {
        if(temp_feedback.user_id == user_id && temp_feedback.resolved == false)
        {
            //found user in db,show feedback to manager and ask if it's resolved or not.
            //write updated feedback to db.
            send(client_socket,temp_feedback.feed_back,strlen(temp_feedback.feed_back),0);
            send(client_socket,"If feedback is resolved,press 'Y' else press 'N'.",strlen("If feedback is resolved,press 'Y' else press 'N'."),0);
            char manager_response;
            if(recv(client_socket,&manager_response,sizeof(char),0) == -1)
            {
                perror("recv error");
                close(fd);
                return;
            }
            if(manager_response == 'Y')
            {
                temp_feedback.resolved = true;
                send(client_socket,"Feedback marked as resolved",strlen("Feedback marked as resolved"),0);
            }
            else if(manager_response == 'N')
            {
                temp_feedback.resolved = false;
                send(client_socket,"Feedback marked as not resolved",strlen("Feedback marked as not resolved"),0);
            }
            else
            {
                send(client_socket,"Invalid input",strlen("Invalid input"),0);
                close(fd);
                return;
            }
            if(lseek(fd,-sizeof(struct feedback),SEEK_CUR) == -1)
            {
                perror("lseek error");
                close(fd);
                return;
            }
            if((write(fd,&temp_feedback,sizeof(struct feedback))) <= 0)
            {
                perror("write error");
                close(fd);
                return;
            }
            //break;
            //Removed break as single user can have multiple feedbacks.
        }
    }
    close(fd);
}
