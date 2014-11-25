/* Copyright © 2003-2014 Jakub Wilk <jwilk@jwilk.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the “Software”), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "autoconfig.h"

#include <stdbool.h>
#include <string.h>

#ifdef HAVE_NCURSES
#include <ncurses.h>
#include <term.h>
#include <unistd.h>
#endif

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
  if (*n == 0)
  {
    *cbuffer = NULL;
    return;
  }
  if (result == NULL || result == (char*)-1)
  {
    **cbuffer = '\0';
    --*n;
    return;
  }
  if (parm != -1)
    result = tparm(result, parm);
  if ((len = strlen(result)) < *n)
  {
    if (strstr(result, "$<")) {
      // Only obscure terminals use delays.
      // We can safely ignore them.
      *cbuffer = NULL;
      return;
    }
    strcpy(*cbuffer, result);
    *n -= len + 1;
    *cbuffer += len;
  }
  else
    *cbuffer = NULL;
}

#endif /* HAVE_NCURSES */

void setup_termstrings(bool have_term, bool use_utf8, bool use_color)
{
  term_strings.init = "";
  term_strings.hash = use_utf8 ? "\xe2\x96\x88\xe2\x96\x88" : "##";
  term_strings.light[0] = "";
  term_strings.light[1] = "";
  term_strings.dark = "";
  term_strings.error = "";
  term_strings.v = use_utf8 ? "\xe2\x94\x82" : "|";
  term_strings.h = use_utf8 ? "\xe2\x94\x80\xe2\x94\x80" : "--";
  term_strings.tl = use_utf8 ? "\xe2\x94\x8c" : ".";
  term_strings.tr = use_utf8 ? "\xe2\x94\x90" : ".";
  term_strings.bl = use_utf8 ? "\xe2\x94\x94" : "`";
  term_strings.br = use_utf8 ? "\xe2\x94\x98" : "'";

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

  if (!have_term)
    return;
  if (!isatty(STDOUT_FILENO))
    return;
  if (setupterm(NULL, STDOUT_FILENO, &err) == ERR)
    return;

  TBEGIN(); TPUT("enacs", -1);
  TEND(term_strings.init);
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
  TBEGIN(); TPUT("smacs", -1); TPUT_ACS('a', 2); TPUT("rmacs", -1);
  TEND(term_strings.hash);
  if (use_color)
  {
    TBEGIN(); TPUT("sgr0", -1);
    TEND(term_strings.dark);
    TBEGIN(); TPUT("bold", -1); TPUT("setaf", 7); TPUT("setab", COLOR_CYAN);
    TEND(term_strings.light[0]);
    TBEGIN(); TPUT("bold", -1); TPUT("setaf", 7); TPUT("setab", COLOR_MAGENTA);
    TEND(term_strings.light[1]);
    TBEGIN(); TPUT("bold", -1); TPUT("setaf", 7); TPUT("setab", COLOR_RED);
    TEND(term_strings.error);
  }
  else
  {
    TBEGIN(); TPUT("smacs", -1); TPUT_ACS('0', 2); TPUT("rmacs", -1);
    TEND(term_strings.hash);
  }
#undef BUFFER_SIZE
#endif /* HAVE_NCURSES */
}

/* vim:set ts=2 sts=2 sw=2 et: */
