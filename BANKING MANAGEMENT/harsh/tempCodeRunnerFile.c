
#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>
#include <fcntl.h>      
#include <unistd.h>     
#include <sys/stat.h> 
#include <sys/socket.h>
#include <stdbool.h>
#include <openssl/evp.h>
#include "commonstruct.h"
#include <ctype.h>
#include "credentials.h"




//username will be unique in db.
void trim_trailing_spaces(char *str) {
    if (str == NULL) return; // Check for NULL pointer

    int len = strlen(str);
    
    // Find the last non-space character
    while (len > 0 && isspace((unsigned char)str[len - 1])) {
        len--;
    }
    
    // Null terminate the string at the new length
    str[len] = '\0';
}
void trim_leading_spaces(char *str) {
    // Check if the string is empty
    if (str == NULL || *str == '\0') {
        return;
    }

    // Move the pointer to the first non-space character
    while (*str && isspace((unsigned char)*str)) {
        str++;
    }

    // Copy the trimmed string back to the original string
    // This effectively removes the leading spaces
    memmove(str - (str - str), str, strlen(str) + 1);
}
void hash_password(const char* pswd, unsigned char* hashed_pswd) {
    EVP_MD_CTX* context = EVP_MD_CTX_new();
    const EVP_MD* md = EVP_sha256(); //specify SHA256 as the algo that we want to use.        
    EVP_DigestInit_ex(context, md, NULL);
    EVP_DigestUpdate(context, pswd, strlen(pswd));
    EVP_DigestFinal_ex(context, hashed_pswd, NULL);

    EVP_MD_CTX_free(context);  
}
void password_hash_to_hex(const unsigned char* hashed_pswd,char* hex_pswd)
{
	for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(hex_pswd + (i * 2), "%02x", hashed_pswd[i]);
    }
}

void edit_credentials_customer(int client_socket,int user_id)
{
	//this fucntion will work only for customer.
	//find username and pswd in the customer_db.txt file and then update only password.
	struct customer temp;
	int fd = open("customer_db.txt",O_RDWR);
	while(read(fd,&temp,sizeof(temp)) > 0)
	{
		if(temp.customer_id == user_id)
		{
			//found the username in db,take advisory lock then overwrite it but set offset properly before doing so.
			struct flock lk;
			lk.l_type = F_WRLCK;
			lk.l_start = lseek(fd,0,SEEK_CUR) - sizeof(temp);
			lk.l_len = sizeof(temp);
			lk.l_whence = SEEK_CUR;
			if(fcntl(fd,F_SETLKW,&lk) == -1)
			{
				perror("error while taking lock");
				close(fd);
				//close(client_socket);
				return;
			}
			if(lseek(fd,-sizeof(temp),SEEK_CUR) == -1)
			{
				perror("lseek error");
				close(fd);
				return;
			}
			unsigned char hashed_pswd[SHA256_DIGEST_LENGTH];
			char hex_pswd[65];
			char temp_pswd[128];
			send(client_socket,"Enter new password:",strlen("Enter new password:"),0);
			int rec_client = recv(client_socket,&temp_pswd,sizeof(temp_pswd),0);
			temp_pswd[rec_client] = '\0';
			if(rec_client == -1)
			{
				perror("recv error");
				return;
			}
			trim_trailing_spaces(temp_pswd);
			trim_leading_spaces(temp_pswd);
			hash_password(temp_pswd,hashed_pswd);
			password_hash_to_hex(hashed_pswd,hex_pswd);
			strcpy(temp.password,hex_pswd);
			//write this updated value in db file.
			if(lseek(fd,-sizeof(struct customer),SEEK_CUR) == -1)
			{
				perror("lseek error");
				return;
			}
			if((write(fd,&temp,sizeof(struct customer))) <= 0)
			{
				perror("write error");
				return;
			}
			lk.l_type = F_UNLCK;
			if(fcntl(fd,F_SETLK,&lk) == -1)
			{
				perror("error while releasing lock");
				close(fd);
				//close(client_socket);
				return;
			}
			send(client_socket,"Password updated successfully",strlen("Password updated successfully"),0);
			close(fd);
			return;
		}
	}
		send(client_socket,"User credentials not present in db.",strlen("User credentials not present in db."),0);
		close(fd);
		return;
}
void edit_credentials_employee(int client_socket,int user_id)
{
	//this fucntion will work for all except for customer bcz in struct  customer we've customer_id while in other struct we've emp_id.
	struct employee temp;
	int fd = open("employee_db.txt",O_RDWR);
	while(read(fd,&temp,sizeof(temp)) > 0)
	{
		if(temp.emp_id == user_id)
		{
			//found the username in db,take advisory lock then overwrite it but set offset properly before doing so.
			struct flock lk;
			lk.l_type = F_WRLCK;
			lk.l_start = lseek(fd,0,SEEK_CUR) - sizeof(temp);
			lk.l_len = sizeof(temp);
			lk.l_whence = SEEK_CUR;
			if(fcntl(fd,F_SETLKW,&lk) == -1)
			{
				perror("error while taking lock");
				close(fd);
				//close(client_socket);
				return;
			}
			if(lseek(fd,-sizeof(temp),SEEK_CUR) == -1)
			{
				perror("lseek error");
				close(fd);
				return;
			}
			unsigned char hashed_pswd[SHA256_DIGEST_LENGTH];
			char hex_pswd[65];
			char temp_pswd[128];
			send(client_socket,"Enter new password:",strlen("Enter new password:"),0);
			int rec_client = recv(client_socket,&temp_pswd,sizeof(temp_pswd),0);
			temp_pswd[rec_client] = '\0';
			if(rec_client == -1)
			{
				perror("recv error");
				return;
			}
			trim_trailing_spaces(temp_pswd);
			trim_leading_spaces(temp_pswd);
			hash_password(temp_pswd,hashed_pswd);
			password_hash_to_hex(hashed_pswd,hex_pswd);
			strcpy(temp.password,hex_pswd);
			//write this updated value in db file.
			if(lseek(fd,-sizeof(struct employee),SEEK_CUR) == -1)
			{
				perror("lseek error");
				return;
			}
			if((write(fd,&temp,sizeof(struct employee))) <= 0)
			{
				perror("write error");
				return;
			}
			lk.l_type = F_UNLCK;
			if(fcntl(fd,F_SETLK,&lk) == -1)
			{
				perror("error while releasing lock");
				close(fd);
				//close(client_socket);
				return;
			}
			send(client_socket,"Password updated successfully",strlen("Password updated successfully"),0);
			close(fd);
			return;
		}
	}
	send(client_socket,"User credentials not present in db.",strlen("User credentials not present in db."),0);
	close(fd);
	return;
}

void edit_credentials_manager(int client_socket,int user_id)
{
	struct manager temp;
	int fd = open("manager_db.txt",O_RDWR);
	while(read(fd,&temp,sizeof(temp)) > 0)
	{
		if(temp.emp_id == user_id)
		{
			//found the username in db,take advisory lock then overwrite it but set offset properly before doing so.
			struct flock lk;
			lk.l_type = F_WRLCK;
			lk.l_start = lseek(fd,0,SEEK_CUR) - sizeof(temp);
			lk.l_len = sizeof(temp);
			lk.l_whence = SEEK_CUR;
			if(fcntl(fd,F_SETLKW,&lk) == -1)
			{
				perror("error while taking lock");
				close(fd);
				//close(client_socket);
				return;
			}
			if(lseek(fd,-sizeof(temp),SEEK_CUR) == -1)
			{
				perror("lseek error");
				close(fd);
				return;
			}
			unsigned char hashed_pswd[SHA256_DIGEST_LENGTH];
			char hex_pswd[65];
			char temp_pswd[128];
			send(client_socket,"Enter new password:",strlen("Enter new password:"),0);
			int rec_client = recv(client_socket,&temp_pswd,sizeof(temp_pswd),0);
			temp_pswd[rec_client] = '\0';
			if(rec_client == -1)
			{
				perror("recv error");
				return;
			}
			trim_trailing_spaces(temp_pswd);
			trim_leading_spaces(temp_pswd);
			hash_password(temp_pswd,hashed_pswd);
			password_hash_to_hex(hashed_pswd,hex_pswd);
			strcpy(temp.password,hex_pswd);
			//write this updated value in db file.
			if(lseek(fd,-sizeof(struct manager),SEEK_CUR) == -1)
			{
				perror("lseek error");
				return;
			}
			if((write(fd,&temp,sizeof(struct manager))) <= 0)
			{
				perror("write error");
				return;
			}
			lk.l_type = F_UNLCK;
			if(fcntl(fd,F_SETLK,&lk) == -1)
			{
				perror("error while releasing lock");
				close(fd);
				//close(client_socket);
				return;
			}
			send(client_socket,"Password updated successfully",strlen("Password updated successfully"),0);
			close(fd);
			return;
		}
	}
	send(client_socket,"User credentials not present in db.",strlen("User credentials not present in db."),0);
	close(fd);
	return;
}

void edit_credentials_admin(int client_socket,int user_id)
{
	//this fucntion will work for all except for customer bcz in struct  customer we've customer_id while in other struct we've emp_id.
	struct admin temp;
	int fd = open("admin_db.txt",O_RDWR);
	while(read(fd,&temp,sizeof(temp)) > 0)
	{
		if(temp.emp_id == user_id)
		{
			//found the username in db,take advisory lock then overwrite it but set offset properly before doing so.
			struct flock lk;
			lk.l_type = F_WRLCK;
			lk.l_start = lseek(fd,0,SEEK_CUR) - sizeof(temp);
			lk.l_len = sizeof(temp);
			lk.l_whence = SEEK_CUR;
			if(fcntl(fd,F_SETLKW,&lk) == -1)
			{
				perror("error while taking lock");
				close(fd);
				//close(client_socket);
				return;
			}
			if(lseek(fd,-sizeof(temp),SEEK_CUR) == -1)
			{
				perror("lseek error");
				close(fd);
				return;
			}
			unsigned char hashed_pswd[SHA256_DIGEST_LENGTH];
			char hex_pswd[65];
			char temp_pswd[128];
			send(client_socket,"Enter new password:",strlen("Enter new password:"),0);
			int rec_client = recv(client_socket,&temp_pswd,sizeof(temp_pswd),0);
			temp_pswd[rec_client] = '\0';
			if(rec_client == -1)
			{
				perror("recv error");
				return;
			}
			trim_trailing_spaces(temp_pswd);
			trim_leading_spaces(temp_pswd);
			hash_password(temp_pswd,hashed_pswd);
			password_hash_to_hex(hashed_pswd,hex_pswd);
			strcpy(temp.password,hex_pswd);
			//write this updated value in db file.
			if(lseek(fd,-sizeof(struct admin),SEEK_CUR) == -1)
			{
				perror("lseek error");
				return;
			}
			if((write(fd,&temp,sizeof(struct admin))) <= 0)
			{
				perror("write error");
				return;
			}
			lk.l_type = F_UNLCK;
			if(fcntl(fd,F_SETLK,&lk) == -1)
			{
				perror("error while releasing lock");
				close(fd);
				//close(client_socket);
				return;
			}
			send(client_socket,"Password updated successfully",strlen("Password updated successfully"),0);
			close(fd);
			return;
		}
	}
	send(client_socket,"User credentials not present in db.",strlen("User credentials not present in db."),0);
	close(fd);
	return;
}
bool authenticate_customer(int client_socket,int user_id,const char *pswd)
{
	struct customer temp; //to read data from file
	unsigned char hashed_pswd[SHA256_DIGEST_LENGTH];
	char hex_pswd[65];
	trim_trailing_spaces(pswd);
	trim_leading_spaces(pswd);
	hash_password(pswd,hashed_pswd);
	password_hash_to_hex(hashed_pswd,hex_pswd);

	//find userid and pswd in db and match them.
	int fd = open("customer_db.txt",O_RDWR);
	while(read(fd,&temp,sizeof(struct customer)) > 0)
	{
		if(temp.customer_id == user_id)
		{
			//first check if user is already logged in or not.
			//allow login only if user is not logged in.
			if(temp.is_active == false)
			{
				send(client_socket,"Inactive account",strlen("Inactive account"),0);
				return false;
			}
			if(temp.is_online == true)
			{
				send(client_socket,"User already logged in.",strlen("User already logged in."),0);
				return false;
			}
			if(strcmp(temp.password,hex_pswd) == 0)
			{
				send(client_socket,"Authentication successful",strlen("Authentication successful"),0);
				temp.is_online = true;
				//write this updated value in db file.
				if(lseek(fd,-sizeof(struct customer),SEEK_CUR) == -1)
				{
					perror("lseek error");
					return false;
				}
				if((write(fd,&temp,sizeof(struct customer))) <= 0)
				{
					perror("write error");
					return false;
				}
				return true;
			}
		}
	}
	close(fd);
	send(client_socket,"Authentication failed",strlen("Authentication failed"),0);
	//close(client_socket);
	return false;
}

bool authenticate_employee(int client_socket,int user_id,char* pswd)
{
	struct employee temp; //to read data from file
	unsigned char hashed_pswd[SHA256_DIGEST_LENGTH];
	char hex_pswd[65];
	trim_trailing_spaces(pswd);
	trim_leading_spaces(pswd);
	hash_password(pswd,hashed_pswd);
	password_hash_to_hex(hashed_pswd,hex_pswd);
	//this is for testing
	char msg[1024];
	sprintf(msg,"original:%s|,empid:%d",pswd,user_id);
	send(client_socket,msg,strlen(msg),0);
	//find username and pswd in db and match them.
	int fd = open("employee_db.txt",O_RDWR);
	while(read(fd,&temp,sizeof(struct employee)) > 0)
	{
		if(temp.emp_id == user_id)
		{
			//first check if user is already logged in or not.
			//allow login only if user is not logged in.
			if(temp.is_online == true)
			{
				send(client_socket,"User already logged in.",strlen("User already logged in."),0);
				return false;
			}
			if(temp.is_active == false)
			{
				send(client_socket,"Inactive account",strlen("Inactive account"),0);
				return false;
			}
			if(strcmp(temp.password,hex_pswd) == 0)
			{
				send(client_socket,"Authentication successful",strlen("Authentication successful"),0);
				temp.is_online = true;
				//write this updated value in db file.
				if(lseek(fd,-sizeof(struct employee),SEEK_CUR) == -1)
				{
					perror("lseek error");
					return false;
				}
				if((write(fd,&temp,sizeof(struct employee))) <= 0)
				{
					perror("write error");
					return false;
				}
				return true;
			}
		}
	}
	close(fd);
	send(client_socket,"Authentication failed",strlen("Authentication failed"),0);
	//close(client_socket);
	return false;
}

bool authenticate_manager(int client_socket,int user_id,char* pswd)
{
	struct manager temp; //to read data from file
	unsigned char hashed_pswd[SHA256_DIGEST_LENGTH];
	char hex_pswd[65];
	trim_trailing_spaces(pswd);
	trim_leading_spaces(pswd);
	hash_password(pswd,hashed_pswd);
	password_hash_to_hex(hashed_pswd,hex_pswd);

	//find username and pswd in db and match them.
	int fd = open("manager_db.txt",O_RDWR);
	while(read(fd,&temp,sizeof(struct manager)) > 0)
	{
		if(temp.emp_id == user_id)
		{
			//first check if user is already logged in or not.
			//allow login only if user is not logged in.
			if(temp.is_online == true)
			{
				send(client_socket,"User already logged in.",strlen("User already logged in."),0);
				return false;
			}
			if(strcmp(temp.password,hex_pswd) == 0)
			{
				send(client_socket,"Authentication successful",strlen("Authentication successful"),0);
				temp.is_online = true;
				//write this updated value in db file.
				if(lseek(fd,-sizeof(struct manager),SEEK_CUR) == -1)
				{
					perror("lseek error");
					return false;
				}
				if((write(fd,&temp,sizeof(struct manager))) <= 0)
				{
					perror("write error");
					return false;
				}
				return true;
			}
		}
	}
	close(fd);
	send(client_socket,"Authentication failed",strlen("Authentication failed"),0);
	//close(client_socket);
	return false;
}

bool authenticate_admin(int client_socket,int user_id,char* pswd)
{
	struct admin temp; //to read data from file
	unsigned char hashed_pswd[SHA256_DIGEST_LENGTH];
	char hex_pswd[65];
	//this is for testing
	trim_trailing_spaces(pswd);
	trim_leading_spaces(pswd);
	//char msg[128];
	//sprintf(msg,"original:%s|",pswd);
	//send(client_socket,msg,strlen(msg),0);
	
	hash_password(pswd,hashed_pswd);
	//sprintf(msg,"hash:%s",hashed_pswd);
	//send(client_socket,msg,strlen(msg),0);
	password_hash_to_hex(hashed_pswd,hex_pswd);
	//this if for testing
	//char msg[1024];
	//sprintf(msg,"hex password:%s",hex_pswd);
	//send(client_socket,msg,strlen(msg),0);
	//find username and pswd in db and match them.
	int fd = open("admin_db.txt",O_RDWR);
	while(read(fd,&temp,sizeof(struct admin)) > 0)
	{
		if(temp.emp_id == user_id)
		{
			//first check if user is already logged in or not.
			//allow login only if user is not logged in.
			if(temp.is_online == true)
			{
				send(client_socket,"User already logged in.",strlen("User already logged in."),0);
				return false;
			}
			if(strcmp(temp.password,hex_pswd) == 0)
			{
				send(client_socket,"Authentication successful",strlen("Authentication successful"),0);
				temp.is_online = true;
				//write this updated value in db file.
				if(lseek(fd,-sizeof(struct admin),SEEK_CUR) == -1)
				{
					perror("lseek error");
					return false;
				}
				if((write(fd,&temp,sizeof(struct admin))) <= 0)
				{
					perror("write error");
					return false;
				}
				return true;
			}
		}
	}
	close(fd);
	send(client_socket,"Authentication failed",strlen("Authentication failed"),0);
	//close(client_socket);
	return false;
}
void customer_logout(int client_socket,int user_id)
{
	//find user in credentials_db and make is_online=false.
	struct customer temp; //to read data from file
	int fd = open("customer_db.txt",O_RDWR);
	while(read(fd,&temp,sizeof(struct customer)) > 0)
	{
		if(temp.customer_id == user_id)
		{
			temp.is_online = false;
			//write this updated value in db file.
			if(lseek(fd,-sizeof(struct customer),SEEK_CUR) == -1)
			{
				perror("lseek error");
				return ;
			}
			if((write(fd,&temp,sizeof(struct customer))) <= 0)
			{
				perror("write error");
				return ;
			}
			send(client_socket,"Logged out",strlen("Logged out"),0);
			break;
		}
	}
}

void employee_logout(int client_socket,int user_id)
{
	//find user in credentials_db and make is_online=false.
	struct employee temp; //to read data from file
	int fd = open("employee_db.txt",O_RDWR);
	while(read(fd,&temp,sizeof(struct employee)) > 0)
	{
		if(temp.emp_id == user_id)
		{
			temp.is_online = false;
			//write this updated value in db file.
			if(lseek(fd,-sizeof(struct employee),SEEK_CUR) == -1)
			{
				perror("lseek error");
				return ;
			}
			if((write(fd,&temp,sizeof(struct employee))) <= 0)
			{
				perror("write error");
				return ;
			}
			send(client_socket,"Logged out",strlen("Logged out"),0);
			break;
		}
	}
}

void manager_logout(int client_socket,int user_id)
{
	//find user in credentials_db and make is_online=false.
	struct manager temp; //to read data from file
	int fd = open("manager_db.txt",O_RDWR);
	while(read(fd,&temp,sizeof(struct manager)) > 0)
	{
		if(temp.emp_id == user_id)
		{
			temp.is_online = false;
			//write this updated value in db file.
			if(lseek(fd,-sizeof(struct manager),SEEK_CUR) == -1)
			{
				perror("lseek error");
				return ;
			}
			if((write(fd,&temp,sizeof(struct manager))) <= 0)
			{
				perror("write error");
				return ;
			}
			send(client_socket,"Logged out",strlen("Logged out"),0);
			break;
		}
	}
}

void admin_logout(int client_socket,int user_id)
{
	//find user in credentials_db and make is_online=false.
	struct admin temp; //to read data from file
	int fd = open("admin_db.txt",O_RDWR);
	while(read(fd,&temp,sizeof(struct admin)) > 0)
	{
		if(temp.emp_id == user_id)
		{
			temp.is_online = false;
			//write this updated value in db file.
			if(lseek(fd,-sizeof(struct admin),SEEK_CUR) == -1)
			{
				perror("lseek error");
				return ;
			}
			if((write(fd,&temp,sizeof(struct admin))) <= 0)
			{
				perror("write error");
				return ;
			}
			send(client_socket,"Logged out",strlen("Logged out"),0);
			break;
		}
	}
}

