/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/
#include "hphp/util/shm-counter.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <new>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

bool ShmCounters::created;
int ShmCounters::shmid;
ShmCounters::logError_t ShmCounters::logError;
ShmCounters *ShmCounters::s_shmCounters;

#define LOG_ERROR(fmt, args...) \
do { \
  if (ShmCounters::logError) { \
    ShmCounters::logError(fmt, ##args); \
  } else { \
    fprintf(stderr, fmt, ##args); \
  } \
} while (false)

ShmCounters::~ShmCounters() {
#ifdef ENABLE_SHM_COUNTER
  if (!ShmCounters::created) return;
  if (shmdt(s_shmCounters) == -1) {
    LOG_ERROR("shmdt failed: %d\n", errno);
    return;
  }
  struct shmid_ds sb;
  if (shmctl(ShmCounters::shmid, IPC_STAT, &sb) == -1) {
    LOG_ERROR("shmctl failed: %d\n", errno);
    return;
  }
  if (sb.shm_nattch == 0) shmctl(shmid, IPC_RMID, 0);
#endif
}

bool ShmCounters::initialize(bool create, logError_t logError /* = NULL */) {
#ifdef ENABLE_SHM_COUNTER
  ShmCounters::logError = logError;
  int flags = 0666;
  if (create) flags |= IPC_CREAT;
  int shmid = shmget(SHM_COUNTER_KEY, sizeof(ShmCounters), flags);
  if (shmid == -1) {
    LOG_ERROR("shmget failed: %d\n", errno);
    exit(-1);
  }
  struct shmid_ds sb;
  if (shmctl(shmid, IPC_STAT, &sb) == -1) {
    LOG_ERROR("shmctl failed: %d\n", errno);
    exit(-1);
  }
  if (sb.shm_nattch == 0 && !create) {
    LOG_ERROR("no process attached, exiting...\n");
    shmctl(shmid, IPC_RMID, 0);
    return false;
  }
  s_shmCounters = (ShmCounters *)shmat(shmid, 0, 0);
  if (s_shmCounters == (void *)-1) {
    LOG_ERROR("shmat failed: %d\n", errno);
    exit(-1);
  }
  if (create) new (s_shmCounters) ShmCounters();
  ShmCounters::created = create;
  ShmCounters::shmid = shmid;
#endif
  return true;
}

void ShmCounter::dump() {
  fprintf(stderr, "%s:\t%lld\n", name, count);
}

void ShmCounters::dump() {
  if (s_shmCounters == nullptr) return;
  for (ShmCounter *cp = &s_shmCounters->dummy_def1;
       cp < (ShmCounter *)&s_shmCounters->dummy_defmax;
       cp++) {
    cp->dump();
  }
}

///////////////////////////////////////////////////////////////////////////////
}
