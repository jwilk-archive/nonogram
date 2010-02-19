/* Copyright © 2003, 2004, 2005, 2006, 2008, 2009 Jakub Wilk <jwilk@jwilk.net>
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License, version 2, as published
 * by the Free Software Foundation.
 */

#ifndef NONOGRAM_IO_H
#define NONOGRAM_IO_H

char freadchar(FILE *file);
char readchar(void);
void pf(const char *str);
void mpf(unsigned int count, ...);

#endif

/* vim:set ts=2 sw=2 et: */
