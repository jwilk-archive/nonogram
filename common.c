/* Copyright © 2003, 2004, 2005, 2006, 2008, 2009 Jakub Wilk <jwilk@jwilk.net>
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License, version 2, as published
 * by the Free Software Foundation.
 */

#include <stdio.h>
#include <stdarg.h>

#include "config.h"

void message(char *message, ...)
{
  va_list ap;
  va_start(ap, message);
  if (!config.stats)
    vfprintf(stderr, message, ap);
  va_end(ap);
}

/* vim:set ts=2 sw=2 et: */
