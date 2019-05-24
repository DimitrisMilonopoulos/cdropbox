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

int recogniseMessage(char *message, int newsock)
{
    //1: LOG_ON
    //2: GET_CLIENTS
    //3: LOG_OFF
    //0:not a valid message
    int amount;
    char *substring;
    char buffer[30];
    substring = copySubstring(message, 6);
    if (strcmp(substring, "LOG_ON") == 0)
    {
        free(substring);
        return 1;
    }
    free(substring);

    substring = copySubstring(message, 7);
    if (substring != NULL)
    {
        if (strcmp(substring, "GET_CLI") == 0)
        {
            if (read(newsock, &buffer, 5) == 5)
            {
                if (strcmp(buffer, "ENTS") == 0)
                {
                    free(substring);
                    return 2;
                }
            }
            free(substring);
            return 0;
        }
    }

    substring = copySubstring(message, 7);
    printf("DA SUBSTRING: %s\n", substring);
    if (substring != NULL)
    {
        if (strcmp(substring, "LOG_OFF") == 0)
        {
            free(substring);
            return 3;
        }
        free(substring);
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