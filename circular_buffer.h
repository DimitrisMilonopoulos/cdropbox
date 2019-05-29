#ifndef _CIRCULAR_BUFFER_
#define _CIRCULAR_BUFFER_

struct circular_buffer
{
    struct BufferObject *buffer;
    int bufin;
    int bufout;
    int totalitems;
    int BufferSize;
    pthread_mutex_t bufferlock;
    pthread_cond_t cond_nonempty;
    pthread_cond_t cond_nonfull;
};

struct circular_buffer *InitBuffer(int);
int getitem(struct circular_buffer *bufferStruct, struct BufferObject *object);
int putitem(struct circular_buffer *bufferStruct, struct BufferObject item);
#endif