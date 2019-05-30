#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "functions.h"
time_t getFileTime(char *path)
{
    struct stat statbuffer;
    if (stat(path, &statbuffer) == -1)
    {
        perror(path);
        return -1;
    }
    return statbuffer.st_mtime;
}
int countFiles(char *name, FILE *logPtr, int *Filecounter)
{
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(name)))
        return -1;
    int counter = 0;
    char newpath[PATH_MAX];

    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_DIR)
        {
            char path[1024];

            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            counter++;
            sprintf(path, "%s/%s", name, entry->d_name);
            // newprunePath(path, newpath);
            // strcat(newpath, "/");
            // printf("Directory: %s\n", newpath);
            countFiles(path, logPtr, Filecounter);
        }
        else
        {
            counter++;
            strcpy(newpath, name);
            strcat(newpath, "/");
            strcat(newpath, entry->d_name);
            // printf("File: %s\n", newpath);
            //printf("DAFILEFOUND\n");
            //     if (sendFile(newpath, fd, BUFFERSIZE, logPtr, logFD))
            //         return -1;
            // }
            // printf("File: %s time: %ld\n", newpath, getFileTime(newpath));
            *Filecounter += 1; //File found increment the counter of files
        }
    }
    if (counter == 0)
    {
        strcpy(newpath, name);
        strcat(newpath, "/");
        // if (sendFile(newpath, fd, BUFFERSIZE, logPtr, logFD))
        //     return -1;
        printf("Folder: %s\n", newpath);
    }
    closedir(dir);
    return 0;
}

int listdir(char *name, int Socketfd, int BUFFERSIZE, FILE *logPtr, int logFD)
{
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(name)))
        return -1;
    int counter = 0;
    char newpath[PATH_MAX];

    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_DIR)
        {
            char path[1024];

            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            counter++;
            sprintf(path, "%s/%s", name, entry->d_name);
            // newprunePath(path, newpath);
            // strcat(newpath, "/");
            // printf("Directory: %s\n", newpath);
            listdir(path, Socketfd, BUFFERSIZE, logPtr, logFD);
        }
        else
        {
            counter++;
            strcpy(newpath, name);
            strcat(newpath, "/");
            strcat(newpath, entry->d_name);
            // printf("File: %s\n", newpath);
            //printf("DAFILEFOUND\n");
            //     if (sendFile(newpath, fd, BUFFERSIZE, logPtr, logFD))
            //         return -1;
            //
            printf("Sending File: %s time: %ld\n", newpath, getFileTime(newpath));
            if (write(Socketfd, &newpath, strlen(newpath) + 1) < 0)
            {
                perror("Pathname write:");
            }
            time_t timestamp = getFileTime(newpath);
            char timestamp_string[25];
            sprintf(timestamp_string, "%ld", timestamp);
            printf("Time to send in string: %s\n", timestamp_string);
            if (write(Socketfd, timestamp_string, strlen(timestamp_string) + 1) < 0)
            {
                perror("Filetime write");
            }
        }
    }
    if (counter == 0)
    {
        strcpy(newpath, name);
        strcat(newpath, "/");
        // if (sendFile(newpath, fd, BUFFERSIZE, logPtr, logFD))
        //     return -1;
        printf("Folder: %s\n", newpath);
    }
    closedir(dir);
    return 0;
}

long int findSize(const char *file_name) /*Function that recognizes files size*/
{
    struct stat st; /*declare stat variable*/

    if (stat(file_name, &st) == 0)
        return (st.st_size);
    else
        return -1;
}

int main(void)
{
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    FILE *filePtr = fopen(cwd, "r");
    if (filePtr == NULL)
    {
        printf("Error!");
    }
    listdir("nigga", -1, 0, filePtr, 0);
    int fileCount = 0;
    countFiles("nigga", filePtr, &fileCount);
    printf("Count files: %d\n", fileCount);
    printf("Size : %lu\n", sizeof(time_t));
}