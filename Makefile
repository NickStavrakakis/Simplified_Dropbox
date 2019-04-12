OUT = mirror_client
OBJS = $(OUT) sender receiver deleter
SOURCE = mirror_client.c sender.c receiver.c deleter.c functions.c
HEADER = functions.h structs.h
CC = gcc
FLAGS= -o
CLEAR = clear_screen

all: $(CLEAR) $(OBJS)

clear_screen:
	clear

$(OUT): mirror_client.c
	$(CC) mirror_client.c functions.c $(FLAGS) $(OUT)

sender: sender.c
	$(CC) sender.c functions.c $(FLAGS) sender

receiver: receiver.c
	$(CC) receiver.c functions.c $(FLAGS) receiver

deleter: deleter.c
	$(CC) deleter.c functions.c $(FLAGS) deleter

clean:
	rm -f $(OBJS) $(OUT)
