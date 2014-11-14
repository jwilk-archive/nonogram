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

#include "autoconfig.h"

#include <getopt.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "config.h"

Config config = {
  .color = false,
  .utf8 = false,
  .html = false,
  .xhtml = false,
  .stats = false
};

static void show_usage(void)
{
  fprintf(stderr,
    "Usage: nonogram [OPTIONS]\n\n"
    "Options:\n"
    "  -c, --colors      use colors\n"
    "  -u, --utf-8       use UTF-8 drawing characters\n"
    "  -H, --html        HTML output\n"
    "  -X, --xhtml       XHTML output\n"
#if ENABLE_DEBUG
    "  -f, --file=FILE   validate the result using FILE\n"
#endif
    "  -h, --help        display this help and exit\n"
    "  -v, --version     output version information and exit\n\n");
  exit(EXIT_FAILURE);
}

static void show_version(void)
{
  fprintf(stderr,
    PACKAGE_STRING " -- a nonogram solver\n"
    "Copyright (c) 2003-2014 Jakub Wilk\n");
  exit(EXIT_FAILURE);
}

void parse_arguments(int argc, char **argv, char **vfn)
{
  static struct option options [] =
  {
    { "version",    0, 0, 'v' },
    { "help",       0, 0, 'h' },
    { "colors",     0, 0, 'c' },
    { "mono",       0, 0, 'm' },
    { "utf-8",      0, 0, 'u' },
    { "html",       0, 0, 'H' },
    { "xhtml",      0, 0, 'X' },
    { "file",       0, 0, 'f' }, // XXX undocumented
    { "statistics", 0, 0, 's' }, // XXX undocumented
    { NULL,         0, 0, '\0' }
  };

  int optindex, c;

  while (true)
  {
    optindex = 0;
    c = getopt_long(argc, argv, "vhcmuHXsf:", options, &optindex);
    if (c < 0)
      break;
    if (c == 0)
      c = options[optindex].val;
    switch (c)
    {
    case 'v':
      show_version();
      break;
    case 'h':
      show_usage();
      break;
    case 'm':
      config.color = false;
      break;
    case 'c':
      config.color = true;
      break;
    case 'u':
      config.utf8 = true;
      break;
    case 'H':
      config.html = true;
      break;
    case 'X':
      config.html = config.xhtml = true;
      break;
    case 'f':
      if (ENABLE_DEBUG && optarg != NULL)
        *vfn = optarg;
      break;
    case 's':
      config.stats = true;
      break;
    default:
      exit(EXIT_FAILURE);
      ;
    }
  }
  if (optind < argc)
  {
    fprintf(stderr, "%s: too many arguments\n", argv[0]);
    exit(EXIT_FAILURE);
  }
}

/* vim:set ts=2 sts=2 sw=2 et: */
