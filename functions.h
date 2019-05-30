#ifndef _FUNCTIONS_
#define _FUNCTIONS_

char *copySubstring(char *, int);
int recogniseMessage(char, int newsock);
int recogniseClientMessage(char first_char, int newsock);
uint32_t copyIP(char *buffer);
uint32_t copyPort(char *buffer);
uint64_t ntoh64(const uint64_t *input);
uint64_t hton64(const uint64_t *input);
#endif