# name:               Colin Hunt
# ONE Card number:    1222665
# Unix id:            colin
# lecture section:    A1
# instructor's name:  Mohammad Bhuiyan
# lab section:        D05
#
# 'make clean' will rm all object files, the executable, a core file, and any
#   output files ending with .out
# 'make tar' will create the tar file for submission
# 'make linewidth' will report the linewidths of some submission files 

# built-in variables
CC = gcc
CFLAGS = -Wall
LDLIBS = -lm -lpthread

# regular variables used
submit = submit_server.tar
object_files = request_handler.o server.o

all: server_f server_p

server_f: $(object_files) forked_handler.o
	$(CC) -o server_f $(object_files) forked_handler.o -lpthread -Wall

server_p: $(object_files) threaded_handler.o
	$(CC) -o server_p $(object_files) threaded_handler.o -lpthread -Wall

server.o: server.c

forked_handler.o: forked_handler.c concurrent_handler.h request_handler.h

threaded_handler.o: threaded_handler.c concurrent_handler.h request_handler.h

request_handler.o: request_handler.c request_handler.h

tar:
	mkdir colin_hunt
	cp *.c *.h Makefile colin_hunt
	tar cvf $(submit) colin_hunt

clean:
	rm -rf *.o server_* core *.out *.tar colin_hunt test

linewidth:
	-wc -L Makefile README *.c *.h