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
#include "neo_misc.h"
#include "neo_rand.h"

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
