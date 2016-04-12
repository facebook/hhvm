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

#include <string.h>

#include "neo_misc.h"
#include "neo_err.h"
#include "ulocks.h"

#ifdef HAVE_PTHREADS

NEOERR *mLock(pthread_mutex_t *mutex) 
{
  int err;

  if((err = pthread_mutex_lock(mutex)))
    return nerr_raise(NERR_LOCK, "Mutex lock failed: %s", strerror(err));

  return STATUS_OK;
}

NEOERR *mUnlock(pthread_mutex_t *mutex) 
{
  int err;

  if((err = pthread_mutex_unlock(mutex)))
    return nerr_raise(NERR_LOCK, "Mutex unlock failed: %s", strerror(err));

  return STATUS_OK;
}

#endif
