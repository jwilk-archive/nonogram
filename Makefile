# Copyright © 2004-2010 Jakub Wilk <jwilk@jwilk.net>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the “Software”), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# Available directives:
#  . M_FANCY:   yes | no
#  . M_DEBUG:   yes | no
#  . M_NCURSES: yes | no

VERSION  = $(shell sed -n -e '1 s/.*(\([0-9.]*\)).*/\1/p' < doc/changelog)
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

XSL = http://docbook.sourceforge.net/release/xsl/current/manpages/docbook.xsl
XMLLINT = /usr/bin/xmllint --valid --nonet
XSLTPROC = /usr/bin/xsltproc --nonet

doc/nonogram.1: doc/nonogram.xml
	sed -i -e "s/\(.*<!ENTITY version '\).*\('.*\)/\1$(VERSION)\2/" $(<)
	$(XMLLINT) $(<) > /dev/null
	$(XSLTPROC) --output $(@) $(XSL) - < $(<)

.PHONY: all mtest test stats clean dist

# vim:ts=4 sw=4
