#
# tp2 - Tiled Powers of 2
# Makefile for SCO OpenServer
# Copyright (C) 2015 Tim Hentenaar.
#
# This code is licenced under the Simplified BSD License.
# See the LICENSE file for details.
#
# Building this is as easy as:
#
# $ /usr/css/bin/make -f Makefile.sco
#
LIBS=-lcurses
RM=/bin/rm
CC=/usr/ccs/bin/cc

# For information about the various CFLAGS settings
# available in SCO's cc, see 'man cc' or
#
# http://osr507doc.sco.com/en/man/html.CP/cc.CP.html
#
CFLAGS=-O2 -b elf -w 3 -X c

# Objects to build
OBJS=src/game.o src/ui.o src/terminal.o src/main.o

#
# Targets
#
all: clean tp2

tp2: $(OBJS)
	@echo "  LD $@"
	@$(CC) -o $@ $(OBJS) $(LIBS)

clean:
	@$(RM) -f $(OBJS) tp2

.c.o:
	@echo "  CC $@"
	@$(CC) $(CFLAGS) -c -o $@ $<

