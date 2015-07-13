#
# tp2 - Tiled Powers of 2
# Copyright (C) 2015 Tim Hentenaar.
#
# This code is licenced under the Simplified BSD License.
# See the LICENSE file for details.
#
LIBS=-lcurses
INSTALL := $(shell which install 2>/dev/null)
PREFIX ?= /usr

# We want SUSv2 compliant interfaces
CFLAGS=-O2 -D_XOPEN_SOURCE=500

# Gather the sources
SRCS := $(wildcard src/*.c)
HS := $(wildcard src/*.h)

# Detect the compiler type and features
include compiler.mk

ifeq ($(origin INDENT), undefined)
INDENT := $(shell which indent 2>/dev/null)
endif

# .c to .o
OBJS = ${SRCS:.c=.o}

#
# Targets
#
tp2: $(OBJS)
	@echo "  LD $@"
	@$(CC) -o $@ $^ $(LIBS)

all: tp2

install: tp2
	@echo " INSTALL tp2 -> $(PREFIX)/bin/tp2"
	@mkdir -p $(PREFIX)/bin
	@$(INSTALL) -s -m0755 tp2 $(PREFIX)/bin/tp2

uninstall:
	@echo " UNINSTALL tp2"
	@$(RM) -f $(PREFIX)/bin/tp2

clean:
	@$(RM) -f $(OBJS) tp2

indent:
ifneq (,$(INDENT))
	@echo "  INDENT src/*.[ch]"
	@VERSION_CONTROL=none $(INDENT) $(SRCS) $(HS)
else
	@echo "'indent' not found."
endif

.c.o:
	@echo "  CC $@"
	@$(CC) $(CFLAGS) -c -o $@ $<

.SUFFIXES: .c .o
.PHONY: all install uninstall clean indent

