#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "credentials.h"
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>  
#include "commonstruct.h"




int main()
{
    // Fix: Added O_CREAT and permissions 0644
    int fd = open("admin_db.txt", O_RDWR | O_CREAT, 0644);

    // Best Practice: Always check if open() succeeded
    if (fd == -1) {
        perror("open error");
        return 1;
    }

    struct admin temp;
    temp.emp_id = 1;
    strcpy(temp.username, "admin1");
    char psd[128] = "pswd";
    
    // Fixed bug from before: Hashed password is binary, not a string
    unsigned char hashed_pswd[SHA256_DIGEST_LENGTH]; 
    
    // Fixed bug from before: Hex string needs 65 bytes (64 hex chars + 1 null terminator)
    char hex_pswd[65];

    printf("original: %s\n", psd);
    
    hash_password(psd, hashed_pswd);
    password_hash_to_hex(hashed_pswd, hex_pswd);
    strcpy(temp.password, hex_pswd);
    
    printf("hex password: %s\n", temp.password);
    
    temp.is_online = false;

    if (write(fd, &temp, sizeof(struct admin)) <= 0)
    {
        perror("write error");
        close(fd); // Good practice to close the file descriptor on error
        return 1;
    }

    printf("Admin user created successfully in admin_db.txt\n");
    close(fd); // Don't forget to close the file on success!
    return 0;
}
