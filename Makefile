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
#  . M_DEBUG:   yes | no
#  . M_NCURSES: yes | no

VERSION  = $(shell sed -n -e '1 s/.*(\([0-9.]*\)).*/\1/p' < doc/changelog)
M_DEBUG    = no
M_NCURSES  = yes

CFILES = $(wildcard *.c)
OFILES = $(CFILES:.c=.o)

LDLIBS = -lm
CFLAGS = $(CFLAGS_std) $(CFLAGS_wrn) $(CFLAGS_opt) $(CFLAGS_def)

CFLAGS_std := -std=gnu99
CFLAGS_wrn := -Wall -Wno-unused
CFLAGS_opt := -O2
CFLAGS_def := -DVERSION="\"$(VERSION)\""

ifeq ($(M_DEBUG),yes)
  CFLAGS_def += -DDEBUG
endif

ifeq ($(M_NCURSES),yes)
  CFLAGS_def += -DHAVE_NCURSES
  LDLIBS += -lncurses
endif

.PHONY: all
all: nonogram

include Makefile.dep

$(OFILES): %.o: %.c
	$(CC) $(CFLAGS) -c $(<) -o $(@)

nonogram: $(OFILES)
	$(LINK.c) $(^) $(LOADLIBES) $(LDLIBS) -o $(@)

.PHONY: mtest
mtest: nonogram
	./nonogram -m < test-input

.PHONY: test
test: nonogram
	./nonogram < test-input

.PHONY: clean
clean:
	rm -f *.o nonogram doc/*.1

xsl = http://docbook.sourceforge.net/release/xsl/current/manpages/docbook.xsl

doc/nonogram.1: doc/nonogram.xml
	sed -i -e "s/\(.*<!ENTITY version '\).*\('.*\)/\1$(VERSION)\2/" $(<)
	xmllint --valid --nonet $(<) > /dev/null
	xsltproc --nonet --output $(@) $(xsl) - < $(<)

# vim:ts=4 sw=4
