/* Copyright © 2003, 2004, 2005, 2006, 2008, 2009 Jakub Wilk <jwilk@jwilk.net>
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License, version 2, as published
 * by the Free Software Foundation.
 */

#ifndef NONOGRAM_COMMON_H
#define NONOGRAM_COMMON_H

#ifdef DEBUG
#  define debug 1
#else
#  define debug 0
#endif

#ifndef VERSION
#  define VERSION "[devel]"
#endif

#define max(p,q) ((p)>(q))?(p):(q)

void message(char *message, ...);

#endif

/* vim:set ts=2 sw=2 et: */
