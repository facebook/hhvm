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
#include "hphp/util/embedded-vfs.h"

/*
 * based on test_demovfs.c and test_multiplex.c in sqlite3
 */
#include <sqlite3.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#ifndef SQLITE_CORE
  #define SQLITE_CORE 1  /* Disable the API redefinition in sqlite3ext.h */
#endif
#include <sqlite3ext.h>

namespace HPHP {

/*
** For a build without mutexes, no-op the mutex calls.
*/
#if defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE==0
#define sqlite3_mutex_alloc(X)    ((sqlite3_mutex*)8)
#define sqlite3_mutex_free(X)
#define sqlite3_mutex_enter(X)
#define sqlite3_mutex_try(X)      SQLITE_OK
#define sqlite3_mutex_leave(X)
#define sqlite3_mutex_held(X)     ((void)(X),1)
#define sqlite3_mutex_notheld(X)  ((void)(X),1)
#endif /* SQLITE_THREADSAFE==0 */

struct embeddedConn {
  sqlite3_file base;
  int fd;
  size_t offset;
  size_t len;
};

static struct {
  sqlite3_vfs*          pOrigVfs;
  sqlite3_vfs           sThisVfs;
  sqlite3_io_methods    sIoMethodsV1;
  sqlite3_mutex*        pMutex;
  int                   isInitialized;
} gEmbedded;

static void embeddedEnter() { sqlite3_mutex_enter(gEmbedded.pMutex); }
static void embeddedLeave() { sqlite3_mutex_leave(gEmbedded.pMutex); }

static int embeddedOpen(sqlite3_vfs *pVfs, const char *zName,
                        sqlite3_file *pConn, int flags,
                        int *pOutFlags) {
  memset(pConn, 0, pVfs->szOsFile);
  assert(zName || (flags & SQLITE_OPEN_DELETEONCLOSE));

  if (zName) {
    const char *p = strchr(zName, ':');
    unsigned long off, len;
    char c;
    if (p && sscanf(p + 1, "%lu:%lu%c", &off, &len, &c) == 2) {
      embeddedConn *eConn = (embeddedConn*)pConn;
      eConn->base.pMethods = &gEmbedded.sIoMethodsV1;
      char *tmp = strdup(zName);
      tmp[p - zName] = 0;
      eConn->fd = open(tmp, O_RDONLY, 0);
      free(tmp);
      if (eConn->fd < 0) return SQLITE_CANTOPEN;
      eConn->offset = off;
      eConn->len = len;
      *pOutFlags = SQLITE_OPEN_READONLY;
      return SQLITE_OK;
    }
  }

  sqlite3_vfs *pOrigVfs = gEmbedded.pOrigVfs;   /* Real VFS */
  return pOrigVfs->xOpen(pOrigVfs, zName, pConn, flags, pOutFlags);
}

static int embeddedDelete(sqlite3_vfs* pVfs, const char* zName, int syncDir) {
  return gEmbedded.pOrigVfs->xDelete(gEmbedded.pOrigVfs, zName, syncDir);
}

static int embeddedAccess(sqlite3_vfs* a, const char* b, int c, int* d) {
  return gEmbedded.pOrigVfs->xAccess(gEmbedded.pOrigVfs, b, c, d);
}

static int embeddedFullPathname(sqlite3_vfs* a, const char* b, int c, char* d) {
  return gEmbedded.pOrigVfs->xFullPathname(gEmbedded.pOrigVfs, b, c, d);
}

static void* embeddedDlOpen(sqlite3_vfs* a, const char* b) {
  return gEmbedded.pOrigVfs->xDlOpen(gEmbedded.pOrigVfs, b);
}

static void embeddedDlError(sqlite3_vfs* a, int b, char* c) {
  gEmbedded.pOrigVfs->xDlError(gEmbedded.pOrigVfs, b, c);
}

static void (*embeddedDlSym(sqlite3_vfs* a, void* b, const char* c))() {
  return gEmbedded.pOrigVfs->xDlSym(gEmbedded.pOrigVfs, b, c);
}

static void embeddedDlClose(sqlite3_vfs* a, void* b) {
  gEmbedded.pOrigVfs->xDlClose(gEmbedded.pOrigVfs, b);
}

static int embeddedRandomness(sqlite3_vfs* a, int b, char* c) {
  return gEmbedded.pOrigVfs->xRandomness(gEmbedded.pOrigVfs, b, c);
}

static int embeddedSleep(sqlite3_vfs* a, int b) {
  return gEmbedded.pOrigVfs->xSleep(gEmbedded.pOrigVfs, b);
}

static int embeddedCurrentTime(sqlite3_vfs* a, double* b) {
  return gEmbedded.pOrigVfs->xCurrentTime(gEmbedded.pOrigVfs, b);
}

static int embeddedGetLastError(sqlite3_vfs* a, int b, char* c) {
  return gEmbedded.pOrigVfs->xGetLastError(gEmbedded.pOrigVfs, b, c);
}

static int embeddedCurrentTimeInt64(sqlite3_vfs* a, sqlite3_int64* b) {
  return gEmbedded.pOrigVfs->xCurrentTimeInt64(gEmbedded.pOrigVfs, b);
}

static int embeddedClose(sqlite3_file* pConn) {
  embeddedConn* p = (embeddedConn*)pConn;
  return close(p->fd) ? SQLITE_OK : SQLITE_ERROR;
}

static int embeddedRead(sqlite3_file* pConn, void* pBuf,
                        int iAmt, sqlite3_int64 iOfst) {
  embeddedConn* p = (embeddedConn*)pConn;
  if (iOfst > p->len) return SQLITE_IOERR_READ;
  int rc = SQLITE_OK;
  if (iAmt + iOfst > p->len) {
    rc = SQLITE_IOERR_SHORT_READ;
    iAmt = p->len - iOfst;
  }
  iOfst += p->offset;
  embeddedEnter();
  if (lseek(p->fd, iOfst, SEEK_SET) != iOfst) {
    rc = SQLITE_IOERR_READ;
  } else if (read(p->fd, pBuf, iAmt) != iAmt) {
    rc = SQLITE_IOERR_READ;
  }
  embeddedLeave();
  return rc;
}

static int embeddedWrite(sqlite3_file* pConn, const void* pBuf,
                         int iAmt, sqlite3_int64 iOfst) {
  return SQLITE_IOERR_WRITE;
}

static int embeddedTruncate(sqlite3_file* pConn, sqlite3_int64 size) {
  return SQLITE_IOERR_TRUNCATE;
}

static int embeddedSync(sqlite3_file* pConn, int flags) {
  return SQLITE_OK;
}

static int embeddedFileSize(sqlite3_file* pConn, sqlite3_int64* pSize) {
  embeddedConn* p = (embeddedConn*)pConn;
  *pSize = p->len;
  return SQLITE_OK;
}

static int embeddedLock(sqlite3_file* pConn, int lock) {
  return SQLITE_OK;
}

static int embeddedUnlock(sqlite3_file* pConn, int lock) {
  return SQLITE_OK;
}

static int embeddedCheckReservedLock(sqlite3_file* pConn, int* pResOut) {
  return SQLITE_IOERR_CHECKRESERVEDLOCK;
}

static int embeddedFileControl(sqlite3_file* pConn, int op, void* pArg) {
  return SQLITE_OK;
}

static int embeddedSectorSize(sqlite3_file* pConn) {
  return 0;
}

static int embeddedDeviceCharacteristics(sqlite3_file* pConn) {
  return 0;
}

int sqlite3_embedded_initialize(const char* zOrigVfsName, int makeDefault) {
  sqlite3_vfs* pOrigVfs;
  if (gEmbedded.isInitialized) return SQLITE_MISUSE;
  pOrigVfs = sqlite3_vfs_find(zOrigVfsName);
  if (pOrigVfs==0) return SQLITE_ERROR;
  assert(pOrigVfs!=&gEmbedded.sThisVfs);
  gEmbedded.pMutex = sqlite3_mutex_alloc(SQLITE_MUTEX_FAST);
  if (!gEmbedded.pMutex) {
    return SQLITE_NOMEM;
  }

  gEmbedded.isInitialized = 1;
  gEmbedded.pOrigVfs = pOrigVfs;
  gEmbedded.sThisVfs = *pOrigVfs;
  gEmbedded.sThisVfs.szOsFile += sizeof(embeddedConn);
  gEmbedded.sThisVfs.zName = "embedded";
  gEmbedded.sThisVfs.xOpen = embeddedOpen;
  gEmbedded.sThisVfs.xDelete = embeddedDelete;
  gEmbedded.sThisVfs.xAccess = embeddedAccess;
  gEmbedded.sThisVfs.xFullPathname = embeddedFullPathname;
  gEmbedded.sThisVfs.xDlOpen = embeddedDlOpen;
  gEmbedded.sThisVfs.xDlError = embeddedDlError;
  gEmbedded.sThisVfs.xDlSym = embeddedDlSym;
  gEmbedded.sThisVfs.xDlClose = embeddedDlClose;
  gEmbedded.sThisVfs.xRandomness = embeddedRandomness;
  gEmbedded.sThisVfs.xSleep = embeddedSleep;
  gEmbedded.sThisVfs.xCurrentTime = embeddedCurrentTime;
  gEmbedded.sThisVfs.xGetLastError = embeddedGetLastError;
  gEmbedded.sThisVfs.xCurrentTimeInt64 = embeddedCurrentTimeInt64;

  gEmbedded.sIoMethodsV1.iVersion = 1;
  gEmbedded.sIoMethodsV1.xClose = embeddedClose;
  gEmbedded.sIoMethodsV1.xRead = embeddedRead;
  gEmbedded.sIoMethodsV1.xWrite = embeddedWrite;
  gEmbedded.sIoMethodsV1.xTruncate = embeddedTruncate;
  gEmbedded.sIoMethodsV1.xSync = embeddedSync;
  gEmbedded.sIoMethodsV1.xFileSize = embeddedFileSize;
  gEmbedded.sIoMethodsV1.xLock = embeddedLock;
  gEmbedded.sIoMethodsV1.xUnlock = embeddedUnlock;
  gEmbedded.sIoMethodsV1.xCheckReservedLock = embeddedCheckReservedLock;
  gEmbedded.sIoMethodsV1.xFileControl = embeddedFileControl;
  gEmbedded.sIoMethodsV1.xSectorSize = embeddedSectorSize;
  gEmbedded.sIoMethodsV1.xDeviceCharacteristics =
    embeddedDeviceCharacteristics;

  sqlite3_vfs_register(&gEmbedded.sThisVfs, makeDefault);

  return SQLITE_OK;
}

int sqlite3_embedded_shutdown() {
  if( gEmbedded.isInitialized==0 ) return SQLITE_MISUSE;
  gEmbedded.isInitialized = 0;
  sqlite3_mutex_free(gEmbedded.pMutex);
  sqlite3_vfs_unregister(&gEmbedded.sThisVfs);
  memset(&gEmbedded, 0, sizeof(gEmbedded));
  return SQLITE_OK;
}

}
