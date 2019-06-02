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
extern int terminate;

int clientAnswer(int fd);
int getMessage(int fd, char *message);

void clean_up(void *arg)
{
    printf("THIS IS THE CLEANUP FUNCTION\n");
    struct circular_buffer *circBuffer = (struct circular_buffer *)arg;
    pthread_cond_signal(&circBuffer->cond_nonempty);
    pthread_cond_signal(&circBuffer->cond_nonfull);
    pthread_mutex_unlock(&circBuffer->bufferlock);
}

void threadFunc(struct circular_buffer *arg_struct)
{

    printf("The initial terminate value is: %d\n", terminate);
    printf("Hallo i am a lovely thread\n");
    printf("The buffer size is: %d\n", arg_struct->BufferSize);
    //get the name of the host
    char hostIP[30];
    char hostName[60];
    getIP(hostIP, hostName);
    printf("\nThe hostname of the THREAD: %s\n", hostName);
    struct BufferObject temp_object;
    struct ip_port temp_client;
    uint32_t network_ip;
    uint16_t network_port;
    int sock;
    struct sockaddr_in fellow_client;
    struct sockaddr *fellow_clientptr = (struct sockaddr *)&fellow_client;
    pthread_cleanup_push(clean_up, (void *)arg_struct);
    while (1)
    {
        getitem(arg_struct, &temp_object);
        if (terminate)
        {
            printf("Terminating from get item!\n");
            break;
        }
        pthread_cond_signal(&arg_struct->cond_nonfull); //Struct isn't full
        printf("\nPOPPED AN ITEM FROM QUEUE\n");
        //check if the object exists inside the client list
        temp_client.ip = temp_object.ip;
        temp_client.port = temp_object.port;
        printf("THREAD: GOING TO CONNECT TO: %u/%u\n", temp_client.ip, temp_client.port);
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
        printf("THREAD: GOING TO CONNECT TO(NBO): %u/%u\n", network_ip, network_port);

        //connect to the fellow client to get the file list
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            perror("socket");
        fellow_client.sin_family = AF_INET;
        fellow_client.sin_port = network_port;
        struct in_addr temp_addr = {};
        temp_addr.s_addr = network_ip;
        fellow_client.sin_addr = temp_addr;
        if (connect(sock, fellow_clientptr, sizeof(fellow_client)) < 0)
            perror("connect THREAD");
        else
        {
            printf("Connected SUCCESS!\n");
        }
        /////////////////////

        //recognize if the temp_object is just port/ip
        if (temp_object.pathname[0] == '\0' && temp_object.version == 0) //means it's just port and ip so we need to get files
        {

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
                if (terminate)
                {
                    close(sock);
                    break;
                }
                pthread_cond_signal(&arg_struct->cond_nonempty);
                printf("FILE INSERTED IN CIRCULAR QUEUE\n");
            }
            if (terminate)
            {
                break;
            }
        }
        else
        {
            printf("FILE OPERATION!\n");
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
            sprintf(path, "./%s_mirror/%s_%d", hostName, he->h_name, temp_client.port);
            strcat(path, temp_object.pathname);

            if (write(sock, "GET_FILE", 9) < 0)
            {
                perror("Sending file request:");
            }

            if (write(sock, temp_object.pathname, strlen(temp_object.pathname) + 1) < 0)
            {
                perror("Writting to fellow client error:");
            }
            if (!fileExists(path)) //if file doesn't exists
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
                printf("RECEIVING FILE!\n");
                //local file outdated, time to read the updated file from fellow client
                getMessage(sock, message);
                long newversion = atol(message);
                getMessage(sock, message);

                unsigned int filesize = atoi(message);
                printf("The size of the file going to be written is: %u\n in path: %s\n", filesize, path);
                createPath(path);
                if (fdtoFile(path, sock, 200, filesize))
                {
                    printf("\n\n ERROR SENDING FILE \n\n");
                }
            }
            }
        }
        close(sock);
    }
    pthread_cleanup_pop(1);
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