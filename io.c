/* Copyright © 2003, 2004, 2005, 2006, 2008, 2009 Jakub Wilk
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License, version 2, as published
 * by the Free Software Foundation.
 */

#include <stdarg.h>
#include <stdio.h>

#include "io.h"

char freadchar(FILE *file)
// Synopsis:
// | reads one char from a file represented by `file' variable
// | if there's nothing to read or an error ocurred, returns '\0'
// | otherwise, returns the char
{
  char buf = '\0';
  fread((void*)&buf, sizeof(char), 1, file);
  return buf;
}

char readchar(void)
// Synopsis:
// | reads one char from standard input
// | if there's nothing to read or an error ocurred, returns '\0'
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

/* vim:set ts=2 sw=2 et: */
