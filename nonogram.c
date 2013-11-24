/* Copyright © 2003, 2004, 2005, 2006, 2008, 2009, 2010, 2013 Jakub Wilk <jwilk@jwilk.net>
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
#include <errno.h>
#include <math.h>
#ifndef __WIN32__
#include <signal.h>
#endif
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "io.h"
#include "common.h"
#include "config.h"
#include "memory.h"
#include "nonogram.h"
#include "queue.h"
#include "term.h"

Picture *mainpicture;
unsigned int *leftborder, *topborder;
uint64_t *gtestfield;
unsigned int xsize, ysize, xysize, xpysize, vsize;
unsigned int lmax, tmax;

uint64_t fingercounter;

static double binomln(int n, int k)
// Synopsis:
// | evaluates ln binom(`n', `k')
// | if possible, returns the result
// | otherwise, returns +0.0
{
  double tmp;

  if (n <= k || n <= 0 || k <= 0)
    return 0.0;

  double dn = (double)n;
  double dk = (double)k;

  tmp = -0.5 * log(8 * atan(1)); // atan 1 = pi / 4
  tmp += (dn + 0.5) * log(dn);
  tmp -= (dk + 0.5) * log(dk);
  tmp -= (dn - dk + 0.5) * log(dn - dk);
  return tmp;
}

static void raise_input_error(unsigned int n)
// Synopsis:
// | tells about invalid input at line `n'
// | afterwards, aborts
{
  message("Invalid input at line %u!\n", n);
  abort();
}

static void handle_sigint()
// Synopsis:
// | the SIGINT handler
// | says ``Ouch!''
// | if necessary, turns all colors off
{
  fflush(stdout);
  message("%s\nOuch!\n\n", term_strings.dark);
  exit(EXIT_FAILURE);
}

static void setup_sigint()
{
#ifndef __WIN32__
  struct sigaction act;
  act.sa_handler = handle_sigint;
  act.sa_flags = 0;
  sigemptyset(&act.sa_mask);
  sigaddset(&act.sa_mask, SIGINT);
  sigaction(SIGINT, &act, NULL);
#endif
}

static void print_picture_plain(bit *picture, bit *cpicture, bool use_ncurses)
// Synopsis:
// | vanilla version of generic picture printer
// | `picture' is a picture with our result
// | `cpicture' is a picture with which we want to compare our result
{
  char *str_color;

  if (debug && cpicture == NULL)
    cpicture = picture;

  unsigned int i, j, t;

  setup_termstrings(use_ncurses, config.utf8, config.color);

  pf(term_strings.init);

  for (i = 0; i < tmax; i++)
  {
    pf(" ");
    for (j = 0; j < lmax; j++) pf("  ");
    for (j = 0; j < xsize; j++)
    {
      str_color = term_strings.light[j & 1];
      t = topborder[j * ysize + i];
      if (t != 0 || i == 0)
        pf(str_color), printf("%2u", t), pf(term_strings.dark);
      else
        mpf(3, str_color, "  ", term_strings.dark);
    }
    pf("\n");
  }

  for (i = 0; i < lmax; i++) pf("  ");
  pf(term_strings.tl);
  for (i = 0; i < xsize; i++) pf(term_strings.h);
  mpf(2, term_strings.tr, "\n");
  for (i = 0; i < ysize; i++)
  {
    for (j = 0; j < lmax; j++)
    {
      str_color =term_strings.light[j & 1];
      t = leftborder[i * xsize + j];
      if (t != 0 || j == 0)
        pf(str_color), printf("%2u", t), pf(term_strings.dark);
      else
        mpf(3, str_color, "  ", term_strings.dark);
    }
    pf(term_strings.v);
    for (j = 0; j < xsize; j++)
    {
      str_color = term_strings.light[j & 1];
      switch (*picture)
      {
        case Q:
          mpf(3, str_color, "<>", term_strings.dark);
          break;
        case O:
          if (debug && *cpicture == X)
            mpf(2, term_strings.error, "..");
          else
            mpf(2, str_color, "  ");
          pf(term_strings.dark);
          break;
        case X:
          if (debug && *cpicture == O)
            mpf(2, term_strings.error, term_strings.hash);
          else
            mpf(2, str_color, term_strings.hash);
          pf(term_strings.dark);
          break;
      }
      picture++;
      debug && cpicture++;
    }
    mpf(2, term_strings.v, "\n");
  }
  for (i = 0; i < lmax; i++) pf("  ");
  pf(term_strings.bl);
  for (i = 0; i < xsize; i++) pf(term_strings.h);
  mpf(2, term_strings.br, "\n\n");
  fflush(stdout);
}

static void print_html_dtd(bool use_xhtml, bool need_charset)
{
  if (use_xhtml && need_charset)
    pf("<?xml version='1.0' encoding='ISO-8859-1'?>\n");
  pf(use_xhtml ?
    "<!DOCTYPE html PUBLIC '-//W3C//DTD XHTML 1.0 Strict//EN' 'http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd'>\n" :
    "<!DOCTYPE html PUBLIC '-//W3C//DTD HTML 4.01//EN' 'http://www.w3.org/TR/html4/strict.dtd'>\n");
}

#ifdef FANCY
static void print_picture_html(bit *picture, bool use_xhtml)
// Synopsis
// | HTML version of generic picture printer
// | `picture': our result
{
  unsigned int i, j;
  print_html_dtd(use_xhtml, true);
  mpf(3,
    "<html>\n"
    "<head>\n"
    "<title>Nonogram solution</title>\n"
    "<meta http-equiv='Content-type' content='text/html; charset=ISO-8859-1'", (use_xhtml ? " /" : ""), ">\n"
    "<style type='text/css'>\n"
    "  table "  "{ border-collapse: collapse; } \n"
    "  td, th " "{ font: 8pt Arial, sans-serif; width: 11pt; height: 11pt; }\n"
    "  th "     "{ background-color: #fff; color: #000;"
                 " border: dotted 1px #888; }\n"
    "  th.empty  { border: none; }\n"
    "  td.x "   "{ background-color: #000; color: #000; }\n"
    "  td.v "   "{ background-color: #888; color: #f00; }\n"
    "  td "     "{ background-color: #eee; color: #000;"
                 " border: solid 1px #888; text-align: center; }\n"
    "</style>\n"
    "</head>\n"
    "<body>\n"
    "<table border='0' cellpadding='0' cellspacing='0'>");

  unsigned int *top_desc_size = alloc(xsize * sizeof (unsigned int));
  for (i = 0; i < xsize; i++)
  {
    top_desc_size[i] = 0;
    for (j = 0; j < tmax; j++)
      if (topborder[i * ysize + j] == 0)
        break;
      else
        top_desc_size[i]++;
  }

  for (i = 0; i < tmax; i++)
  {
    pf("<tr>");
    if (i == 0)
      printf("<th class='empty' colspan='%u' rowspan='%u'>\xa0</th>", lmax, tmax);
    for (j = 0; j < xsize; j++)
    {
      if (i < tmax - top_desc_size[j])
        pf("<th>\xa0</th>");
      else
        printf("<th>%u</th>", topborder[j * ysize + i - tmax + top_desc_size[j]]);
    }
    pf("</tr>\n");
  }
  
  free(top_desc_size);

  for (i = 0; i < ysize; i++)
  {
    pf("<tr>");
    for (j = 0; j < lmax; j++)
      if (leftborder[i * xsize + j] == 0)
        break;
    for (; j < lmax; j++)
      printf("<th>\xa0</th>");
    for (j = 0; j < lmax; j++)
    {
      unsigned int t = leftborder[i * xsize + j];
      if (t != 0)
        printf("<th>%u</th>", t);
    }
    for (j = 0; j < xsize; j++, picture++)
    switch (*picture)
    {
    case Q:
      pf("<td class='v'>?</td>");
      break;
    case O:
      pf("<td>\xa0</td>");
      break;
    case X:
      pf("<td class='x'>#</td>");
      break;
    }
    pf("</tr>\n");
  }
  pf("</table>\n</body>\n</html>\n");
}
#else /* undefined FANCY: */
static void print_picture_html(bit *picture, bool use_xhtml)
{
  config.color = false;
  print_html_dtd(use_xhtml, false);
  pf("<html>\n<head>\n<title>Nonogram solution</title>\n</head>\n<body>\n<pre>\n");
  print_picture_plain(picture, NULL, false);
  pf("</pre>\n</body>\n</html>\n");
}
#endif /* FANCY */

static inline void print_picture(bit *picture, bit *cpicture)
// Synopsis:
// | picture printer
// | automatically chooses an appropriate version
// |   of generic printer
// | `picture': our result
// | `cpicture': picture to compare with our result
{
  if (config.stats)
    return; // XXX undocumented!
  if (config.html)
    print_picture_html(picture, config.xhtml);
  else
    print_picture_plain(picture, cpicture, true);
}

static uint64_t touch_line(bit *picture, unsigned int range, uint64_t *testfield, unsigned int *borderitem, bool vert)
// it's rather ``touch of the mind'' :=)
{
  unsigned int i, j, k, count, sum, mul;
  uint64_t z, ink;
  bool ok;

  fingercounter++;

  sum = count = 0;
  mul = vert ? xsize : 1;

  for (i = 0; borderitem[i] > 0; i++)
  {
    count++;
    sum += borderitem[i];
  }

  if (sum + count > range + 1)
    return 0;

  k = borderitem[0];
  z = 0;
  if (count == 1)
  for (i = 0; i + k <= range; i++)
  {
    ok = true;
    for (j = 0; j < i; j++)               if (picture[j * mul] == X) { ok = false; break; };
    if (!ok) break;
    for (j = i; j < i + k && ok; j++)     if (picture[j * mul] == O) ok = false;
    for (j = i + k; j < range && ok; j++) if (picture[j * mul] == X) ok = false;
    if (ok)
    {
      for (j = i; j < i + k; j++) testfield[j] += 1;
      z++;
    }
  }
  else
  for (i = 0; i <= range - sum - count + 1; i++)
  {
    ok = true;
    for (j = 0; j < i; j++)           if (picture[j * mul] == X) { ok = false; break; };
    if (!ok) break;
    for (j = i; j < i + k && ok; j++) if (picture[j * mul] == O) ok = false;
    if (!ok) continue;
    if (i + k < range && picture[(i + k) * mul] == X) continue;
    j = i + k + 1;
    ink =
      (count == 1) ?
        1 :
        touch_line(picture + j * mul, range - j, testfield + j, borderitem + 1, vert);
    if (ink != 0)
    {
      for (j = i; j < i + k; j++)
        testfield[j] += ink;
      z += ink;
    }
  }
  return z;
}

static void finger_line(Picture *mpicture, Queue *queue)
// shouldn't it be splitted into two functions?
{
  bit *picture;
  uint64_t *testfield;
  uint64_t q, u;
  unsigned int i, j, imul, mul, size, oline, line;
  int factor;
  bool vert;

  fingercounter++;
  line = oline = get_from_queue(queue);
  if (line < ysize)
    imul = xsize, mul = 1, size = xsize, vert = false;
  else
    imul = 1, mul = xsize, size = ysize, line -= ysize, vert = true;

  j = mpicture->linecounter[oline];
  if (j == 0 || j == size)
    return;

  picture = mpicture->bits + line * imul;
  testfield = gtestfield;
  memset(testfield, 0, size * sizeof(uint64_t));

  if (vert)
    q = touch_line(picture, size, testfield, topborder + line * size, true);
  else
    q = touch_line(picture, size, testfield, leftborder + line * size, false);

  j = vert ? 0 : ysize;
  for (i = j; i < j + size; i++)
  {
    u = *testfield++;
    if ((u == q || u == 0) && (*picture == Q))
    {
      mpicture->counter--;
      mpicture->linecounter[oline]--;
      factor = MAX_FACTOR * (--mpicture->linecounter[i]) / size + mpicture->evilcounter[i];
      put_into_queue(queue, i, factor);
      *picture = u ? X : O;
    }
    picture += mul;
  }
}

static bool check_consistency(bit *picture)
{
  bool fr;
  unsigned int i, j;
  unsigned int r, rv;
  unsigned int *border;
  bit *tpicture;

  for(i = 0; i < ysize; i++)
  {
    fr = true;
    r = 0;
    rv = 0;
    border = leftborder + i * xsize;
    tpicture = picture + i * xsize;
    for (j = 0; j < xsize && fr; j++, tpicture++)
    switch (*tpicture)
    {
    case Q:
      fr = false;
      break;
    case X:
      rv++;
      break;
    case O:
      if (rv == 0)
        break;
      if (*border != rv)
      {
        debug && fprintf(stderr, "Inconsistency at row #%u[%u]! (%u, expected %u)!\n", i, j, rv, *border);
        return false;
      }
      rv = 0; r++; border++;
      break;
    default:
      ;
    }
    if (fr && *border != rv)
    {
      debug && fprintf(stderr, "Inconsistency at the end of row #%u! (%u, expected %u)\n", i, rv, *border);
      return false;
    }
  }

  for (i = 0; i < xsize; i++)
  {
    fr = true;
    r = 0;
    rv = 0;
    border = topborder + i * ysize;
    tpicture = picture + i;
    for (j = 0; j < ysize && fr; j++, tpicture += xsize)
    switch (*tpicture)
    {
    case Q:
      fr = false;
      break;
    case X:
      rv++;
      break;
    case O:
      if (rv == 0)
        break;
      if (*border != rv)
      {
        debug && fprintf(stderr, "Inconsistency at column #%u[%u]! (%u, expected %u)\n", i, j, rv, *border);
        return false;
      }
      rv = 0; r++; border++;
      break;
    default:
      ;
    }
    if (fr && *border != rv)
    {
      debug && fprintf(stderr, "Inconsistency at the end of column #%u! (%u, expected %u)\n", i, rv, *border);
      return false;
    }
  }
  return true;
}

static inline void *alloc_border(void)
// Synopsis:
// | allocates a border
{
  return alloc(vsize * sizeof(unsigned int));
}

static inline void *alloc_testfield(void)
// Synopsis:
// | allocates a testfield
{
  return alloc(xysize * sizeof(uint64_t));
}

static void *alloc_picture(void)
// Synopsis:
// | allocates a picture
{
  unsigned int i;
  Picture *tmp =
    alloc(
      offsetof(Picture, bits) +
      vsize * sizeof(bit) );
  tmp->linecounter = alloc(sizeof(unsigned int) * xpysize);
  tmp->evilcounter = alloc(sizeof(unsigned int) * xpysize);
  for (i = 0; i < ysize; i++)
    tmp->linecounter[i] = xsize;
  for (i = 0; i < xsize; i++)
    tmp->linecounter[ysize + i] = ysize;
  tmp->counter = vsize;
  return tmp;
}

static inline void free_picture(Picture *picture)
// Synopsis:
// | frees `picture'
{
  free(picture->linecounter);
  free(picture->evilcounter);
  free(picture);
}

static inline void duplicate_picture(Picture *src, Picture *dst)
// Synopsis:
// | copies picture `src' onto picture `dst'
{
  dst->counter = src->counter;
  memcpy(dst->linecounter, src->linecounter, sizeof(unsigned int) * xpysize);
  memcpy(dst->evilcounter, src->evilcounter, sizeof(unsigned int) * xpysize);
  memcpy(dst->bits, src->bits, vsize * sizeof(bit));
}

static void preliminary_shake(Picture *mpicture)
{
  unsigned int i, j, k;
  unsigned int R, ML;
  bit *picture;
  unsigned int *band;

  for (i = 0; i < ysize; i++)
  {
    band = leftborder + i * xsize;
    R = *band++;
    ML = R;
    while (*band > 0)
      ML += *band++ + 1;

    band = leftborder + i * xsize;
    if (*band == 0)
    {
      picture = &mpicture->bits[i * xsize];
      for(j = 0; j < xsize; j++, picture++)
      {
        *picture = O;
        mpicture->counter--;
      }
    }
    while (*band > 0)
    {
      k = xsize - ML;
      if (k < R)
      {
        picture = mpicture->bits + i * xsize + k;
        for ( ; k < R; k++, picture++)
        {
          *picture = X;
          mpicture->counter--;
        }
      }
      ML -= *band; ML--;
      R++; R += *++band;
    }
  }

  for (i = 0; i < xsize; i++)
  {
    band = topborder + i * ysize;
    R = *band++;
    ML = R;
    while (*band > 0)
      ML += *band++ + 1;

    band = topborder + i * ysize;
    if (*band == 0)
    {
      picture = mpicture->bits + i;
      for (j = 0; j < ysize; j++, picture += xsize)
      if (*picture == Q)
      {
        *picture = O;
        mpicture->counter--;
      }
    }
    while (*band > 0)
    {
      k = ysize - ML;
      if (k < R)
      {
        picture = mpicture->bits + k * xsize + i;
        for ( ; k < R; k++, picture += xsize)
        if (*picture == Q)
        {
          *picture = X;
          mpicture->counter--;
        }
      }
      ML -= *band; ML--;
      R++; R += *++band;
    }
  }

  picture = mpicture->bits;
  for (i = 0; i < ysize; i++)
  for (j = 0; j < xsize; j++, picture++)
  if (*picture != Q)
  {
    mpicture->linecounter[i]--;
    mpicture->linecounter[ysize + j]--;
  }
}

static inline void shake(Picture *mpicture)
{
  unsigned int i, j;
  int factor;
  Queue *queue = alloc_queue();

  for (i = 0; i < ysize; i++)
  {
    factor = MAX_FACTOR * mpicture->linecounter[i] / xsize + mpicture->evilcounter[i];
    put_into_queue(queue, i, factor);
  }

  for (i = 0, j = ysize; i < xsize; i++, j++)
  {
    factor = MAX_FACTOR * mpicture->linecounter[j] / ysize + mpicture->evilcounter[i];
    put_into_queue(queue, j, factor);
  }

  while (!is_queue_empty(queue))
    finger_line(mpicture, queue);

  free_queue(queue);
  return;
}

static bool make_unambiguous(Picture *mpicture)
{
  Picture *mclone;
  bit *picture;
  unsigned int i, j, n;
  bool res = false;

  mclone = alloc_picture();
  picture = mpicture->bits;
  n = 0;
  for (i = 0; i < ysize && !res; i++)
  for (j = 0; j < xsize && !res; j++, n++, picture++)
  if (*picture == Q)
  {
    duplicate_picture(mpicture, mclone); // mpicture --> mclone
    mclone->bits[n] = O;
    mclone->counter--;
    mclone->linecounter[i]--;
    mclone->linecounter[ysize + j]--;
    shake(mclone);
    res = make_unambiguous(mclone);
    if (res)
      duplicate_picture(mclone, mpicture); // mclone --> mpicture
    else
    {
      *picture = X;
      mpicture->counter--;
      mpicture->linecounter[i]--;
      mpicture->linecounter[ysize + j]--;
      shake(mpicture);
    }
  }
  free_picture(mclone);
  return check_consistency(mpicture->bits);
}

static unsigned int measure_evil(int r, int k)
{
  double tmp = binomln(r, k);
  if (tmp > MAX_EVIL)
    tmp = MAX_EVIL;
  return floor(tmp * MAX_EVIL * MAX_FACTOR);
}

int main(int argc, char **argv)
{
  char c;
  unsigned int i, j, k, sane;
  unsigned int evs, evm;
  bit *checkbits = NULL;

#ifdef DEBUG
  FILE *verifyfile;
  Picture *checkpicture;
#endif
  static char *verifyfname = NULL;

  setup_sigint();

  parse_arguments(argc, argv, &verifyfname);

  xsize = ysize = 0;
  c = readchar();
  while (c >= '0' && c <= '9') { xsize *= 10; xsize += c-'0'; c = readchar(); }
  while (c == ' ' || c == '\t') c = readchar();
  while (c >= '0' && c <= '9') { ysize *= 10; ysize += c-'0'; c = readchar(); }
  while (c <= ' ') c = readchar();

  if (xsize < 1 || ysize < 1 || xsize > MAX_SIZE || ysize > MAX_SIZE)
    raise_input_error(1);

  vsize = xsize * ysize;
  xpysize = xsize + ysize;
  xysize = max(xsize, ysize);

  leftborder = alloc_border();
  topborder = alloc_border();
  gtestfield = alloc_testfield();

  mainpicture = alloc_picture();

  evs = evm = 0;
  sane = (unsigned int)-1;
  lmax = 0;
  for (i = j = 0; i < ysize; )
  {
    k = 0;
    while (c >= '0' && c <= '9') { k *= 10; k += c-'0'; c=readchar(); }
    sane += k + 1;
    if ((sane>xsize) || (k == 0 && j > 0))
      raise_input_error(2 + i);
    leftborder[i * xsize + j] = k;
    evs += k;
    if (k > evm)
      evm = k;
    while (c == ' ' || c == '\t') c = readchar();
    if (c == '\r' || c == '\n' || c == '\0')
    {
      if (j > lmax)
        lmax = j;
      mainpicture->evilcounter[i] = measure_evil(xsize - evs + 1, j + 1);
      evs = evm = 0;
      i++;
      j = 0;
      sane = -1;
      do
        c = readchar();
      while (c == '\r' || c == '\n');
    }
    else
      j++;
  }

  assert(sane == (unsigned int)-1);
  tmax = 0;
  for (i = j = 0; i < xsize; )
  {
    k = 0;
    while (c >= '0' && c <= '9') { k *= 10; k += c-'0'; c = readchar(); }
    sane += k + 1;
    if ((sane > ysize) || (k == 0 && j > 0))
      raise_input_error(2 + ysize + i);
    topborder[i * ysize + j] = k;
    evs += k;
    if (k > evm)
      evm = k;
    while (c == ' ' || c == '\t') c = readchar();
    if (c == '\r' || c == '\n' || c == '\0')
    {
      if (j > tmax)
        tmax = j;
      mainpicture->evilcounter[ysize + i] = measure_evil(ysize - evs + 1, j + 1);
      evs = evm = 0;
      i++;
      j = 0;
      sane =- 1;
      do
        c = readchar();
      while (c=='\r' || c=='\n');
    }
    else
      j++;
  }

  lmax++;
  tmax++;

#ifdef DEBUG
  if (verifyfname != NULL)
  {
    verifyfile = fopen(verifyfname, "r");
    if (verifyfile != NULL)
    {
      checkpicture = alloc_picture();
      checkpicture->counter = 0;
      c = 0;
      for (i = 0; i < ysize; i++)
      {
        while (c < ' ')
          c = freadchar(verifyfile);
        for (j = 0; j < xsize; j++)
        {
          checkpicture->bits[i * xsize + j] = (c == '#') ? X : O;
          freadchar(verifyfile);
          c = freadchar(verifyfile);
        }
      }
      fclose(verifyfile);
      checkbits = checkpicture->bits;
    }
  }
#endif /* DEBUG */

  fingercounter = 0;

  preliminary_shake(mainpicture);
  shake(mainpicture);

  if (!check_consistency(mainpicture->bits))
  {
    fingercounter = 0;
    message("Inconsistent! Bamf!\n");
    if (debug)
      print_picture(mainpicture->bits, checkbits);
  }
  else
  {
    if ((mainpicture->counter == 0) || debug)
      print_picture(mainpicture->bits, checkbits);
    if (mainpicture->counter != 0)
    {
      message("Ambiguous (%u)! But trying to find a solution...\n", mainpicture->counter);
      if (make_unambiguous(mainpicture))
        print_picture(mainpicture->bits, checkbits);
      else
      {
        fingercounter = 0;
        message("Inconsistent! Bamf!\n");
      }
    }
  }

  if (config.stats)
    fprintf(stderr, "%ju\n", fingercounter);

  return EXIT_SUCCESS;
}

/* vim:set ts=2 sw=2 et: */
