CC = gcc 
FLAGS = -g -c -lpthread
OBJS = client.o list.o info.o functions.o circular_buffer.o fileFunctions.o thread_func.o server.o

all : dropbox_client dropbox_server

dropbox_client: client.o list.o info.o functions.o circular_buffer.o fileFunctions.o thread_func.o
	$(CC)  client.o list.o info.o functions.o circular_buffer.o fileFunctions.o thread_func.o -lpthread -g -o dropbox_client

dropbox_server: server.o list.o info.o functions.o
	$(CC)  server.o list.o info.o functions.o -g -o dropbox_server

circular_buffer.o: circular_buffer.c
	$(CC) $(FLAGS) circular_buffer.c

client.o: client.c
	$(CC) $(FLAGS) client.c

fileFunctions.o: fileFunctions.c
	$(CC) $(FLAGS) fileFunctions.c 

functions.o: functions.c
	$(CC) $(FLAGS) functions.c

info.o: info.c
	$(CC) $(FLAGS) info.c

list.o: list.c
	$(CC) $(FLAGS) list.c

server.o: server.c
	$(CC) $(FLAGS) server.c

thread_func.o: thread_func.c
	$(CC) $(FLAGS) thread_func.c

clean:
	rm -f $(OBJS) dropbox_client dropbox_server