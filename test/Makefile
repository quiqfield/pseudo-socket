OBJS-s  =	server.o
OBJS-c  =	client.o
CFLAGS  =	-Wall -I../include
LDFLAGS =

all: server client

server: $(OBJS-s)
	$(CC) $(CFLAGS) $(LDFLAGS) -o server.out $(OBJS-s) $(LDLIBS)

client: $(OBJS-c)
	$(CC) $(CFLAGS) $(LDFLAGS) -o client.out $(OBJS-c) $(LDLIBS)

clean:
	rm -f *.out *.o

run: all
	./run.sh
