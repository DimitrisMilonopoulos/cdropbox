#include <stdio.h>
#include <sys/types.h>  /* sockets */
#include <sys/socket.h> /* sockets */
#include <netinet/in.h> /* internet sockets */
#include <unistd.h>     /* read, write, close */
#include <netdb.h>      /* gethostbyaddr */
#include <stdlib.h>     /* exit */
#include <string.h>     /* strlen */
#include <signal.h>
#include <pthread.h>

/*For the IP*/
#include <unistd.h>

#include <sys/ioctl.h>
#include <net/if.h>
#include <errno.h>
#include <arpa/inet.h>

#include <sys/select.h> /*For the select command*/

#include "info.h"
#include "circular_buffer.h"
#include "list.h"
#include "functions.h"
#include "thread_func.h"

void threadFunc(struct circular_buffer *arg_struct)
{
    printf("Hallo i am a lovely thread\n");
    printf("The buffer size is: %d\n", arg_struct->BufferSize);
}