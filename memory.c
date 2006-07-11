/* Copyright (c) 2003, 2004, 2005 Jakub Wilk <ubanus@users.sf.net>
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include "common.h"

static inline void raise_oom_error(void)
// Synopis:
// | tells about OOM error
// | afterwards, aborts
{
  message("Out of memory!\n");
  abort();
}

void *alloc(size_t size)
// Synopsis:
// | allocates `size' bytes of memory
// | if necessary, handles errors
{
  void *tmp = calloc(1, size);
  if (tmp == NULL)
    raise_oom_error();
  return tmp;
}

/* vim:set ts=2 sw=2 et: */
