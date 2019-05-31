#ifndef _FILE_FUNCTIONS_
#define _FILE_FUNCTIONS_
int newprunePath(char *string, char *newstring);
time_t getFileTime(char *path);
int countFiles(char *name, FILE *logPtr, int *Filecounter);
int listdir(char *name, int Socketfd, FILE *logPtr);
long int findSize(const char *file_name);
int createPath(char *path);
int copyfile(char *sourcefile, int outputptr, int BUFFSIZE);
int fdtoFile(char *outname, int indesc, int BUFFSIZE, int fileSize);
int fileExists(char *filename);
#endif