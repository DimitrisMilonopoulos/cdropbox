#include <stdio.h>
#include <sys/types.h>  /* sockets */
#include <sys/socket.h> /* sockets */
#include <netinet/in.h> /* internet sockets */
#include <unistd.h>     /* read, write, close */
#include <netdb.h>      /* gethostbyaddr */
#include <stdlib.h>     /* exit */
#include <string.h>     /* strlen */

/*For the IP*/
#include <unistd.h>

#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>

#include <sys/select.h> /*For the select command*/

#include "info.h"
#include "list.h"

void perror_exit(char *message);
void getHostIP(char *);
void getIP(char *buffer);

int main(int argc, char **argv)
{
    char ip_buffer[30];
    getIP(ip_buffer);
    printf("Found your ip adress to be: %s\n", ip_buffer);
    struct client_info *info = read_client_args(argc, argv);
    printClientInfo(info);

    int port, sock, i;
    char buf[256];

    struct sockaddr_in server;
    struct sockaddr *serverptr = (struct sockaddr *)&server;
    struct hostent *rem;

    //create the list for the fellow clients
    struct HeadNode *clientList = createQueue();
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
    if (connect(sock, serverptr, sizeof(server)) < 0)
        perror_exit("connect");

    /*Send log on message to server*/

    //convert the ip adress and socket to binary form for transfer
    struct sockaddr_in myaddr;
    inet_aton(ip_buffer, &myaddr.sin_addr);
    uint32_t ipbinary = htonl(myaddr.sin_addr.s_addr); //htonl(myaddr.sin_addr.s_addr); //network compatible
    uint16_t portnet = htons(info->portNum);           //htons(info->portNum);

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

    //send the log on message to server
    if (write(sock, "GET_CLIENTS", 12) < 0)
        perror("write");
    //send the ip adress
    if (write(sock, &ipbinary, 4) < 0)
        perror("write");

    //send the port number
    if (write(sock, &portnet, 2) < 0)
        perror("write");
    close(sock);
    //read the response from the server
    //read the number of clients the server is going to send
    uint32_t number_of_clients;
    if (read(sock, &number_of_clients, 4) != 4)
    {
        perror("read");
    }
    number_of_clients = ntohl(number_of_clients);
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
        printf("New client %d new port %d\n", newclient->ip, newclient->port);
        printf("Me %d port %d\n", ntohl(ipbinary), ntohs(portnet));
        if (newclient->ip == ntohl(ipbinary) && newclient->port == ntohs(portnet))
        {
            printf("Client identical\n");
            free(newclient);
            continue;
        }
        else
        {
            printf("Inserting new client\n");
            InsertNode(clientList, newclient);
        }
    }

    close(sock);
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

    //create the select interface for the client

    //create the socket for the client

    struct sockaddr_in cl_server, client;
    struct sockaddr *cl_serverptr = (struct sockaddr *)&cl_server;
    struct sockaddr *clientptr = (struct sockaddr *)&client;
    struct hostent *rem;

    int cl_sock;
    /* Create socket */
    if ((cl_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        perror_exit("socket");

    cl_server.sin_family = AF_INET; /* Internet domain */
    cl_server.sin_addr.s_addr = htonl(INADDR_ANY);
    cl_server.sin_port = htons(info->portNum); /* The given port */

    /* Bind socket to address */
    if (bind(cl_sock, cl_serverptr, sizeof(cl_server)) < 0)
        perror_exit("bind");
    /* Listen for connections */
    if (listen(cl_sock, 15) < 0)
        perror_exit(" listen ");
    printf("Listening for connections to port %d \n", info->portNum);

    fd_set active_fd_set, read_fd_set;
    /* Initialize the set of active sockets. */
    FD_ZERO(&active_fd_set);
    FD_SET(cl_sock, &active_fd_set);
    int size;

    while (1)
    {
        /* Block until input arrives on one or more active sockets. */
        read_fd_set = active_fd_set;
        if (select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0)
        {
            perror("select");
            exit(EXIT_FAILURE);
        }

        /* Service all the sockets with input pending. */
        for (i = 0; i < FD_SETSIZE; ++i)
            if (FD_ISSET(i, &read_fd_set))
            {
                if (i == cl_sock)
                {
                    /* Connection request on original socket. */
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
                            "Server: connect from host %s, port %hd.\n",
                            inet_ntoa(client.sin_addr),
                            ntohs(client.sin_port));
                    FD_SET(new, &active_fd_set);
                }
                else
                {
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
                        case 1: //user on command
                            struct ip_port *newclient = malloc(sizeof(struct ip_port));
                            if (read(i, newclient->ip, 4) != 4)
                            {
                                perror('read');
                                break;
                            }
                            newclient->ip = htonl(newclient->ip);

                            if (read(i, newclient->ip, 2) != 2)
                            {
                                perror('read');
                                break;
                            }
                            newclient->port = htons(newclient->port);
                            if (newclient->port == htons(portnet) && newclient->ip == htonl(ipbinary))
                            {
                                printf("Same client!\n");
                                break;
                            }
                            InsertNode(clientList, newclient);
                            break;
                        case 2:
                            struct ip_port tempclient;
                            if (read(i, tempclient.ip, 4) != 4)
                            {
                                perror('read');
                                break;
                            }
                            tempclient.ip = htonl(tempclient.ip);

                            if (read(i, tempclient.port, 2) != 2)
                            {
                                perror('read');
                                break;
                            }
                            tempclient.port = htons(tempclient.port);
                            if (tempclient.port == htons(portnet) && tempclient.ip == htonl(ipbinary))
                            {
                                printf("Same client!\n");
                                break;
                            }
                            if (DeleteNode(clientList, &tempclient))
                            {
                                printf("Client deleted successfully!\n");
                            }
                            else
                            {
                                printf("Client not found!\n");
                            }
                            break;
                        case 3:
                            break;
                        case 4:
                            break;
                        }
                    }
                }
            }
    }
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