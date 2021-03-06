#include <stdio.h>

#include <sys/wait.h>   /* sockets */
#include <sys/types.h>  /* sockets */
#include <sys/socket.h> /* sockets */
#include <netinet/in.h> /* internet sockets */
#include <netdb.h>      /* ge th os tb ya dd r */
#include <unistd.h>     /* fork */
#include <stdlib.h>     /* exit */
#include <ctype.h>      /* toupper */
#include <string.h>
#include <sys/select.h>
#include <signal.h>
#include <errno.h>
/*For the IP*/
#include <unistd.h>

#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>

#include "info.h"
#include "list.h"
#include "functions.h"
void perror_exit(char *message);
// void getIP(char *buffer);
int terminate = 0;

void closeServer(int signum)
{
    terminate = 1;
}

int main(int argc, char **argv)
{
    char buffer[30];
    char hostname[60];
    getIP(buffer, hostname);
    printf("The IP is %s\n", buffer);
    int port, sock, newsock;
    struct sockaddr_in server, client;
    socklen_t clientlen;

    struct sockaddr *serverptr = (struct sockaddr *)&server;
    struct sockaddr *clientptr = (struct sockaddr *)&client;
    struct hostent *rem;
    if (strcmp(argv[1], "-p") != 0)
    {
        fprintf(stderr, "Invalid arguments, usage: ./server -p <portNum>\n");
        return -1;
    }
    port = atoi(argv[2]);
    /* Create socket */
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        perror_exit("socket");

    server.sin_family = AF_INET; /* Internet domain */
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port); /* The given port */

    /* Bind socket to address */
    if (bind(sock, serverptr, sizeof(server)) < 0)
        perror_exit("bind");
    /* Listen for connections */
    if (listen(sock, 15) < 0)
        perror_exit(" listen ");
    printf("Listening for connections to port %d \n", port);

    //initialize the interrupt
    struct sigaction toExit = {};

    toExit.sa_handler = closeServer;
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

    //create list to save the connections
    struct HeadNode *connectionList = createQueue();

    //set the multiple file descriptors
    fd_set active_fd_set, read_fd_set;
    /* Initialize the set of active sockets. */
    FD_ZERO(&active_fd_set);
    FD_SET(sock, &active_fd_set);
    char message_buffer[256];
    int amount;
    uint32_t client_ip;
    uint32_t client_port;
    while (1)
    {
        /* Block until input arrives on one or more active sockets. */
        read_fd_set = active_fd_set;
        if (select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0 && errno != EINTR)
        {
            perror("select");
            exit(EXIT_FAILURE);
        }
        if (terminate)
        {
            break;
        }
        for (int i = 0; i < FD_SETSIZE; ++i)
        {
            if (FD_ISSET(i, &read_fd_set))
            {

                if (i == sock)
                {
                    clientlen = sizeof(*clientptr);
                    if ((newsock = accept(sock, clientptr, &clientlen)) < 0)
                        perror_exit("accept");

                    fprintf(stderr,
                            "Server: connect from host %s, port %d.\n",
                            inet_ntoa(client.sin_addr),
                            client.sin_port);
                    FD_SET(newsock, &active_fd_set);
                }
                else
                {
                    char first_char;
                    if ((amount = read(i, &first_char, 1)) == 1)
                    {
                        int type = recogniseMessage(first_char, i);

                        switch (type)
                        {
                        case 0:
                            fprintf(stderr, "Invalid command received from client");
                            break;
                        case 1:
                            printf("LOG_ON command received: initialising the new client\n");
                            //first 7 characters of the buffer contain the log on string,
                            //the other 6 contain the ip adress and the port in binary form

                            if ((amount = read(i, &client_ip, 4)) != 4)
                            {
                                printf("FATAL ERROR!\n");
                                break;
                            }

                            if ((amount = read(i, &client_port, 2)) != 2)
                            {
                                printf("FATAL ERROR!\n");
                                break;
                            }
                            close(i);
                            client_ip = ntohl(client_ip);
                            client_port = ntohs(client_port);
                            printf("ip: %u\nport: %u\n", client_ip, client_port);
                            struct ip_port *entry = malloc(sizeof(struct ip_port));
                            entry->ip = client_ip;
                            entry->port = client_port;
                            if (FindNode(connectionList, *entry))
                            {
                                printf("Client Already exists in the mainframe\n");
                                free(entry);
                            }
                            else
                            {
                                uint32_t temp_ip;
                                uint16_t temp_port;

                                printf("informing fellow clients about the log on\n");
                                struct Node *curr = connectionList->head;

                                struct sockaddr_in newclient;
                                struct sockaddr *newclientptr = (struct sockaddr *)&newclient;
                                printf("The client_ip is %u and client_port %u\n\n", client_ip, client_port);
                                client_ip = htonl(client_ip);
                                client_port = htons(client_port);
                                while (curr != NULL)
                                {
                                    printf("The IP VALUE TO CONNECT IS %u PORT: %u\n", curr->value->ip, curr->value->port);
                                    temp_ip = htonl(curr->value->ip);
                                    temp_port = htons(curr->value->port);
                                    int client_sock;
                                    if ((client_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
                                        perror_exit("socket");
                                    /* Find server address */
                                    newclient.sin_family = AF_INET; /* Internet domain */
                                    newclient.sin_port = temp_port; /* Server port:used htons */
                                    /* Initiate connection */
                                    struct in_addr temp_in = {};
                                    temp_in.s_addr = temp_ip;
                                    newclient.sin_addr = temp_in;
                                    if (connect(client_sock, newclientptr, sizeof(newclient)) < 0)
                                        perror_exit("connect");
                                    printf("Connected Successfully!\n");
                                    if (write(client_sock, "USER_ON", 8) != 8)
                                    {
                                        perror("write");
                                    }
                                    if (write(client_sock, &client_ip, 4) != 4)
                                    {
                                        perror("write");
                                    }
                                    if (write(client_sock, &client_port, 2) != 2)
                                    {
                                        perror("write");
                                    }
                                    close(client_sock);
                                    curr = curr->next;
                                }
                                InsertNode(connectionList, entry);
                                printf("New client added\n");
                            }

                            break;
                        case 2:
                            printf("Sending Client List to Client, clients: %d\n", connectionList->nitems);
                            //send files to the client
                            // uint32_t numberofClients = htonl(connectionList->nitems);

                            if ((amount = read(i, &client_ip, 4)) != 4)
                            {
                                printf("FATAL ERROR!\n");
                                break;
                            }

                            if ((amount = read(i, &client_port, 2)) != 2)
                            {
                                printf("FATAL ERROR!\n");
                                break;
                            }
                            client_ip = ntohl(client_ip);
                            client_port = ntohs(client_port);

                            if (write(i, "CLIENT_LIST", 12) != 12)
                            {
                                perror("write");
                            }

                            // if (write(i, &numberofClients, 4) < 0)
                            //     perror("write");
                            char numbuf[10];
                            sprintf(numbuf, "%d", connectionList->nitems);
                            if (write(i, numbuf, strlen(numbuf) + 1) < 0)
                                perror("write");
                            struct Node *curr = connectionList->head;
                            while (curr != NULL)
                            {
                                client_ip = htonl(curr->value->ip);
                                client_port = htons(curr->value->port);
                                printf("Sending Client IP: %u client port: %u\n", curr->value->ip, curr->value->port);
                                if (write(i, &client_ip, 4) < 0)
                                    perror("write");
                                if (write(i, &client_port, 2) < 0)
                                    perror("write");

                                curr = curr->next;
                            }
                            printf("Clients sent successfully!\n");

                            break;
                        case 3:
                            printf("\nClient requested log off from server.\n");
                            if ((amount = read(i, &client_ip, 4)) != 4)
                            {
                                printf("FATAL ERROR!\n");
                                break;
                            }
                            if ((amount = read(i, &client_port, 2)) != 2)
                            {
                                printf("FATAL ERROR!\n");
                                break;
                            }
                            client_ip = ntohl(client_ip);
                            client_port = ntohs(client_port);
                            struct ip_port temp;
                            temp.ip = client_ip;
                            temp.port = client_port;
                            client_ip = htonl(client_ip);
                            client_port = htons(client_port);
                            printf("IP: %u, port: %u\n", temp.ip, temp.port);
                            //printList(connectionList);
                            if (DeleteNode(connectionList, &temp))
                            {
                                printf("Client DELETED successfully\n");
                                uint32_t temp_ip;
                                uint16_t temp_port;

                                printf("informing fellow clients about the log off\n");
                                struct Node *curr = connectionList->head;
                                if (curr == NULL)
                                {
                                    printf("List is empty!\n");
                                }
                                while (curr != NULL)
                                {
                                    temp_ip = htonl(curr->value->ip);
                                    temp_port = htons(curr->value->port);
                                    printf("To inform IP: %u and port: %u\n", curr->value->ip, curr->value->port);
                                    struct sockaddr_in newclient;
                                    struct sockaddr *newclientptr = (struct sockaddr *)&newclient;
                                    int client_sock;
                                    if ((client_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
                                        perror_exit("socket");
                                    /* Find server address */
                                    newclient.sin_family = AF_INET; /* Internet domain */
                                    newclient.sin_port = temp_port; /* Server port:used htons */
                                    /* Initiate connection */
                                    newclient.sin_addr.s_addr = temp_ip;
                                    if (connect(client_sock, newclientptr, sizeof(newclient)) < 0)
                                        perror_exit("connect");

                                    if (write(client_sock, "USER_OFF", 9) != 9)
                                    {
                                        perror("write");
                                    }
                                    if (write(client_sock, &client_ip, 4) != 4)
                                    {
                                        perror("write");
                                    }
                                    if (write(client_sock, &client_port, 2) != 2)
                                    {
                                        perror("write");
                                    }
                                    close(client_sock);
                                    curr = curr->next;
                                }
                            }
                            else
                            {
                                printf("ERROR_IP_PORT_NOT_FOUND_IN_LIST\n");
                            }
                            break;
                        }
                    }
                    close(i);
                    FD_CLR(i, &active_fd_set);
                }
            }
        }
    }
    deleteList(connectionList);
    free(connectionList);
    close(newsock);
    return 0;
}

void perror_exit(char *message)
{
    perror(message);
    exit(EXIT_FAILURE);
}