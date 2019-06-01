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
#include "thread_func.h"
#include "fileFunctions.h"

void perror_exit(char *message);
void getHostIP(char *);
int terminate = 0;

void closeClient(int signum)
{
    terminate = 1;
}

int main(int argc, char **argv)
{
    char ip_buffer[30];
    char host_name[60];
    getIP(ip_buffer, host_name);
    printf("Found your ip adress to be: %s\n", ip_buffer);
    struct client_info *info = read_client_args(argc, argv);
    printClientInfo(info);

    int port, sock, i;
    char buf[256];

    struct sockaddr_in server;
    struct sockaddr *serverptr = (struct sockaddr *)&server;
    struct hostent *rem;

    struct sigaction toExit = {};

    toExit.sa_handler = closeClient;
    toExit.sa_flags = SA_NODEFER;

    if (sigaction(SIGINT, &toExit, NULL) == -1)
    {
        perror("Error in sigaction");
        exit(EXIT_FAILURE);
    }

    if (sigaction(SIGQUIT, &toExit, NULL) == -1)
    {
        perror("Error in sigaction");
        exit(EXIT_FAILURE);
    }

    //create the list for the fellow clients
    struct HeadNode *clientList = createQueue();
    //initializing the circular buffer
    struct circular_buffer *circBuf = InitBuffer((int)info->bufferSize, clientList);
    struct BufferObject object_to_insert;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        perror_exit("socket");
    /* Find server address */
    server.sin_family = AF_INET;               /* Internet domain */
    server.sin_port = htons(info->serverPort); /* Server port:used htons */
    /* Initiate connection */
    if (!inet_aton(info->serverIP, &server.sin_addr))
    {
        printf("Error\n");
    }
    printf("The adress in uint is %d\n", server.sin_addr.s_addr);
    if (connect(sock, serverptr, sizeof(server)) < 0)
        perror_exit("connect");

    /*Send log on message to server*/
    //create the socket for the client

    struct sockaddr_in cl_server,
        client;
    struct sockaddr *cl_serverptr = (struct sockaddr *)&cl_server;
    struct sockaddr *clientptr = (struct sockaddr *)&client;

    int cl_sock;
    /* Create socket */
    if ((cl_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        perror_exit("socket");

    cl_server.sin_family = AF_INET;                /* Internet domain */
    cl_server.sin_addr.s_addr = htonl(INADDR_ANY); //watch out
    cl_server.sin_port = htons(info->portNum);     /* The given port */

    /* Bind socket to address */
    if (bind(cl_sock, cl_serverptr, sizeof(cl_server)) < 0)
        perror_exit("bind");
    /* Listen for connections */
    if (listen(cl_sock, 15) < 0)
        perror_exit(" listen ");

    fd_set active_fd_set, read_fd_set;
    /* Initialize the set of active sockets. */
    FD_ZERO(&active_fd_set);
    FD_SET(cl_sock, &active_fd_set);
    socklen_t size;
    //create threads
    // pthread_t mythread[2];
    // pthread_create(&mythread[0], NULL, (void *)threadFunc, circBuf);
    // pthread_create(&mythread[1], NULL, (void *)threadFunc, circBuf);
    pthread_t mythreads[info->workerThreads];
    for (int i = 0; i < info->workerThreads; i++)
    {
        pthread_create(&mythreads[i], NULL, (void *)threadFunc, circBuf);
    }

    printf("Going to create threads!");
    //convert the ip adress and socket to binary form for transfer
    struct sockaddr_in myaddr;
    inet_aton(ip_buffer, &myaddr.sin_addr);
    printf("The sin_addr is %u\n", myaddr.sin_addr.s_addr);
    uint32_t ipbinary = myaddr.sin_addr.s_addr; //htonl(myaddr.sin_addr.s_addr); //network compatible
    printf("The sin_addr is %u\n\n", ipbinary);
    uint16_t portnet = htons(info->portNum); //htons(info->portNum);

    //send the log on message to server
    if (write(sock, "LOG_ON", 7) < 0)
        perror("write");

    //send the ip adress
    if (write(sock, &ipbinary, 4) < 0)
        perror("write");

    //send the port number
    if (write(sock, &portnet, 2) < 0)
        perror("write");
    close(sock);
    // shutdown(sock, SHUT_WR);
    printf("Connecting again\n");
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        perror_exit("socket");
    if (connect(sock, serverptr, sizeof(server)) < 0)
        perror_exit("connect");
    //Get the other clients
    printf("Getting the clients\n");
    //send the log on message to server
    if (write(sock, "GET_CLIENTS", 12) < 0)
        perror("write");
    //send the ip adress
    if (write(sock, &ipbinary, 4) < 0)
        perror("write");

    //send the port number
    if (write(sock, &portnet, 2) < 0)
        perror("write");

    //read the response from the server
    //read the number of clients the server is going to send
    char response[30];
    if (read(sock, response, 12) != 12)
    {
        perror("read");
    }

    if (strcmp(response, "CLIENT_LIST") != 0)
    {
        printf("Invalid response");
    }
    int number_of_clients;
    // if (read(sock, &number_of_clients, 4) != 4)
    // {
    //     perror("read");
    // }
    // number_of_clients = ntohl(number_of_clients);

    char character;
    char numbuf[10];
    int k = 0;
    do
    {
        if (read(sock, &character, 1) != 1)
        {
            perror("read");
        }
        numbuf[k] = character;
        k++;
    } while (character != '\0');
    printf("READ: %s\n", numbuf);
    number_of_clients = atoi(numbuf);
    printf("Number of clients it is goind to read %d\n", number_of_clients);
    for (int i = 0; i < number_of_clients; i++)
    {
        struct ip_port *newclient = malloc(sizeof(struct ip_port));
        if (read(sock, &newclient->ip, 4) != 4)
            perror("read");
        newclient->ip = ntohl(newclient->ip);

        if (read(sock, &newclient->port, 2) != 2)
            perror("read");
        newclient->port = ntohs(newclient->port);
        printf("New client %u new port %u\n", newclient->ip, newclient->port);
        printf("Me %u port %u\n", ntohl(ipbinary), ntohs(portnet));
        if (newclient->ip == ntohl(ipbinary) && newclient->port == ntohs(portnet))
        {
            printf("Client identical\n");
            free(newclient);
            continue;
        }
        else
        {
            printf("Inserting new client\n");
            pthread_mutex_lock(&circBuf->listlock);
            InsertNode(clientList, newclient);
            pthread_mutex_unlock(&circBuf->listlock);
            object_to_insert.ip = newclient->ip;
            object_to_insert.port = newclient->port;
            object_to_insert.pathname[0] = '\0';
            object_to_insert.version = 0;
            putitem(circBuf, object_to_insert);
            pthread_cond_signal(&circBuf->cond_nonempty);
        }
    }

    close(sock);
    sock = -1;
    // shutdown(sock, SHUT_WR);
    printf("Connecting again\n");
    // if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    //     perror_exit("socket");
    // if (connect(sock, serverptr, sizeof(server)) < 0)
    //     perror_exit("connect");

    // if (write(sock, "LOG_OFF", 8) < 0)
    //     perror("write");

    // //send the ip adress
    // if (write(sock, &ipbinary, 4) < 0)
    //     perror("write");

    // //send the port number
    // if (write(sock, &portnet, 2) < 0)
    //     perror("write");
    //create the worker threads

    //create the select interface for the client

    // //create the socket for the client

    // struct sockaddr_in cl_server,
    //     client;
    // struct sockaddr *cl_serverptr = (struct sockaddr *)&cl_server;
    // struct sockaddr *clientptr = (struct sockaddr *)&client;

    // int cl_sock;
    // /* Create socket */
    // if ((cl_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    //     perror_exit("socket");

    // cl_server.sin_family = AF_INET;                /* Internet domain */
    // cl_server.sin_addr.s_addr = htonl(INADDR_ANY); //watch out
    // cl_server.sin_port = htons(info->portNum);     /* The given port */

    // /* Bind socket to address */
    // if (bind(cl_sock, cl_serverptr, sizeof(cl_server)) < 0)
    //     perror_exit("bind");
    // /* Listen for connections */
    // if (listen(cl_sock, 15) < 0)
    //     perror_exit(" listen ");

    // fd_set active_fd_set, read_fd_set;
    // /* Initialize the set of active sockets. */
    // FD_ZERO(&active_fd_set);
    // FD_SET(cl_sock, &active_fd_set);
    // socklen_t size;

    while (1)
    {
        /* Block until input arrives on one or more active sockets. */
        read_fd_set = active_fd_set;
        // printf("Listening for connections to port %d \n", info->portNum);

        if (select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0 && errno != EINTR)
        {
            perror("select");
            exit(EXIT_FAILURE);
        }
        if (terminate)
        {
            break;
        }
        /* Service all the sockets with input pending. */
        for (i = 0; i < FD_SETSIZE; ++i)
            if (FD_ISSET(i, &read_fd_set))
            {
                if (i == cl_sock)
                {
                    /* Connection request on original socket. */
                    printf("EIIIII found a new connection!\n");
                    int new;
                    size = sizeof(clientptr);
                    new = accept(cl_sock,
                                 clientptr,
                                 &size);
                    if (new < 0)
                    {
                        perror("accept");
                        exit(EXIT_FAILURE);
                    }
                    fprintf(stderr,
                            "Server: connect from host %s, port %u.\n",
                            inet_ntoa(client.sin_addr),
                            ntohs(client.sin_port));
                    FD_SET(new, &active_fd_set);
                }
                else
                {
                    struct ip_port tempclient;
                    struct ip_port *newclient;
                    char first_char;
                    /* Data arriving on an already-connected socket. */
                    if (read(i, &first_char, 1) == 1)
                    {
                        int result = recogniseClientMessage(first_char, i);

                        switch (result)
                        {
                        case 0:
                            printf("Invalid command!");
                            break;
                        case 1:
                            newclient = malloc(sizeof(struct ip_port));
                            if (read(i, &newclient->ip, 4) != 4)
                            {
                                perror("read");
                                break;
                            }
                            //newclient->ip = htonl(newclient->ip);
                            newclient->ip = ntohl(newclient->ip);
                            if (read(i, &newclient->port, 2) != 2)
                            {
                                perror("read");
                                break;
                            }
                            //newclient->port = htons(newclient->port);
                            newclient->port = ntohs(newclient->port);
                            printf("\nBefore we insert %u/%u\n", ntohl(ipbinary), ntohs(portnet));
                            if (newclient->port == ntohs(portnet) && newclient->ip == ntohl(ipbinary))
                            {
                                printf("Same client!\n");
                                break;
                            }
                            printf("New client to insert ip: %u port %u\n", newclient->ip, newclient->port);

                            InsertNode(clientList, newclient);
                            object_to_insert.ip = newclient->ip;
                            object_to_insert.port = newclient->port;
                            object_to_insert.pathname[0] = '\0';
                            object_to_insert.version = 0;
                            putitem(circBuf, object_to_insert);
                            pthread_cond_signal(&circBuf->cond_nonempty);

                            printf("USER ON NODE INSERTED!\n");
                            break;
                        case 2:
                            if (read(i, &tempclient.ip, 4) != 4)
                            {
                                perror("read");
                                break;
                            }
                            tempclient.ip = ntohl(tempclient.ip);

                            if (read(i, &tempclient.port, 2) != 2)
                            {
                                perror("read");
                                break;
                            }
                            tempclient.port = ntohs(tempclient.port);
                            printf("USER_OFF from ip: %u and port: %u\n", tempclient.ip, tempclient.port);
                            printf("Local USER: %u/%u", ipbinary, portnet);
                            if (tempclient.port == ntohs(portnet) && tempclient.ip == ntohl(ipbinary))
                            {
                                printf("Same client!\n");
                                break;
                            }
                            pthread_mutex_lock(&circBuf->listlock);
                            if (DeleteNode(clientList, &tempclient))
                            {
                                printf("Client deleted successfully!\n");
                            }
                            else
                            {
                                printf("Client not found!\n");
                            }
                            pthread_mutex_unlock(&circBuf->listlock);
                            break;
                        case 3:
                            //Case for the GET_CLIENT_LIST command
                            if (write(i, "FILE_LIST", 10) < 0)
                            {
                                perror("writting file list:");
                            }
                            //get the number of files in the directory
                            char cwd[1024];
                            getcwd(cwd, sizeof(cwd)); //get the working directory
                            FILE *filePtr = fopen(cwd, "r");
                            if (filePtr == NULL)
                            {
                                printf("\n\nError opening file!!\n\n");
                            }
                            int fileCount = 0;
                            countFiles(info->dirName, filePtr, &fileCount);
                            //convert file count to string
                            char FileCounter[15];
                            sprintf(FileCounter, "%d", fileCount);
                            if (write(i, FileCounter, strlen(FileCounter) + 1) < 0)
                            {
                                perror("Error sending file Count:");
                            }
                            listdir(info->dirName, i, filePtr);
                            close(i);
                            break;
                        case 4:
                        { //read the pathname
                            int j = 0;
                            char pathBuffer[128];
                            char readChar;
                            do
                            {
                                if (read(i, &readChar, 1) < 0)
                                {
                                    perror("Reading path:");
                                }
                                pathBuffer[j] = readChar;
                                j++;
                            } while (readChar != '\0');
                            //read the file version
                            j = 0;
                            char fileVersion[21];
                            do
                            {
                                if (read(i, &readChar, 1) < 0)
                                {
                                    perror("Reading path:");
                                }
                                fileVersion[j] = readChar;
                                j++;
                            } while (readChar != '\0');
                            long Receivedtimestamp;
                            Receivedtimestamp = atol(fileVersion);

                            //Check if the file exists
                            //create the path to file
                            char pathToFile[128];
                            strcpy(pathToFile, info->dirName);
                            strcat(pathToFile, pathBuffer);
                            printf("PATH TO THE FILE TO SEND: %s\n", pathToFile);
                            if (fileExists(pathToFile))
                            {
                                if (Receivedtimestamp >= getFileTime(pathToFile))
                                {
                                    if (write(i, "FILE_UP_TO_DATE", 16) < 0)
                                    {
                                        perror("Sending message");
                                    }
                                }
                                else
                                {
                                    if (write(i, "FILE_SIZE", 10) < 0)
                                    {
                                        perror("Sending message");
                                    }
                                    //send version of the file
                                    long fileversion = getFileTime(pathToFile);
                                    char num[25];
                                    sprintf(num, "%ld", fileversion);
                                    if (write(i, num, strlen(num) + 1) < 0)
                                    {
                                        perror("Can't write file length");
                                    }
                                    //send the size of the file
                                    int filesize = findSize(pathToFile);
                                    //convert it to string
                                    sprintf(num, "%d", filesize);
                                    if (write(i, num, strlen(num) + 1) < 0)
                                    {
                                        perror("Can't write file length");
                                    }
                                    //sending the file
                                    copyfile(pathToFile, i, 200);
                                }
                            }
                            else
                            {
                                if (write(i, "FILE_NOT_FOUND", 15) < 0)
                                {
                                    perror("Sending message");
                                }
                            }
                        }
                        }
                        close(i);
                        FD_CLR(i, &active_fd_set);
                        printf("FILE DESCRIPTOR CLEARED!\n");
                    }
                }
            }

        //Finished with the select function
    }

    printf("\nEXITING\n");
    destroyStruct(circBuf);
    free(info);
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        perror_exit("socket");
    if (connect(sock, serverptr, sizeof(server)) < 0)
        perror_exit("connect");

    if (write(sock, "LOG_OFF", 8) < 0)
        perror("write");

    //send the ip adress
    if (write(sock, &ipbinary, 4) < 0)
        perror("write");

    //send the port number
    if (write(sock, &portnet, 2) < 0)
        perror("write");
}

void perror_exit(char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}

void getHostIP(char *buffer)
{
    int fd;
    struct ifreq ifr;

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    ifr.ifr_addr.sa_family = AF_INET;

    snprintf(ifr.ifr_name, IFNAMSIZ, "wlp1s0");

    ioctl(fd, SIOCGIFADDR, &ifr);

    /* and more importantly */
    strcpy(buffer, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

    close(fd);
}

// void checkHostName(int hostname)
// {
//     if (hostname == -1)
//     {
//         perror("gethostname");
//         exit(1);
//     }
// }

// // Returns host information corresponding to host name
// void checkHostEntry(struct hostent *hostentry)
// {
//     if (hostentry == NULL)
//     {
//         perror("gethostbyname");
//         exit(1);
//     }
// }

// // Converts space-delimited IPv4 addresses
// // to dotted-decimal format
// void checkIPbuffer(char *IPbuffer)
// {
//     if (NULL == IPbuffer)
//     {
//         perror("inet_ntoa");
//         exit(1);
//     }
// }

// // Driver code
// void getIP(char *buffer)
// {
//     char hostbuffer[256];
//     char *IPbuffer;
//     struct hostent *host_entry;
//     int hostname;

//     // To retrieve hostname
//     hostname = gethostname(hostbuffer, sizeof(hostbuffer));
//     checkHostName(hostname);

//     // To retrieve host information
//     host_entry = gethostbyname(hostbuffer);
//     checkHostEntry(host_entry);

//     // To convert an Internet network
//     // address into ASCII string
//     IPbuffer = inet_ntoa(*((struct in_addr *)
//                                host_entry->h_addr_list[0]));

//     printf("Hostname: %s\n", hostbuffer);
//     printf("Host IP: %s", IPbuffer);

//     strcpy(buffer, IPbuffer);
// }
