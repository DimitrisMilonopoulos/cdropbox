#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "list.h"
#include "info.h"
#include "circular_buffer.h"

struct circular_buffer *InitBuffer(int BufferSize, struct HeadNode *List_clients)
{
    struct circular_buffer *newbuffer = malloc(sizeof(struct circular_buffer));
    newbuffer->buffer = malloc(sizeof(struct BufferObject) * BufferSize);
    newbuffer->bufin = 0;
    newbuffer->bufout = 0;
    newbuffer->BufferSize = BufferSize;
    pthread_mutex_init(&newbuffer->bufferlock, 0);
    pthread_mutex_init(&newbuffer->listlock, 0);
    pthread_cond_init(&newbuffer->cond_nonempty, 0);
    pthread_cond_init(&newbuffer->cond_nonfull, 0);
    newbuffer->client_list = List_clients;
    for (int i = 0; i < BufferSize; i++)
    {
        newbuffer->buffer[i].ip = 0;
        newbuffer->buffer[i].port = 0;
        newbuffer->buffer[i].pathname[0] = '\0';
        newbuffer->buffer[i].version = -1;
    }
    return newbuffer;
}

int getitem(struct circular_buffer *bufferStruct, struct BufferObject *object)
{

    int error;
    int erroritem = 0;

    if ((error = pthread_mutex_lock(&bufferStruct->bufferlock)))
        /*No mutex, give up*/
        return error;

    while (bufferStruct->totalitems <= 0)
    {
        printf(">>Found Buffer Empty\n");
        pthread_cond_wait(&bufferStruct->cond_nonempty, &bufferStruct->bufferlock);
    }

    if (bufferStruct->totalitems > 0)
    {
        /*Buffer has somethings to remove*/
        *object = bufferStruct->buffer[bufferStruct->bufout];
        bufferStruct->bufout = (bufferStruct->bufout + 1) % bufferStruct->BufferSize;
        bufferStruct->totalitems--;
    }
    else
    {
        erroritem = EAGAIN;
    }

    if ((error = pthread_mutex_unlock(&bufferStruct->bufferlock)))
    {
        return error;
    }
    return erroritem;
}

int putitem(struct circular_buffer *bufferStruct, struct BufferObject item)
{
    int error;
    int erroritem = 0;
    if ((error = pthread_mutex_lock(&bufferStruct->bufferlock)))
        return error;

    while (bufferStruct->totalitems >= bufferStruct->BufferSize)
    {
        printf("Found Buffer Full\n");
        pthread_cond_wait(&bufferStruct->cond_nonfull, &bufferStruct->bufferlock);
    }
    if (bufferStruct->totalitems < bufferStruct->BufferSize)
    {
        bufferStruct->buffer[bufferStruct->bufin] = item;
        bufferStruct->bufin = (bufferStruct->bufin + 1) % bufferStruct->BufferSize;
        bufferStruct->totalitems++;
    }
    else
    {
        erroritem = EAGAIN;
    }

    if ((error = pthread_mutex_unlock(&bufferStruct->bufferlock)))
        return error;

    return erroritem;
}

int destroyStruct(struct circular_buffer *mystruct)
{
    pthread_mutex_destroy(&mystruct->listlock);
    pthread_mutex_destroy(&mystruct->bufferlock);
    pthread_cond_destroy(&mystruct->cond_nonempty);
    pthread_cond_destroy(&mystruct->cond_nonfull);
    deleteList(mystruct->client_list);
    free(mystruct->client_list);
    free(mystruct->buffer);
    free(mystruct);
}