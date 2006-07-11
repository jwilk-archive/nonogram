/* Copyright (c) 2003, 2004, 2005 Jakub Wilk <ubanus@users.sf.net>
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>
#include <stdlib.h>

#include "config.h"

#ifdef DEBUG
#  define debug true
#else
#  define debug false
#endif

#ifndef VERSION
#  define VERSION "[devel]"
#endif

#define max(p,q) ((p)>(q))?(p):(q)

#ifdef __TINYC__
#  define add64(a, b) (a) = (a) + (b)
#else
#  define add64(a, b) (a) += (b)
#endif

void message(char *message, ...);

#endif

/* vim:set ts=2 sw=2 et: */
