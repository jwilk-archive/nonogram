/* Copyright © 2003-2010 Jakub Wilk <jwilk@jwilk.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the “Software”), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef NONOGRAM_QUEUE_H
#define NONOGRAM_QUEUE_H

#include <stdbool.h>

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

/* vim:set ts=2 sts=2 sw=2 et: */
