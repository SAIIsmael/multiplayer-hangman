TARGETCLIENT	=client
TARGETSERVER =server

CXX	=	gcc

LDFLAGS	=	-Wall -g -lpthread

SRC_CLIENT	=	 ../src/tcpClient.c

SRC_SERVER = ../src/functions.c \
						 ../src/tcpServer.c

CXXFLAGS= -ansi -pedantic -I./inc

OBJS_CLIENT	=	$(SRC_CLIENT:.c=.o)
OBJS_SERVER	=	$(SRC_SERVER:.c=.o)

all: client server

client : $(OBJS_CLIENT)
	$(CXX) -o $(TARGETCLIENT) $(OBJS_CLIENT) $(LDFLAGS)

server : $(OBJS_SERVER)
	$(CXX) -o $(TARGETSERVER) $(OBJS_SERVER) $(LDFLAGS)

fclean: clean
	rm -f $(OBJS_SERVER) $(OBJS_CLIENT)
clean:
	rm -f $(TARGETCLIENT) $(TARGETSERVER)
	rm -f ../src/*.o
re: fclean all
