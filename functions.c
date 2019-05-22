#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "info.h"
#include "functions.h"

char *copySubstring(char *string, int length)
{
    //returns a substring of the original string (newly allocated)
    if (strlen(string) < length)
    {
        printf("Input length bigger than strings length");
        return NULL;
    }

    char *newstring = malloc(sizeof(char) * (length + 1));

    for (int i = 0; i < length; i++)
    {
        newstring[i] = string[i];
    }
    newstring[length] = '\0';
    return newstring;
}

int recogniseMessage(char *message)
{
    //1: LOG_ON
    //2: GET_CLIENTS
    //3: LOG_OFF
    //0:not a valid message
    if (strlen(message) < 6)
    {
        return 0;
    }
    char *substring;
    substring = copySubstring(message, 6);
    if (strcmp(substring, "LOG_ON") == 0)
    {
        free(substring);
        return 1;
    }
    free(substring);

    substring = copySubstring(message, 11);
    if (substring != NULL)
    {
        if (strcmp(substring, "GET_CLIENTS") == 0)
        {
            free(substring);
            return 2;
        }
        free(substring);
    }

    substring = copySubstring(message, 7);
    if (substring != NULL)
    {
        if (strcmp(substring, "LOG_OFF") == 0)
        {
            free(substring);
            return 3;
        }
        free(substring);
    }
}