#ifndef _INFO_
#define _INFO_
#include <stdint.h>
struct client_info
{
    char *dirName;
    char *serverIP;
    uint32_t portNum;
    uint16_t workerThreads;
    uint32_t bufferSize;
    uint32_t serverPort;
};

void printClientInfo(struct client_info *info);
struct client_info *read_client_args(int argc, char **argv);
#endif