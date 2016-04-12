/*
 *
 * Thread-safe Skiplist Using Integer Identifiers
 * Copyright 1998-2000 Scott Shambarger (scott@shambarger.net)
 *
 * This software is open source. Permission to use, copy, modify, and
 * distribute this software for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies.  No
 * warranty of any kind is expressed or implied.  Use at your own risk.
 *
 * 1/14/2001 blong
 *   Made it use neo errs... probably need to check locking functions
 *   for error returns...
 *
 */

#ifndef incl_HPHP_ULOCKS_H_
#define incl_HPHP_ULOCKS_H_

#ifdef HAVE_PTHREADS

#include <pthread.h>

NEOERR *mLock(pthread_mutex_t *mutex);
/*
 * Function:    mLock - lock a mutex.
 * Description: Locks the mutex <mutex>.  This call blocks until the mutex
 *              is acquired.
 * Input:       mutex - mutex to lock.
 * Output:      None.
 * Return:      STATUS_OK on success
 *              NERR_LOCK on failure
 * MT-Level:    Safe.
 */

NEOERR *mUnlock(pthread_mutex_t *mutex);
/*
 * Function:    mUnlock - unlock a mutex.
 * Description: Unlocks the mutex <mutex>.
 * Input:       mutex - mutex to unlock.
 * Output:      None.
 * Return:      STATUS_OK on success
 *              NERR_LOCK on failure
 * MT-Level:    Safe.
 */

#endif /* HAVE_PTHREAD */

#endif                                                       /* incl_HPHP_ULOCKS_H_ */
