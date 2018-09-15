#ifndef GDHELPERS_H
#define GDHELPERS_H 1

#include <sys/types.h>

#include "hphp/runtime/base/memory-manager.h"

/* TBB: strtok_r is not universal; provide an implementation of it. */

extern char *gd_strtok_r(char *s, char *sep, char **state);

/* Check for request OOM.  Call before allocating a lot of memory. */
inline bool precheckOOM(size_t allocsz) {
  return allocsz > HPHP::kMaxSmallSize && HPHP::tl_heap->preAllocOOM(allocsz);
}

/* These functions wrap memory management. gdFree is
  in gd.h, where callers can utilize it to correctly
  free memory allocated by these functions with the
  right version of free(). */
#define gdCalloc(nmemb, size) HPHP::req::calloc_noptrs(nmemb, size)
#define gdMalloc(size)        HPHP::req::malloc_noptrs(size)
#define gdRealloc(ptr, size)  HPHP::req::realloc_noptrs(ptr, size)
#define gdFree(ptr)           HPHP::req::free(ptr)
#define gdPMalloc(ptr)        malloc(ptr)
#define gdPFree(ptr)          free(ptr)
#define gdPEstrdup(ptr)       strdup(ptr)

inline char *gdEstrdup(const char *s) {
  auto length = strlen(s);
  char* ret = (char*)HPHP::req::malloc_noptrs(length + 1);
  memcpy(ret, s, length);
  ret[length] = '\0';
  return ret;
}

inline void *safe_emalloc(size_t nmemb, size_t size, size_t offset) {
  return HPHP::req::malloc_noptrs(nmemb * size + offset);
}

/* Returns nonzero if multiplying the two quantities will
  result in integer overflow. Also returns nonzero if
  either quantity is negative. By Phil Knirsch based on
  netpbm fixes by Alan Cox. */

int overflow2(int a, int b);

#define gdMutexDeclare(x) pthread_mutex_t x
#define gdMutexSetup(x) pthread_mutex_init(&x, 0)
#define gdMutexShutdown(x) pthread_mutex_destroy(&x)
#define gdMutexLock(x) pthread_mutex_lock(&x)
#define gdMutexUnlock(x) pthread_mutex_unlock(&x)

#endif /* GDHELPERS_H */
