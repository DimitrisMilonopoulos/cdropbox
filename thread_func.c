#include <stdio.h>
#include <sys/types.h>  /* sockets */
#include <sys/socket.h> /* sockets */
#include <netinet/in.h> /* internet sockets */
#include <unistd.h>     /* read, write, close */
#include <netdb.h>      /* gethostbyaddr */
#include <stdlib.h>     /* exit */
#include <string.h>     /* strlen */
#include <signal.h>
#include <pthread.h>

/*For the IP*/
#include <unistd.h>

#include <sys/ioctl.h>
#include <net/if.h>
#include <errno.h>
#include <arpa/inet.h>

#include <sys/select.h> /*For the select command*/

#include "info.h"
#include "circular_buffer.h"
#include "list.h"
#include "functions.h"
#include "fileFunctions.h"
#include "thread_func.h"

int clientAnswer(int fd);
int getMessage(int fd, char *message);

void threadFunc(struct circular_buffer *arg_struct)
{
    printf("Hallo i am a lovely thread\n");
    printf("The buffer size is: %d\n", arg_struct->BufferSize);
    struct BufferObject temp_object;
    struct ip_port temp_client;
    uint32_t network_ip;
    uint16_t network_port;
    int sock;
    struct sockaddr_in fellow_client;
    struct sockaddr *fellow_clientptr = (struct sockaddr *)&fellow_client;

    while (1)
    {
        getitem(arg_struct, &temp_object);
        pthread_cond_signal(&arg_struct->cond_nonfull); //Struct isn't full
        //recognize if the temp_object is just port/ip
        if (temp_object.port == 0 && temp_object.ip == 0) //means it's just port and ip so we need to get files
        {
            //check if the object exists inside the client list
            temp_client.ip = temp_object.ip;
            temp_client.port = temp_object.port;
            pthread_mutex_lock(&arg_struct->listlock);
            if (!FindNode(arg_struct->client_list, temp_client))
            {
                pthread_mutex_unlock(&arg_struct->listlock);
                printf("ERROR CLIENT NOT FOUND IN THE SYSTEM...\n");
                continue;
            }
            pthread_mutex_unlock(&arg_struct->listlock);
            //convert to network byte order
            network_ip = htonl(temp_object.ip);
            network_port = htons(temp_object.port);
            //connect to the fellow client to get the file list
            if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
                perror("socket");
            fellow_client.sin_family = AF_INET;
            fellow_client.sin_port = network_port;
            fellow_client.sin_addr.s_addr = network_ip;
            if (connect(sock, fellow_clientptr, sizeof(fellow_client)) < 0)
                perror("connect");
            if (write(sock, "GET_FILE_LIST", 14) < 0)
            {
                perror("Sending file list request:");
            }
            //recognise the message replied
            char response[128];
            getMessage(sock, response);
            printf("THE RESPONSE RECEIVED: %s\n", response);
            if (strcmp(response, "FILE_LIST") != 0)
            {
                close(sock);
                printf("\nGETTING FILE LIST FAILED\n\n");
                continue;
            }

            int number_of_files;
            getMessage(sock, response);
            number_of_files = atoi(response);
            long version;
            for (int i = 0; i < number_of_files; i++)
            {
                getMessage(sock, response);
                strcpy(temp_object.pathname, response);
                getMessage(sock, response);
                version = atol(response);
                temp_object.version = version;
                putitem(arg_struct, temp_object);
                pthread_cond_signal(&arg_struct->cond_nonempty);
                printf("FILE INSERTED IN CIRCULAR QUEUE\n");
            }
            close(sock);
        }
        else
        {
            //check if the object exists inside the client list
            temp_client.ip = temp_object.ip;
            temp_client.port = temp_object.port;
            pthread_mutex_lock(&arg_struct->listlock);
            if (!FindNode(arg_struct->client_list, temp_client))
            {
                pthread_mutex_unlock(&arg_struct->listlock);
                printf("ERROR CLIENT NOT FOUND IN THE SYSTEM...\n");
                continue;
            }
            pthread_mutex_unlock(&arg_struct->listlock);

            //check if the file exists
            char path[128];
            //create the folder name
            struct hostent *he;
            struct in_addr addr;
            addr.s_addr = htonl(temp_client.ip);
            he = gethostbyaddr(&addr, sizeof(addr), AF_INET);
            sprintf(path, "/%s_%d", he->h_name, temp_client.port);
            strcat(path, temp_object.pathname);
            network_ip = htonl(temp_object.ip);
            network_port = htons(temp_object.port);
            //connect to the fellow client to get the file list
            if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
                perror("socket");
            fellow_client.sin_family = AF_INET;
            fellow_client.sin_port = network_port;
            fellow_client.sin_addr.s_addr = network_ip;
            if (connect(sock, fellow_clientptr, sizeof(fellow_client)) < 0)
                perror("connect");

            if (write(sock, "GET_FILE", 9) < 0)
            {
                perror("Sending file request:");
            }

            if (write(sock, temp_object.pathname, strlen(temp_object.pathname) + 1) < 0)
            {
                perror("Writting to fellow client error:");
            }
            if (fileExists(path))
            {
                if (write(sock, "0", 2) < 0)
                {
                    perror("Sending file request:");
                }
            }
            else
            {
                char number[25];
                long timestamp = getFileTime(path);
                sprintf(number, "%ld", timestamp);

                if (write(sock, number, strlen(number) + 1) < 0)
                {
                    perror("Sending file request:");
                }
            }
            //recognise the message
            int answer;
            answer = clientAnswer(sock);
            char message[50];

            switch (answer)
            {
            case 0:
                printf("Invalid command\n");
                break;
            case 1:
                printf("File was not found in the fellow client!!!\n");
                break;
            case 2:
                printf("File up to date!");
                break;
            case 3:
            {
                //local file outdated, time to read the updated file from fellow client
                getMessage(sock, message);
                long newversion = atol(message);
                getMessage(sock, message);

                unsigned int filesize = atoi(message);
                createPath(path);
                fdtoFile(path, sock, 200, filesize);
            }
            }
            close(sock);
        }
    }
}

int getMessage(int fd, char *message)
{
    char readCharacter;
    int i = 0;

    do
    {
        if (read(fd, &readCharacter, 1) < 0)
        {
            perror("read");
            return 0;
        }
        message[i] = readCharacter;
        i++;
    } while (readCharacter != '\0');
    return 1;
}
int clientAnswer(int fd)
{
    char readCharacter;
    char message[30];
    int i = 0;

    do
    {
        if (read(fd, &readCharacter, 1) < 0)
        {
            perror("read");
            return 0;
        }
        message[i] = readCharacter;
        i++;
    } while (readCharacter != '\0');

    if (strcmp(message, "FILE_NOT_FOUND") == 0)
    {
        return 1;
    }
    else if (strcmp(message, "FILE_UP_TO_DATE") == 0)
    {
        return 2;
    }
    else if (strcmp(message, "FILE_SIZE") == 0)
    {
        return 3;
    }
    else
    {
        return 0;
    }
}