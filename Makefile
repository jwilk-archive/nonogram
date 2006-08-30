# Available directives:
#  . M_VERSION
#  . M_CHROME: yes | no
#  . M_DEBUG: yes | no
#  . M_NCURSES : yes | no

M_VERSION  = 0.8.1
M_CHROME   = no
M_DEBUG    = no
M_NCURSES  = yes
M_COMPILER = gcc

HFILES  = $(wildcard *.h)
CFILES = $(wildcard *.c)
OFILES = $(CFILES:.c=.o)

STRIP := strip -s
FAKEROOT := $(shell command -v fakeroot 2>/dev/null)
CTAGS := $(shell command -v ctags 2>/dev/null)
GCC := gcc
CC := $(GCC)

LDFLAGS := -lm
CFLAGS = $(CFLAGS_std) $(CFLAGS_wrn) $(CFLAGS_opt) $(CFLAGS_def)

CFLAGS_std := -std=gnu99
CFLAGS_wrn := -Wall -Wno-unused
CFLAGS_opt := -Os
CFLAGS_def := -DVERSION="\"$(M_VERSION)\""

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

ifeq ($(M_CHROME),yes)
  CFLAGS_def += -DCHROME
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
	ctags $(^)
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

DB2MAN=/usr/share/sgml/docbook/stylesheet/xsl/nwalsh/manpages/docbook.xsl
XMLLINT=/usr/bin/xmllint --valid
XSLTPROC=/usr/bin/xsltproc --nonet

doc/nonogram.1: doc/nonogram.xml
	$(XMLLINT) $(<) > /dev/null
	cd doc && $(XSLTPROC) $(DB2MAN) ../$(<)

.PHONY: all mtest test stats clean dist

# vim:ts=4 sw=4

# vim:ts=4
