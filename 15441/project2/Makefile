################################################################################
# Makefile                                                                     #
#                                                                              #
# Description: This file contains the make rules for Recitation 1.             #
#                                                                              #
# Authors: Athula Balachandran <abalacha@cs.cmu.edu>,                          #
#          Wolf Richter <wolf@cs.cmu.edu>                                      #
#                                                                              #
################################################################################
IDIR=./include
ODIR=./obj
CFLAG=-I$(IDIR)
CC=gcc
default: lisod
_dep = log.h hio.h
dep=$(patsubst %, $(IDIR)/%, $(_dep))
_objects = log.o hio.o lisod.o
objects=$(patsubst %, $(ODIR)/%, $(_objects))
lisod: $(objects)
	$(CC) -o $@ $^

$(ODIR)/%.o: %.c $(dep)
	$(CC) -c $(CFLAG) -o $@ $<

echo_server:
	@gcc echo_server.c -o echo_server -Wall -Werror $(CFLAG)

echo_client:
	@gcc hio.c echo_client.c -o echo_client -Wall -Werror $(CFLAG)

clean:
	@rm -f $(ODIR)/*.o
