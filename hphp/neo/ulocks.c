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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#ifdef _MSC_VER
#include <io.h>
#include <Windows.h>

#define O_NDELAY 0
#define LOCK_EX 2
#define LOCK_NB 4
#define LOCK_UN 8

#define F_LOCK 0
#define F_ULOCK LOCK_UN

int lockf(int fd, int operation, int unused) {
  HANDLE h = (HANDLE)_get_osfhandle(fd);
  if (h == INVALID_HANDLE_VALUE)
    return -1;

  LARGE_INTEGER fileSize;
  if (!GetFileSizeEx(h, &fileSize))
    return -1;

  if (operation & LOCK_UN) {
    if (!UnlockFile(h, 0, 0, fileSize.LowPart, (DWORD)fileSize.HighPart))
      return -1;
  }
  else {
    int flags = 0;
    OVERLAPPED ov;
    if (operation & LOCK_NB)
      flags |= LOCKFILE_FAIL_IMMEDIATELY;
    if (operation & LOCK_EX)
      flags |= LOCKFILE_EXCLUSIVE_LOCK;
    if (!LockFileEx(h, flags, 0, fileSize.LowPart,
                    (DWORD)fileSize.HighPart, &ov))
      return -1;
  }
  return 0;
}
#else
#include <unistd.h>
#endif

#include "neo_misc.h"
#include "neo_err.h"
#include "neo_files.h"
#include "ulocks.h"

NEOERR *fCreate(int *plock, const char *file) 
{
  NEOERR *err;
  int lock;
  char *p;

  *plock = -1;

  /* note the default mode of 666 is possibly a security hole in that
   * someone else can grab your lock and DoS you.  For internal use, who
   * cares?
   */
  if((lock = open(file, O_WRONLY|O_NDELAY|O_APPEND|O_CREAT|O_EXCL, 0666)) < 0) 
  {
    if (errno == ENOENT)
    {
      p = strrchr (file, '/');
      if (p != NULL)
      {
	*p = '\0';
	err = ne_mkdirs(file, 0777);
	*p = '/';
	if (err != STATUS_OK) return nerr_pass(err);
	lock = open(file, O_WRONLY|O_NDELAY|O_APPEND|O_CREAT, 0666);
      }
    }
    if (errno == EEXIST)
      return nerr_pass(fFind(plock, file));

    if (lock < 0)
      return nerr_raise_errno (NERR_IO, "Unable to open lock file %s", file);
  }

  *plock = lock;

  return STATUS_OK;
}

void fDestroy(int lock) 
{

  if(lock < 0)
    return;

  close(lock);

  return;
}

NEOERR *fFind(int *plock, const char *file) 
{
  int lock;

  *plock = -1;

  if((lock = open(file, O_WRONLY|O_NDELAY|O_APPEND, 0666)) < 0) {
    if (errno == ENOENT)
      return nerr_raise (NERR_NOT_FOUND, "Unable to find lock file %s", file);
    return nerr_raise_errno (NERR_IO, "Unable to open lock file %s", file);
  }

  *plock = lock;

  return STATUS_OK;
}

NEOERR *fLock(int lock) 
{

  if(lockf(lock, F_LOCK, 0) < 0)
    return nerr_raise_errno (NERR_LOCK, "File lock failed");

  return STATUS_OK;
}

void fUnlock(int lock) 
{

  if(lock < 0)
    return;

  lockf(lock, F_ULOCK, 0);

  return;
}

#ifdef HAVE_PTHREADS

NEOERR *mCreate(pthread_mutex_t *mutex) 
{
  int err;

  if((err = pthread_mutex_init(mutex, NULL))) {
    return nerr_raise (NERR_LOCK, "Unable to initialize mutex: %s", 
	strerror(err));
  }

  return STATUS_OK;
}

void mDestroy(pthread_mutex_t *mutex) 
{

  pthread_mutex_destroy(mutex);  

  return;
}

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

NEOERR *cCreate(pthread_cond_t *cond) 
{
  int err;

  if((err = pthread_cond_init(cond, NULL))) {
    return nerr_raise(NERR_LOCK, "Unable to initialize condition variable: %s", 
	strerror(err));
  }

  return STATUS_OK;
}

void cDestroy(pthread_cond_t *cond) 
{
  pthread_cond_destroy(cond);  

  return;
}

NEOERR *cWait(pthread_cond_t *cond, pthread_mutex_t *mutex) 
{
  int err;

  if((err = pthread_cond_wait(cond, mutex)))
    return nerr_raise(NERR_LOCK, "Condition wait failed: %s", strerror(err));

  return STATUS_OK;
}

NEOERR *cBroadcast(pthread_cond_t *cond) 
{
  int err;

  if((err = pthread_cond_broadcast(cond)))
    return nerr_raise(NERR_LOCK, "Condition broadcast failed: %s", 
	strerror(err));

  return STATUS_OK;
}

NEOERR *cSignal(pthread_cond_t *cond) 
{
  int err;

  if((err = pthread_cond_signal(cond)))
    return nerr_raise (NERR_LOCK, "Condition signal failed: %s", strerror(err));

  return STATUS_OK;
}

#endif
