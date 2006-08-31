/* Copyright (c) 2003, 2004, 2005, 2006 Jakub Wilk <ubanus@users.sf.net>
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#ifndef NONOGRAM_H
#define NONOGRAM_H

#define MAX_SIZE 999
#define MAX_FACTOR 10000
#define MAX_EVIL 15.0

typedef signed char bit;
#define Q 0
#define O (-1)
#define X 1

typedef struct
{
  unsigned int counter; // how many Q-fields we have
  unsigned int *linecounter;
  unsigned int *evilcounter;
  bit bits[];
} Picture;

extern unsigned int xsize, ysize, xysize, xpysize, vsize;
extern unsigned int lmax, tmax;

#endif

/* vim:set ts=2 sw=2 et: */
