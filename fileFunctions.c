#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "functions.h"
#include "fileFunctions.h"

int newprunePath(char *string, char *newstring)
{
    char *ptr;
    char path[PATH_MAX];
    strcpy(path, string);
    char *token = strtok(path, "./");
    if ((ptr = strstr(string, token)) != NULL)
    {
        ptr += strlen(token);
    }
    strcpy(newstring, ptr);

    return 1;
}
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
            countFiles(path, logPtr, Filecounter);
        }
        else
        {
            counter++;
            strcpy(newpath, name);
            strcat(newpath, "/");
            strcat(newpath, entry->d_name);
            *Filecounter += 1; //File found increment the counter of files
        }
    }
    if (counter == 0)
    {
        strcpy(newpath, name);
        strcat(newpath, "/");
        printf("Folder: %s\n", newpath);
    }
    closedir(dir);
    return 0;
}

int listdir(char *name, int Socketfd, FILE *logPtr)
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
            listdir(path, Socketfd, logPtr);
        }
        else
        {
            counter++;
            strcpy(newpath, name);
            strcat(newpath, "/");
            strcat(newpath, entry->d_name);
            char prunedpath[128];
            newprunePath(newpath, prunedpath);
            printf("Sending FileName: %s version: %ld\n", prunedpath, getFileTime(newpath));
            if (write(Socketfd, &prunedpath, strlen(prunedpath) + 1) < 0)
            {
                perror("Pathname write:");
            }
            time_t timestamp = getFileTime(newpath);
            char timestamp_string[21];
            sprintf(timestamp_string, "%ld", timestamp);
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

int createPath(char *path) /*Create the path that the directory suggests*/
{
    printf("The path to create: %s", path);
    int directory;
    char temp2[PATH_MAX];
    strcpy(temp2, path);
    if (path[strlen(path) - 1] != '/')
        strcat(temp2, "-");
    char temp[PATH_MAX];
    strcpy(temp, "");
    char *token;
    char *rest = temp2;
    token = strtok_r(rest, "/", &rest);
    strcat(temp, token);
    strcat(temp, "/");

    token = strtok_r(rest, "/", &rest);
    while (token != NULL && (token[strlen(token) - 1] != '-'))
    {
        strcat(temp, token);
        printf("MAKING DIRECTORY: %s\n", temp);
        directory = mkdir(temp, 0777);
        strcat(temp, "/");
        token = strtok_r(rest, "/", &rest);
    }
    //means that we've successfully read through the file
    if (token != NULL)
    {
        strcat(temp, token);
        temp[strlen(temp) - 1] = '\0';
        FILE *file = fopen(temp, "w");

        if (file == NULL)
        {
            perror("File couln't be created");
            return 0;
        }
        fclose(file);
        return 1;
    }
    return 0;
}

int copyfile(char *sourcefile, int outputptr, int BUFFSIZE)
{
    /*Copying the file to send it via the file descriptor*/
    int infile;
    ssize_t nread;
    char buffer[BUFFSIZE];

    if ((infile = open(sourcefile, O_RDONLY)) == -1)
    {
        perror("Error opening input file");
        return -1;
    }

    while ((nread = read(infile, buffer, BUFFSIZE)) > 0)
    {
        if (write(outputptr, buffer, nread) < nread)
        {
            close(infile);
            perror("An Error writing output file");
            return -3;
        }
    }
    close(infile);
    if (nread == -1)
        return -4;
    else
        return 0;
}

int fdtoFile(char *outname, int indesc, int BUFFSIZE, int fileSize)
{
    int outfile;
    ssize_t nread;
    char buffer[BUFFSIZE];
    int counter = 0;
    int readAmount = BUFFSIZE;
    if ((outfile = open(outname, O_WRONLY, 0644)) == -1)
    {
        printf("The outname: %s\n", outname);
        perror("Outfile");
        return -2;
    }
    if (fileSize <= BUFFSIZE)
    {
        printf("It's smaller\n");
        nread = read(indesc, buffer, fileSize);
        if (write(outfile, buffer, nread) < nread)
        {
            printf("Fatal error!\n");
            close(outfile);
            return -3;
        }
        close(outfile);
        return 0;
    }
    while ((nread = read(indesc, buffer, readAmount)) > 0)
    {
        counter += nread;
        if (fileSize - counter < BUFFSIZE)
        {
            readAmount = fileSize - counter;
        }
        if (write(outfile, buffer, nread) < nread)
        {
            close(outfile);
            return -4;
        }
        if (counter == fileSize)
        {
            break;
        }
    }

    if (fileSize != counter)
    {
        fprintf(stderr, "File wasn't read correctly");
        close(outfile);
        return -1;
    }
    close(outfile);
    if (nread == -1)
        return -5;
    return 0;
}

int fileExists(char *filename)
{
    if (access(filename, F_OK) != -1)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}