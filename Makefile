# Copyright © 2004, 2005, 2006, 2008 Jakub Wilk <ubanus@users.sf.net>
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License, version 2, as published
# by the Free Software Foundation.
#
# Available directives:
#  . M_FANCY:   yes | no
#  . M_DEBUG:   yes | no
#  . M_NCURSES: yes | no

VERSION  = $(shell sed -n -e '1 s/.*(\([0-9.]*\)).*/\1/p' < debian/changelog)
M_FANCY    = yes
M_DEBUG    = no
M_NCURSES  = yes
M_COMPILER = gcc

HFILES = $(wildcard *.h)
CFILES = $(wildcard *.c)
OFILES = $(CFILES:.c=.o)

STRIP := strip -s
CTAGS := $(shell command -v ctags 2>/dev/null)
GCC := gcc
CC := $(GCC)

LDFLAGS := -lm
CFLAGS = $(CFLAGS_std) $(CFLAGS_wrn) $(CFLAGS_opt) $(CFLAGS_def)

CFLAGS_std := -std=gnu99
CFLAGS_wrn := -Wall -Wno-unused
CFLAGS_opt := -Os
CFLAGS_def := -DVERSION="\"$(VERSION)\""

ifeq ($(M_COMPILER),icc)
  CFLAGS_opt := -O3
endif

ifeq ($(M_DEBUG),yes)
  CFLAGS_def += -DDEBUG
  CFLAGS_opt := -g -O0
  STRIP := true
else
  CFLAGS_def += -DNDEBUG
endif

ifeq ($(M_FANCY),yes)
  CFLAGS_def += -DFANCY
endif
ifeq ($(M_NCURSES),yes)
  CFLAGS_def += -DHAVE_NCURSES
  LDFLAGS += -lncurses
endif

ifeq ($(M_COMPILER),ncc)
  STRIP := true
  CC := ncc -nc$(GCC) -ncoo -ncfabs
endif

ifeq ($(M_COMPILER),icc)
  CC := icc
  CFLAGS_std := -c99
  CFLAGS_wrn := -w
  CFLAGS_def += -D__ICC__
  LDFLAGS += -static-libcxa
endif

ifeq ($(M_COMPILER),tcc)
  CC := tcc
  CFLAGS_std :=
  CFLAGS_wrn := -Wall
endif

all: nonogram tags

tags: $(CFILES)
ifneq ($(CTAGS),)
	$(CTAGS) $(^)
endif

include Makefile.dep

$(OFILES): %.o: %.c
	$(CC) $(CFLAGS) -c $(<) -o $(@)

nonogram: $(OFILES)
	$(CC) $(LDFLAGS) $(CFLAGS) $(OFILES) -o $(@)
	$(STRIP) $(@)

mtest: nonogram
	./nonogram -m < test-input

test: nonogram
	./nonogram < test-input

stats:
	@echo $(shell cat *.c | wc -l) lines.
	@echo $(shell cat *.c | wc -c) bytes.

clean:
	$(RM) *.o nonogram nonogram-*.tar* tags doc/*.1

DB2MAN=/usr/share/xml/docbook/stylesheet/nwalsh/manpages/docbook.xsl
XMLLINT=/usr/bin/xmllint --valid --nonet
XSLTPROC=/usr/bin/xsltproc --nonet

doc/nonogram.1: doc/nonogram.xml
	sed -i -e "s/\(.*<!ENTITY version '\).*\('.*\)/\1$(VERSION)\2/" $(<)
	$(XMLLINT) $(<) > /dev/null
	$(XSLTPROC) --output $(@) $(DB2MAN) - < $(<)

.PHONY: all mtest test stats clean dist

# vim:ts=4 sw=4
