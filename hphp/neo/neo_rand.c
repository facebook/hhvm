/*
 * Copyright 2001-2004 Brandon Long
 * All Rights Reserved.
 *
 * ClearSilver Templating System
 *
 * This code is made available under the terms of the ClearSilver License.
 * http://www.clearsilver.net/license.hdf
 *
 */

#include "cs_config.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "neo_misc.h"
#include "neo_err.h"
#include "neo_rand.h"
#include "ulist.h"

static int RandomInit = 0;

void neo_seed_rand (long int seed)
{
#ifdef HAVE_DRAND48
  srand48(seed);
#elif HAVE_RANDOM
  srandom(seed);
#else
  srand(seed);
#endif
  RandomInit = 1;
}

int neo_rand (int max)
{
  int r;

  if (RandomInit == 0)
  {
    neo_seed_rand (time(NULL));
  }
#ifdef HAVE_DRAND48
  r = drand48() * max;
#elif HAVE_RANDOM
  r = random() * max;
#else
  r = rand() * max;
#endif
  return r;
}

int neo_rand_string (char *s, int max)
{
  int size;
  int x = 0;

  size = neo_rand(max-1);
  for (x = 0; x < size; x++)
  {
    s[x] = (char)(32 + neo_rand(127-32));
    if (s[x] == '/') s[x] = ' ';
  }
  s[x] = '\0';

  return 0;
}

static ULIST *Words = NULL;

int neo_rand_word (char *s, int max)
{
  NEOERR *err;
  int x;
  char *word;

  if (Words == NULL)
  {
    FILE *fp;
    char buf[256];

    err = uListInit(&Words, 40000, 0);
    if (err) 
    {
      nerr_log_error(err);
      return -1;
    }
    fp = fopen ("/usr/dict/words", "r");
    if (fp == NULL) {
      fp = fopen ("/usr/share/dict/words", "r");
      if (fp == NULL) {
        ne_warn("Unable to find dict/words file (looked in /usr/dict/words and /usr/share/dict/words)");
        return -1;
      }
    }
    while (fgets (buf, sizeof(buf), fp) != NULL)
    {
      x = strlen (buf);
      if (buf[x-1] == '\n')
	buf[x-1] = '\0';
      uListAppend(Words, strdup(buf));
    }
    fclose (fp);
  }
  x = neo_rand (uListLength(Words));
  uListGet(Words, x, (void *)&word);
  strncpy (s, word, max);
  s[max-1] = '\0';

  return 0;
}

