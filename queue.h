/* Copyright (c) 2003, 2004, 2005, 2006 Jakub Wilk <ubanus@users.sf.net>
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#ifndef QUEUE_H
#define QUEUE_H

#include "stdbool.h"

typedef struct
{
  unsigned int id;
  int factor;
} QueueItem;

typedef struct
{
  unsigned int size;
  unsigned int *enqueued;
  QueueItem *elements;
  char space[];
} Queue;

Queue *alloc_queue(void);
void free_queue(Queue*);
bool is_queue_empty(Queue*);
bool put_into_queue(Queue*, unsigned int, int);
unsigned int get_from_queue(Queue*);

#endif

/* vim:set ts=2 sw=2 et: */
