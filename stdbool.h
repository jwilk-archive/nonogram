/* Copyright (c) 2003, 2004, 2005 Jakub Wilk
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#ifndef STDBOOL_H
#define STDBOOL_H

#include <stdbool.h>
#ifdef __TINYC__
// FIXME: why is this hack necessary?
#undef bool
typedef int bool;
#endif

#endif

/* vim:set ts=2 sw=2 et: */
