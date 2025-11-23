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
#include <openssl/sha.h> // For SHA256_DIGEST_LENGTH

#include "commonstruct.h"
#include "credentials.h" // For hashing functions
#include "addEdit.h"


// HELPER FUNCTIONS (FOR DUPLICATE CHECKING)

bool customer_exists(int cust_id, int acc_num) {
    int fd = open("customer_db.txt", O_RDONLY);
    if (fd == -1) {
        return false; // File doesn't exist, so user can't exist
    }

    if (flock(fd, LOCK_SH) == -1) { // Shared (read) lock
        perror("customer_exists: flock read");
        close(fd);
        return true; // Fail safe, assume it exists if we can't check
    }

    struct customer cust;
    while(read(fd, &cust, sizeof(struct customer)) > 0) {
        if (cust.customer_id == cust_id || cust.account_number == acc_num) {
            flock(fd, LOCK_UN);
            close(fd);
            return true;
        }
    }

    flock(fd, LOCK_UN);
    close(fd);
    return false;
}

bool employee_exists(int emp_id, const char* username) {
    int fd = open("employee_db.txt", O_RDONLY);
    if (fd == -1) {
        return false;
    }

    if (flock(fd, LOCK_SH) == -1) {
        perror("employee_exists: flock read");
        close(fd);
        return true; 
    }

    struct employee emp;
    while(read(fd, &emp, sizeof(struct employee)) > 0) {
        if (emp.emp_id == emp_id || strcmp(emp.username, username) == 0) {
            flock(fd, LOCK_UN);
            close(fd);
            return true;
        }
    }

    flock(fd, LOCK_UN);
    close(fd);
    return false;
}

// CUSTOMER FUNCTIONS (CALLED BY EMPLOYEE)

void add_customer(int client_socket)
{
    struct customer new_customer;
    char temp_pswd[128];
    int rcv_bytes;
    int a; // For sending success/failure

    recv(client_socket, new_customer.name, sizeof(new_customer.name), 0);
    recv(client_socket, &new_customer.customer_id, sizeof(new_customer.customer_id), 0);
    recv(client_socket, &new_customer.balance, sizeof(new_customer.balance), 0);
    recv(client_socket, &new_customer.account_number, sizeof(new_customer.account_number), 0);
    recv(client_socket, temp_pswd, sizeof(temp_pswd), 0);
    recv(client_socket, new_customer.address, sizeof(new_customer.address), 0);
    

    // Check for duplicates
    if (customer_exists(new_customer.customer_id, new_customer.account_number)) {
        a = 0; // Failure code
        send(client_socket, &a, sizeof(a), 0);
        return; 
    }
    
    // Hash the password
    unsigned char hashed_pswd[SHA256_DIGEST_LENGTH];
    char hex_pswd[65];
    hash_password(temp_pswd, hashed_pswd);
    password_hash_to_hex(hashed_pswd, hex_pswd);
    strcpy(new_customer.password, hex_pswd);

    // Set defaults
    new_customer.need_loan = false;
    new_customer.loan_amount = 0;
    new_customer.loan_approved = false;
    new_customer.is_online = false;
    new_customer.is_active = true;

    // Open, lock, write, unlock, close
    int fd = open("customer_db.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (fd == -1) {
        perror("add_customer: open");
        a = 0; send(client_socket, &a, sizeof(a), 0);
        return;
    }

    if (flock(fd, LOCK_EX) == -1) {
        perror("add_customer: flock");
        close(fd);
        a = 0; send(client_socket, &a, sizeof(a), 0);
        return;
    }

    if (write(fd, &new_customer, sizeof(struct customer)) <= 0) {
        perror("add_customer: write");
        a = 0; // Failure
    } else {
        a = 1; // Success
    }

    flock(fd, LOCK_UN);
    close(fd);
    
    // Send success/failure code
    send(client_socket, &a, sizeof(a), 0);
}
    
void edit_customer(int client_socket)
{
    int user_id;
    int menu_option;
    int rcv_bytes;
    
    // FIX: Receive Customer ID as int
    rcv_bytes = recv(client_socket, &user_id, sizeof(user_id), 0);
    if (rcv_bytes <= 0) { perror("edit_customer: recv id"); return; }
    
    // FIX: Receive Menu Choice as int
    rcv_bytes = recv(client_socket, &menu_option, sizeof(menu_option), 0);
    if (rcv_bytes <= 0) { perror("edit_customer: recv choice"); return; }

    char name_buf[256];
    int contact_buf;
    char address_buf[100];

    // Receive the data based on the choice
    switch (menu_option)
    {
        case 1: // Name
            recv(client_socket, name_buf, sizeof(name_buf), 0);
            break;
        case 2: // Contact
            recv(client_socket, &contact_buf, sizeof(contact_buf), 0);
            break;
        case 3: // Address
            recv(client_socket, address_buf, sizeof(address_buf), 0);
            break;
        default:
            // Client shouldn't send an invalid option, but good to have
            send(client_socket, "Invalid choice.", strlen("Invalid choice.") + 1, 0);
            return;
    }

    int fd = open("customer_db.txt", O_RDWR);
    if (fd < 0) {
        perror("edit_customer: open");
        send(client_socket, "Server file error.", strlen("Server file error.") + 1, 0);
        return;
    }

    bool found = false;
    struct customer cust_buf;
    while ((read(fd, &cust_buf, sizeof(cust_buf))) > 0)
    {
        if (cust_buf.customer_id == user_id)
        {
            found = true;
            // Lock this specific record
            struct flock lk;
            lk.l_type = F_WRLCK;
            lk.l_whence = SEEK_SET;
            lk.l_start = lseek(fd, 0, SEEK_CUR) - sizeof(struct customer);
            lk.l_len = sizeof(struct customer);

            if (fcntl(fd, F_SETLKW, &lk) == -1) {
                perror("edit_customer: fcntl set lock");
                break;
            }

            // Move pointer back to start of record
            if (lseek(fd, -sizeof(struct customer), SEEK_CUR) == -1) {
                perror("edit_customer: lseek");
                break;
            }

            // Update the record buffer based on choice
            if (menu_option == 1) {
                strcpy(cust_buf.name, name_buf);
            } else if (menu_option == 2) {
                cust_buf.contact = contact_buf;
            } else if (menu_option == 3) {
                strcpy(cust_buf.address, address_buf);
            }

            // Write the updated record
            if (write(fd, &cust_buf, sizeof(cust_buf)) < 0) {
                perror("edit_customer: write");
            }

            // Release lock
            lk.l_type = F_UNLCK;
            fcntl(fd, F_SETLK, &lk);
            break; // Exit loop
        }
    }
    close(fd);

    // FIX: Send the exact response string
    if (found) {
        send(client_socket, "record updated.", strlen("record updated.") + 1, 0);
    } else {
        send(client_socket, "user not updated.", strlen("user not updated.") + 1, 0);
    }
}

// EMPLOYEE FUNCTIONS (CALLED BY ADMIN)

void add_employee(int client_socket)
{
    struct employee new_emp;
    char temp_pswd[512];
    int rcv_bytes;

    rcv_bytes = recv(client_socket, new_emp.username, sizeof(new_emp.username), 0);
    if(rcv_bytes <= 0) { perror("add_employee: recv name"); return; }

    rcv_bytes = recv(client_socket, temp_pswd, sizeof(temp_pswd), 0);
    if(rcv_bytes <= 0) { perror("add_employee: recv pass"); return; }

    rcv_bytes = recv(client_socket, &new_emp.emp_id, sizeof(new_emp.emp_id), 0);
    if(rcv_bytes <= 0) { perror("add_employee: recv id"); return; }

    if (employee_exists(new_emp.emp_id, new_emp.username)) {
        send(client_socket, "employee already exists.\n", strlen("employee already exists.\n") + 1, 0);
        return;
    }

    // Hash the password
    unsigned char hashed_pswd[SHA256_DIGEST_LENGTH];
    char hex_pswd[65];
    hash_password(temp_pswd, hashed_pswd);
    password_hash_to_hex(hashed_pswd, hex_pswd);
    strcpy(new_emp.password, hex_pswd);

    // Set defaults
    new_emp.is_online = false;
    new_emp.is_active = true;

    // Open, lock, write, unlock, close
    int fd = open("employee_db.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (fd == -1) {
        perror("add_employee: open db");
        send(client_socket, "Server error.\n", strlen("Server error.\n") + 1, 0);
        return;
    }

    if (flock(fd, LOCK_EX) == -1) {
        perror("add_employee: flock");
        close(fd);
        return;
    }

    if (write(fd, &new_emp, sizeof(struct employee)) <= 0) {
        perror("add_employee: write");
    } else {
        send(client_socket, "Employee added.\n", strlen("Employee added.\n") + 1, 0);
    }
    
    flock(fd, LOCK_UN);
    close(fd);
}

void edit_employee(int client_socket)
{
    int emp_id;
    char name_buf[256];
    int rcv_bytes;

    rcv_bytes = recv(client_socket, &emp_id, sizeof(emp_id), 0);
    if(rcv_bytes <= 0) { perror("edit_employee: recv id"); return; }

    rcv_bytes = recv(client_socket, name_buf, sizeof(name_buf), 0);
    if(rcv_bytes <= 0) { perror("edit_employee: recv name"); return; }

    int fd = open("employee_db.txt", O_RDWR);
    if (fd == -1) {
        perror("edit_employee: open");
        send(client_socket, "Server file error.", strlen("Server file error.") + 1, 0);
        return;
    }

    struct employee temp_emp;
    bool found = false;
    while ((read(fd, &temp_emp, sizeof(struct employee))) > 0)
    {
        if (temp_emp.emp_id == emp_id)
        {
            found = true;
            struct flock lk;
            lk.l_type = F_WRLCK;
            lk.l_whence = SEEK_SET;
            lk.l_start = lseek(fd, 0, SEEK_CUR) - sizeof(struct employee);
            lk.l_len = sizeof(struct employee);
            
            if (fcntl(fd, F_SETLKW, &lk) == -1) {
                perror("edit_employee: fcntl lock"); break; 
            }
            if (lseek(fd, -sizeof(struct employee), SEEK_CUR) < 0) {
                perror("edit_employee: lseek"); break;
            }

            strcpy(temp_emp.username, name_buf); 

            if (write(fd, &temp_emp, sizeof(temp_emp)) < 0) {
                perror("edit_employee: write");
            }

            lk.l_type = F_UNLCK;
            fcntl(fd, F_SETLK, &lk);
            break;
        }
    }
    close(fd);

    if (found) {
        send(client_socket, "Employee updated.", strlen("Employee updated.") + 1, 0);
    } else {
        send(client_socket, "Employee not found.", strlen("Employee not found.") + 1, 0);
    }
}


void deactivate_customer(int client_socket, int customer_id)
{
    int fd_cust = open("customer_db.txt", O_RDWR);
    if(fd_cust == -1) {
        perror("deactivate_customer: open");
        send(client_socket, "Server file error.", strlen("Server file error.") + 1, 0);
        return;
    }

    struct customer temp_cust;
    bool found = false;
    while(read(fd_cust, &temp_cust, sizeof(struct customer)) > 0)
    {
        if(temp_cust.customer_id == customer_id)
        {
            found = true;
            struct flock lk;
            lk.l_start = lseek(fd_cust, 0, SEEK_CUR) - sizeof(struct customer);
            lk.l_len = sizeof(struct customer);
            lk.l_type = F_WRLCK;
            lk.l_whence = SEEK_SET; // Use SEEK_SET with absolute offset

            if(fcntl(fd_cust, F_SETLKW, &lk) == -1) {
                perror("deactivate_customer: fcntl lock"); break;
            }

            if (lseek(fd_cust, lk.l_start, SEEK_SET) == -1) { // Seek back to start
                perror("deactivate_customer: lseek"); break;
            }

            temp_cust.is_active = false; // Deactivate
            
            if (write(fd_cust, &temp_cust, sizeof(struct customer)) <= 0) {
                perror("deactivate_customer: write");
            }

            lk.l_type = F_UNLCK;
            fcntl(fd_cust, F_SETLK, &lk);
            break;
        }
    }
    close(fd_cust);
    
    if (found) {
        send(client_socket, "Account deactivated.", strlen("Account deactivated.") + 1, 0);
    } else {
        send(client_socket, "Customer not found.", strlen("Customer not found.") + 1, 0);
    }
}

void manage_user_role(int client_socket, int employee_id)
{
    // Find employee and copy details
    int fd_emp = open("employee_db.txt", O_RDWR);
    if(fd_emp == -1) {
        perror("manage_user_role: open emp");
        send(client_socket, "Server file error.", strlen("Server file error.") + 1, 0);
        return;
    }

    struct employee temp_emp;
    bool found = false;
    while((read(fd_emp, &temp_emp, sizeof(struct employee))) > 0)
    {
        if(temp_emp.emp_id == employee_id)
        {
            found = true;
            // Lock, deactivate, write, unlock
            struct flock lk_emp;
            lk_emp.l_type = F_WRLCK;
            lk_emp.l_whence = SEEK_SET;
            lk_emp.l_start = lseek(fd_emp, 0, SEEK_CUR) - sizeof(struct employee);
            lk_emp.l_len = sizeof(struct employee);

            if(fcntl(fd_emp, F_SETLKW, &lk_emp) == -1) {
                 perror("manage_user_role: fcntl lock emp"); break;
            }
            if(lseek(fd_emp, lk_emp.l_start, SEEK_SET) == -1) {
                perror("manage_user_role: lseek emp"); break;
            }

            temp_emp.is_active = false; // Deactivate employee role
            write(fd_emp, &temp_emp, sizeof(struct employee));
            
            lk_emp.l_type = F_UNLCK;
            fcntl(fd_emp, F_SETLK, &lk_emp);
            break;
        }
    }
    close(fd_emp);

    if (!found) {
        send(client_socket, "Employee not found.", strlen("Employee not found.") + 1, 0);
        return;
    }

    // Add as new manager
    struct manager new_manager;
    new_manager.emp_id = temp_emp.emp_id;
    strcpy(new_manager.username, temp_emp.username);
    strcpy(new_manager.password, temp_emp.password);
    new_manager.is_online = false;

    int fd_manager = open("manager_db.txt", O_WRONLY | O_APPEND | O_CREAT, 0644);
    if(fd_manager == -1) {
        perror("manage_user_role: open manager");
        send(client_socket, "Server file error.", strlen("Server file error.") + 1, 0);
        return;
    }

    if (flock(fd_manager, LOCK_EX) == -1) {
        perror("manage_user_role: flock manager");
        close(fd_manager);
        return;
    }

    if((write(fd_manager, &new_manager, sizeof(struct manager))) <= 0) {
        perror("manage_user_role: write manager");
    }

    flock(fd_manager, LOCK_UN);
    close(fd_manager);

    char msg[256];
    sprintf(msg,"Employee '%s' is now a manager.\n", temp_emp.username);
    send(client_socket, msg, strlen(msg) + 1, 0);
}