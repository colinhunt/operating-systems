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
LDLIBS = -lm

# regular variables used
executable = server_f
submit = submit_server_f.tar


$(executable): $(executable).o

$(executable).o: $(executable).c

tar:
	tar cvf $(submit) *.c *.h Makefile README

clean:
	-rm -f *.o $(executable) core *.out

linewidth:
	-wc -L Makefile README *.c *.h