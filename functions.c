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