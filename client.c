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

#include "info.h"

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

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        perror_exit("socket");
    /* Find server address */
    server.sin_family = AF_INET;               /* Internet domain */
    server.sin_port = htons(info->serverPort); /* Server port */
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
    uint32_t ipbinary = htonl(myaddr.sin_addr.s_addr); //network compatible
    uint16_t portnet = htons(info->portNum);

    //send the log on message to server
    if (write(sock, "LOG_ON", 7) < 0)
        perror("write");

    //send the ip adress
    if (write(sock, &ipbinary, 4) < 0)
        perror("write");

    //send the port number
    if (write(sock, &portnet, 2) < 0)
        perror("write");

    // shutdown(sock, SHUT_WR);
    // if (connect(sock, serverptr, sizeof(server)) < 0)
    //     perror_exit("connect");

    // if (write(sock, "LOG_OFF", 8) < 0)
    //     perror("write");
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