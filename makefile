SERVER	= server
CLIENT	= client

SSRC	= MTserver.c
CSRC	= client.c

all: $(CLIENT) $(SERVER)

$(SERVER): $(SERVER).o helper.o
	gcc -o $(SERVER) $(SERVER).o helper.o

$(SERVER).o: $(SSRC) helper.h
	gcc -o $(SERVER).o $(SSRC) -c

$(CLIENT): $(CLIENT).o helper.o
	gcc -o $(CLIENT) $(CLIENT).o helper.o

$(CLIENT).o: $(CSRC) helper.h
	gcc -o $(CLIENT).o $(CSRC) -c

helper.o: helper.c helper.h
	gcc -o helper.o helper.c -c
clean:
	rm -f *.o
	rm -f $(CLIENT)
	rm -f $(SERVER)

