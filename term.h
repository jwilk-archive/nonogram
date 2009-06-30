/* Copyright © 2003, 2004, 2005, 2006, 2008, 2009 Jakub Wilk
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License, version 2, as published
 * by the Free Software Foundation.
 */

#ifndef NONOGRAM_TERM_H
#define NONOGRAM_TERM_H

#include <stdbool.h>

typedef struct
{
  char *light[2], *dark, *color, *error;
  char *h, *v, *tl, *bl, *tr, *br;
  char *hash;
  char *init;
} TermStrings;

extern TermStrings term_strings;

void setup_termstrings(bool, bool, bool);

#endif

/* vim:set ts=2 sw=2 et: */
