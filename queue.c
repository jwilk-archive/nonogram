/* Copyright © 2003-2014 Jakub Wilk <jwilk@jwilk.net>
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

#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "memory.h"
#include "nonogram.h"
#include "queue.h"

static inline void update_queue_enq(Queue *queue, unsigned int i)
{
  assert(i < queue->size);
  queue->enqueued[queue->elements[i].id] = i;
}

static void heapify_queue(Queue *queue)
{
  unsigned int i, l, r, max;
  QueueItem ivalue;

  i = 0;
  while (true)
  {
    ivalue = queue->elements[i];
    l = 2 * i + 1;
    r = l + 1;
    max = (l < queue->size && queue->elements[l].factor > ivalue.factor) ? l : i;
    if (r < queue->size && queue->elements[r].factor > queue->elements[max].factor)
      max = r;
    if (max != i)
    {
      queue->elements[i] = queue->elements[max];
      queue->elements[max] = ivalue;
      update_queue_enq(queue, i);
      update_queue_enq(queue, max);
      i = max;
    }
    else
      return;
  }
}

Queue *alloc_queue(void)
{
  Queue *tmp =
    alloc(
      offsetof(Queue, space) +
      xpysize * (sizeof(unsigned int*) + sizeof(QueueItem)) );
  tmp->size = 0;
  tmp->enqueued = (unsigned int*)tmp->space;
  memset(tmp->enqueued, -1, sizeof(unsigned int*) * xpysize);
  tmp->elements = (QueueItem*)(tmp->space + xpysize * sizeof(unsigned int));
  return tmp;
}

void free_queue(Queue *queue)
{
  free(queue);
}

bool is_queue_empty(Queue *queue)
{
  return queue->size == 0;
}

bool put_into_queue(Queue *queue, unsigned int id, int factor)
{
  unsigned int i, j;

  factor = -factor;

  assert(id < xpysize);
  i = queue->enqueued[id];
  if (i == (unsigned int)-1)
    i = queue->size++;
  else
  {
    assert(i < queue->size);
    if (factor <= queue->elements[i].factor)
      return false;
  }

  while (i > 0 && queue->elements[j = (i-1)/2].factor < factor)
  {
    queue->elements[i] = queue->elements[j];
    update_queue_enq(queue, i);
    i = j;
  }

  queue->elements[i].id = id;
  queue->elements[i].factor = factor;
  update_queue_enq(queue, i);

  return true;
}

unsigned int get_from_queue(Queue *queue)
{
  unsigned int resultid, last;
  assert(queue->size > 0);
  resultid = queue->elements[0].id;
  last = --queue->size;
  if (queue->size > 0)
  {
    queue->elements[0] = queue->elements[last];
    update_queue_enq(queue, 0);
    heapify_queue(queue);
  }
  queue->enqueued[resultid] = (unsigned int)-1;
  return resultid;
}

/* vim:set ts=2 sts=2 sw=2 et: */
