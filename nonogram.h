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

/* vim:set ts=2 sts=2 sw=2 et: */
