#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "info.h"

struct client_info *read_client_args(int argc, char **argv)
{

    if (argc != 13)
    {
        fprintf(stderr, "Invalid number of arguments in client\n");
        return NULL;
    }

    struct client_info *clientInfo = malloc(sizeof(struct client_info));

    for (int i = 1; i < 13; i += 2)
    {

        if (strcmp(argv[i], "-d") == 0)
        {
            clientInfo->dirName = argv[i + 1];
        }
        else if (strcmp(argv[i], "-p") == 0)
        {
            clientInfo->portNum = atoi(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-w") == 0)
        {
            clientInfo->workerThreads = atoi(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-b") == 0)
        {
            clientInfo->bufferSize = atoi(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-sp") == 0)
        {
            clientInfo->serverPort = atoi(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-sip") == 0)
        {
            clientInfo->serverIP = argv[i + 1];
        }
        else
        {
            fprintf(stderr, "Argument is invalid %s\n", argv[i]);
            free(clientInfo);
            return NULL;
        }
    }

    return clientInfo;
}

void printClientInfo(struct client_info *info)
{
    if (info == NULL)
        return;

    printf("Dirname: %s\nportNum: %d\nworkerThreads: %d\nbufferSize: %d\nserverPort: %d\nserverIP: %s\n",
           info->dirName, info->portNum, info->workerThreads, info->bufferSize, info->serverPort, info->serverIP);
}