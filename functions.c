#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include <sys/wait.h>   /* sockets */
#include <sys/types.h>  /* sockets */
#include <sys/socket.h> /* sockets */
#include <netinet/in.h> /* internet sockets */
#include <netdb.h>      /* ge th os tb ya dd r */
#include <unistd.h>     /* fork */
#include <stdlib.h>     /* exit */
#include <ctype.h>      /* toupper */

/*For the IP*/
#include <unistd.h>

#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>

#include "info.h"
#include "functions.h"

char *copySubstring(char *string, int length)
{
    //returns a substring of the original string (newly allocated)

    char *newstring = malloc(sizeof(char) * (length + 1));

    for (int i = 0; i < length; i++)
    {
        newstring[i] = string[i];
    }
    newstring[length] = '\0';
    return newstring;
}

int recogniseMessage(char first_char, int newsock)
{
    //1: LOG_ON
    //2: GET_CLIENTS
    //3: LOG_OFF
    //0:not a valid message
    int amount;
    char *substring;
    char message[30];

    //read the whole message
    message[0] = first_char;
    int i = 1;
    char character;
    do
    {
        if (read(newsock, &character, 1) != 1)
        {
            perror("read");
            printf("Characters read: %d\n", amount);
        }

        message[i] = character;
        i++;
    } while (character != '\0');
    if (strcmp(message, "LOG_ON") == 0)
    {
        return 1;
    }
    else if (strcmp(message, "LOG_OFF") == 0)
    {
        return 3;
    }
    else if (strcmp(message, "GET_CLIENTS") == 0)
    {
        return 2;
    }
    else
    {
        return 0;
    }
}

uint32_t copyIP(char *buffer)
{
    uint32_t binaryIP;

    memcpy(&binaryIP, buffer, 4);
    return binaryIP;
}

uint32_t copyPort(char *buffer)
{
    uint32_t binaryPort;
    memcpy(&binaryPort, buffer, 2);
    return binaryPort;
}

int recogniseClientMessage(char first_char, int newsock)
{
    //1: LOG_ON
    //2: GET_CLIENTS
    //3: LOG_OFF
    //0:not a valid message
    int amount;
    char *substring;
    char message[30];

    //read the whole message
    message[0] = first_char;
    int i = 1;
    char character;
    do
    {
        if (read(newsock, &character, 1) != 1)
        {
            perror("read");
            printf("Characters read: %d\n", amount);
        }

        message[i] = character;
        i++;
    } while (character != '\0');

    if (strcmp(message, "USER_ON") == 0)
    {
        return 1;
    }
    else if (strcmp(message, "USER_OFF") == 0)
    {
        return 2;
    }
    else if (strcmp(message, "GET_FILE_LIST") == 0)
    {
        return 3;
    }
    else if (strcmp(message, "GET_FILE") == 0)
    {
        return 4;
    }
    else
    {
        fprintf(stderr, "ERROR: invalid command from client!");
        return 0;
    }
}

//for 64 bit numbers sent via network

uint64_t
ntoh64(const uint64_t *input)
{
    uint64_t rval;
    uint8_t *data = (uint8_t *)&rval;

    data[0] = *input >> 56;
    data[1] = *input >> 48;
    data[2] = *input >> 40;
    data[3] = *input >> 32;
    data[4] = *input >> 24;
    data[5] = *input >> 16;
    data[6] = *input >> 8;
    data[7] = *input >> 0;

    return rval;
}

uint64_t
hton64(const uint64_t *input)
{
    return (ntoh64(input));
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
void getIP(char *buffer, char *myhostname)
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
    strcpy(myhostname, hostbuffer);
}