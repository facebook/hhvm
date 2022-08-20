/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef incl_THRIFT_MUTEX_PORTABILITY_H
#define incl_THRIFT_MUTEX_PORTABILITY_H

#ifdef THRIFT_MUTEX_EMULATE_PTHREAD_TIMEDLOCK

/* Backport of pthread_*_timed*lock() */
template <class MutexType>
inline int thrift_pthread_timedlock_impl(
    MutexType* mutex,
    const timespec* ts,
    int (*lockfunc)(MutexType*),
    int (*trylockfunc)(MutexType*)) {
  if (!ts || ((ts->tv_sec == 0) && (ts->tv_nsec == 0))) {
    return lockfunc(mutex);
  }

  timespec waiting = *ts;
  timespec delay;

  // Use a wait of 1/20th the total time to block between trylock calls.
  delay.tv_sec = waiting.tv_sec / 20;
  delay.tv_nsec = ((waiting.tv_sec % 20) * 50000000) + (waiting.tv_nsec / 20);

  do {
    auto ret = trylockfunc(mutex);
    if (ret != EBUSY) {
      return ret;
    }

    if ((delay.tv_sec > waiting.tv_sec) ||
        ((delay.tv_sec == waiting.tv_sec) &&
         (delay.tv_nsec > waiting.tv_nsec))) {
      // Less than delay-time left to wait
      delay = waiting;
    }
    timespec rem;
    if (nanosleep(&delay, &rem)) {
      // nanosleep was interrupted,
      // put the remainder back on before subtracting the delay
      if (1000000000 - waiting.tv_nsec < rem.tv_nsec) {
        ++waiting.tv_sec;
        waiting.tv_nsec = rem.tv_nsec - (1000000000 - waiting.tv_nsec);
      } else {
        waiting.tv_nsec += rem.tv_nsec;
      }
      waiting.tv_sec += rem.tv_sec;
    }
    if (delay.tv_nsec > waiting.tv_nsec) {
      --waiting.tv_sec;
      waiting.tv_nsec = 1000000000 - (delay.tv_nsec - waiting.tv_nsec);
    } else {
      waiting.tv_nsec -= delay.tv_nsec;
    }
    waiting.tv_sec -= delay.tv_sec;
  } while ((waiting.tv_sec > 0) || (waiting.tv_nsec > 0));
  return EBUSY;
}

inline int pthread_mutex_timedlock(pthread_mutex_t* mutex, const timespec* ts) {
  return thrift_pthread_timedlock_impl(
      mutex, ts, pthread_mutex_lock, pthread_mutex_trylock);
}

inline int pthread_rwlock_timedwrlock(
    pthread_rwlock_t* rwlock, const timespec* ts) {
  return thrift_pthread_timedlock_impl(
      rwlock, ts, pthread_rwlock_wrlock, pthread_rwlock_trywrlock);
}

inline int pthread_rwlock_timedrdlock(
    pthread_rwlock_t* rwlock, const timespec* ts) {
  return thrift_pthread_timedlock_impl(
      rwlock, ts, pthread_rwlock_rdlock, pthread_rwlock_tryrdlock);
}
#endif // THRIFT_MUTEX_EMULATE_PTHREAD_TIMEDLOCK

#endif // incl_THRIFT_MUTEX_PORTABILITY_H
