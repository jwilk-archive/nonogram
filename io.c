/* Copyright © 2003-2010 Jakub Wilk <jwilk@jwilk.net>
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

#include <stdarg.h>
#include <stdio.h>

#include "io.h"

char freadchar(FILE *file)
// Synopsis:
// | reads one char from a file represented by `file' variable
// | if there's nothing to read or an error occurred, returns '\0'
// | otherwise, returns the char
{
  char buf = '\0';
  fread((void*)&buf, sizeof(char), 1, file);
  return buf;
}

char readchar(void)
// Synopsis:
// | reads one char from standard input
// | if there's nothing to read or an error occurred, returns '\0'
// | otherwise, returns the char
{
  return freadchar(stdin);
}

void pf(const char *str)
// Synopsis:
// | prints a string `str' to standard output
{
  fputs(str, stdout);
}

void mpf(unsigned int count, ...)
// Synopsis:
// | prints `count' strings to standard output
{
  unsigned int i;
  va_list ap;
  va_start(ap, count);
  for (i = 0; i < count; i++)
    pf(va_arg(ap, char*));
  va_end(ap);
}

/* vim:set ts=2 sts=2 sw=2 et: */
