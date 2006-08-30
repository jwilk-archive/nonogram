/* Copyright (c) 2003, 2004, 2005 Jakub Wilk <ubanus@users.sf.net>
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include "stdbool.h"
#include <string.h>
#ifdef HAVE_NCURSES
#include <ncurses.h>
#include <term.h>
#include <unistd.h>
#endif

#include "config.h"
#include "term.h"

TermStrings term_strings;

#ifdef HAVE_NCURSES

static char get_acs(char ch)
{
  char *acsc = tigetstr("acsc");
  int i, len;
  if ((acsc == NULL || acsc == (char*)-1))
    return '\0';
  len = strlen(acsc);
  if ((len & 1) != 0)
    return '\0';
  len /= 2;
  for (i = 0; i < 2*len; i += 2)
  if (acsc[i] == ch)
    return acsc[i + 1];
  return '\0';
}

static void tput(char *str, int parm, char **cbuffer, unsigned int *n)
{
  int len;
  char *result = tigetstr(str);
  if (*cbuffer == NULL)
    return;
  if ((result == NULL || result == (char*)-1) && *n > 0)
  {
    **cbuffer = '\0';
    *n--;
    return;
  }
  if (parm != -1)
    result = tparm(result, parm);
  if ((len = strlen(result)) < *n)
  {
    strcpy(*cbuffer, result);
    *n -= len + 1;
    *cbuffer += len;
  }
  else
    *cbuffer = NULL;
}

#endif /* HAVE_NCURSES */

void setup_termstrings(bool use_defaults)
{
  term_strings.init = "";
  term_strings.hash = "##",
  term_strings.light[0] = "";
  term_strings.light[1] = "";
  term_strings.dark = "";
  term_strings.error = "";
  term_strings.v = "|";
  term_strings.h = "--";
  term_strings.tl = "+";
  term_strings.tr = "+";
  term_strings.bl = "+";
  term_strings.br = "+";

#ifdef HAVE_NCURSES
#define BUFFER_SIZE 4096
#define TBEGIN() do { sbuffer = cbuffer; } while (false)
#define TEND(s) do { s = sbuffer; cbuffer++; freebuf--; } while (false)
#define TPUT(s, parm) \
  do { \
    tput(s, parm, &cbuffer, &freebuf); \
    if (cbuffer == NULL) return; \
  } while (false)
#define TPUT_ACS(ch, dbl) \
  do { \
    if (freebuf < 1 + dbl) return; \
    cbuffer[0] = get_acs(ch); \
    if (cbuffer[0] == '\0') return; \
    if (dbl >= 2) cbuffer[1] = cbuffer[0]; \
    cbuffer[dbl] = '\0'; \
    cbuffer += dbl; \
    freebuf -= dbl; \
  } while (false);
  static char buffer[BUFFER_SIZE];
  char *sbuffer, *cbuffer = buffer;
  unsigned int freebuf = BUFFER_SIZE;
  int err;

  if (use_defaults)
    return;
  if (!isatty(STDOUT_FILENO))
    return;
  if (setupterm(NULL, STDOUT_FILENO, &err) == ERR)
    return;

  TBEGIN(); TPUT("enacs", -1);
  TEND(term_strings.init);
  if (config.color)
  {
    TBEGIN(); TPUT("sgr0", -1);
    TEND(term_strings.dark);
    TBEGIN(); TPUT("bold", -1); TPUT("setaf", 7); TPUT("setab", COLOR_CYAN);
    TEND(term_strings.light[0]);
    TBEGIN(); TPUT("bold", -1); TPUT("setaf", 7); TPUT("setab", COLOR_MAGENTA);
    TEND(term_strings.light[1]);
    TBEGIN(); TPUT("bold", -1); TPUT("setaf", 7); TPUT("setab", COLOR_RED);
    TEND(term_strings.error);
    TBEGIN(); TPUT("smacs", -1); TPUT_ACS('a', 2); TPUT("rmacs", -1);
    TEND(term_strings.hash);
  }
  else
  {
    TBEGIN(); TPUT("smacs", -1); TPUT_ACS('0', 2); TPUT("rmacs", -1);
    TEND(term_strings.hash);
  }
  TBEGIN(); TPUT("smacs", -1); TPUT_ACS('q', 2); TPUT("rmacs", -1);
  TEND(term_strings.h);
  TBEGIN(); TPUT("smacs", -1); TPUT_ACS('x', 1); TPUT("rmacs", -1);
  TEND(term_strings.v);
  TBEGIN(); TPUT("smacs", -1); TPUT_ACS('l', 1); TPUT("rmacs", -1);
  TEND(term_strings.tl);
  TBEGIN(); TPUT("smacs", -1); TPUT_ACS('k', 1); TPUT("rmacs", -1);
  TEND(term_strings.tr);
  TBEGIN(); TPUT("smacs", -1); TPUT_ACS('m', 1); TPUT("rmacs", -1);
  TEND(term_strings.bl);
  TBEGIN(); TPUT("smacs", -1); TPUT_ACS('j', 1); TPUT("rmacs", -1);
  TEND(term_strings.br);
#undef BUFFER_SIZE
#endif /* HAVE_NCURSES */
}

/* vim:set ts=2 sw=2 et: */
