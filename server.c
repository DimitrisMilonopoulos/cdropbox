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

/*For the IP*/
#include <unistd.h>

#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>

#include "info.h"
#include "list.h"
#include "functions.h"
void perror_exit(char *message);
void getIP(char *buffer);

int main(int argc, char **argv)
{
    char buffer[30];
    getIP(buffer);
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

    //create list to save the connections
    struct HeadNode *connectionList = createQueue();

    if ((newsock = accept(sock, clientptr, &clientlen)) < 0)
        perror_exit("accept");

    char message_buffer[256];
    int amount;
    if ((amount = read(newsock, message_buffer, 7)) == 7)
    {
        printf("The message received from the client of length: %d\n", amount);
        int type = recogniseMessage(message_buffer, newsock);

        switch (type)
        {
        case 0:
            fprintf(stderr, "Invalid command received from client");
            break;
        case 1:
            printf("LOG_ON command received: initialising the new client\n");
            //first 7 characters of the buffer contain the log on string,
            //the other 6 contain the ip adress and the port in binary form
            uint32_t client_ip;
            if ((amount = read(newsock, &client_ip, 4)) != 4)
            {
                printf("FATAL ERROR!\n");
                break;
            }
            uint32_t client_port;
            if ((amount = read(newsock, &client_port, 2)) != 2)
            {
                printf("FATAL ERROR!\n");
                break;
            }
            client_ip = ntohl(client_ip);
            client_port = ntohs(client_port);
            printf("ip: %X\nport: %X\n", client_ip, client_port);
            struct ip_port *entry = malloc(sizeof(struct ip_port));
            entry->ip = client_ip;
            entry->port = client_port;
            InsertNode(connectionList, entry);
            break;
        case 2:
            break;
        case 3:
            break;
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

void checkHostName(int hostname)
{
    if (hostname == -1)
    {
        perror("gethostname");
        exit(1);
    }
}

// Returns host information corresponding to host name
void checkHostEntry(struct hostent *hostentry)
{
    if (hostentry == NULL)
    {
        perror("gethostbyname");
        exit(1);
    }
}

// Converts space-delimited IPv4 addresses
// to dotted-decimal format
void checkIPbuffer(char *IPbuffer)
{
    if (NULL == IPbuffer)
    {
        perror("inet_ntoa");
        exit(1);
    }
}
// Driver code
void getIP(char *buffer)
{
    char hostbuffer[256];
    char *IPbuffer;
    struct hostent *host_entry;
    int hostname;

    // To retrieve hostname
    hostname = gethostname(hostbuffer, sizeof(hostbuffer));
    checkHostName(hostname);

    // To retrieve host information
    host_entry = gethostbyname(hostbuffer);
    checkHostEntry(host_entry);

    // To convert an Internet network
    // address into ASCII string
    IPbuffer = inet_ntoa(*((struct in_addr *)
                               host_entry->h_addr_list[0]));

    printf("Hostname: %s\n", hostbuffer);
    printf("Host IP: %s", IPbuffer);

    strcpy(buffer, IPbuffer);
}