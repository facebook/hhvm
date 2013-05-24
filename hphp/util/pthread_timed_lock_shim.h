#ifndef PTHREAD_TIMED_LOCK_SHIM
#define PTHREAD_TIMED_LOCK_SHIM

int pthread_mutex_timedlock(pthread_mutex_t *mutex, const struct timespec *timeout)
{
  struct timeval timenow;
  struct timespec sleepytime;
  int retcode;
 
  /* This is just to avoid a completely busy wait */
  sleepytime.tv_sec = 0;
  sleepytime.tv_nsec = 10000000; /* 10ms */

  while ((retcode = pthread_mutex_trylock (mutex)) == EBUSY) {
    gettimeofday (&timenow, NULL);
    if (timenow.tv_sec >= timeout->tv_sec && (timenow.tv_usec * 1000) >= timeout->tv_nsec) {
      return ETIMEDOUT;
    }
    nanosleep (&sleepytime, NULL);
  }
  return retcode;
}

#endif
