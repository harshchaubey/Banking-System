#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <stdbool.h>
#include "commonstruct.h" // Make sure this path is correct

int main() {
    // CHANGE THIS to the file you need to fix
    int fd = open("employee_db.txt", O_RDWR); 
    if (fd == -1) {
        perror("Error opening admin_db.txt. Does it exist?");
        return 1;
    }

    // CHANGE THIS to the struct for that file
    struct admin user; 
    int count = 0;

    if (flock(fd, LOCK_EX) == -1) { // Lock the whole file
        perror("Error locking file");
        close(fd);
        return 1;
    }

    // Read each record
    while (read(fd, &user, sizeof(struct admin)) > 0) {
        if (user.is_online == true) {
            // Found a stuck user, reset the flag
            user.is_online = false; 
            
            // Go back to the start of this record
            if (lseek(fd, -sizeof(struct admin), SEEK_CUR) == -1) {
                perror("Error seeking back");
                break;
            }
            
            // Write the corrected record
            if (write(fd, &user, sizeof(struct admin)) == -1) {
                perror("Error writing corrected record");
                break;
            }
            count++;
        }
    }

    flock(fd, LOCK_UN);
    close(fd);

    printf("Reset complete. Logged out %d admin user(s).\n", count);
    return 0;
}