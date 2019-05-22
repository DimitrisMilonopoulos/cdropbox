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

    port = 3002;
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

    if ((newsock = accept(sock, clientptr, &clientlen)) < 0)
        perror_exit("accept");
    printf("Accepted connection \n");
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