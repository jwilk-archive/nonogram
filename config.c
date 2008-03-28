/* Copyright © 2003, 2004, 2005, 2006, 2008 Jakub Wilk <ubanus@users.sf.net>
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License, version 2, as published
 * by the Free Software Foundation.
 */

#include <getopt.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>

#include "common.h"
#include "config.h"

Config config = { .color = true, .utf8 = false, .html = false, .xhtml = false, .stats = false };

static void show_usage(void)
// Synopsis:
// | prints out usage information and exits
// Note:
// | there is no magic here -- sorry
{
  fprintf(stderr,
    "Usage: nonogram [OPTIONS]\n\n"
    "Options:\n"
    "  -m, --mono        don't use colors\n"
    "  -u, --utf-8       use UTF-8 drawing characters\n"
    "  -H, --html        HTML output\n"
    "  -X, --xhtml       XHTML output\n"
#ifdef DEBUG
    "  -f, --file=FILE   validate the result using FILE\n"
#endif
    "  -h, --help        display this help and exit\n"
    "  -v, --version     output version information and exit\n\n");
  exit(EXIT_FAILURE);
}

static void show_version(void)
// Synopsis:
// | prints out version information and exits
// Note:
// | there is no magic here -- sorry
{
  fprintf(stderr,
    "Nonogram v. " VERSION " -- a nonogram solver.\n"
    "Copyright 2003, 2004, 2005, 2006 Jakub Wilk <ubanus@users.sf.net>\n\n");
  exit(EXIT_FAILURE);
}

void parse_arguments(int argc, char **argv, char **vfn)
// Synopsis:
// | parses the program arguments
// | `vfn' variable is used only if DEBUG directive is defined
// | otherwise, it's ignored
// | we are using getopt (!)
{
  static struct option options [] =
  {
    { "version",    0, 0, 'v' },
    { "help",       0, 0, 'h' },
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
    c = getopt_long(argc, argv, "vhmuHXsf:", options, &optindex);
    if (c < 0)
      break;
    if (c == 0)
      c = options[optindex].val;
    switch (c)
    {
    case 'v': show_version(); break;
    case 'h': show_usage(); break;
    case 'm': config.color = false; break;
    case 'u': config.utf8 = true; break;
    case 'H': config.html = true; break;
    case 'X': config.html = config.xhtml = true; break;
    case 'f': if (debug && optarg != NULL) *vfn = optarg; break;
    case 's': config.stats = true; break;
    default:
      ;
    }
  }
}

/* vim:set ts=2 sw=2 et: */