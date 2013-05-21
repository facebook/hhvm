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
#include "hphp/util/synchronizable.h"

#include "hphp/util/compatibility.h"
#include "hphp/util/rank.h"
#include "hphp/util/timer.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

Synchronizable::Synchronizable() : m_mutex(RankLeaf) {
  pthread_cond_init(&m_cond, nullptr);
}

Synchronizable::~Synchronizable() {
  pthread_cond_destroy(&m_cond);
}

void Synchronizable::wait() {
  UNUSED int ret = pthread_cond_wait(&m_cond, &m_mutex.getRaw());
  assert(ret != EPERM); // did you lock the mutex?
}

bool Synchronizable::wait(long seconds) {
  return wait(seconds, 0);
}

bool Synchronizable::wait(long seconds, long long nanosecs) {
  struct timespec ts;
  Timer::GetRealtimeTime(ts);
  ts.tv_sec += seconds;
  ts.tv_nsec += nanosecs;

  int ret = pthread_cond_timedwait(&m_cond, &m_mutex.getRaw(), &ts);
  assert(ret != EPERM); // did you lock the mutex?

  return ret != ETIMEDOUT;
}

void Synchronizable::notify() {
  pthread_cond_signal(&m_cond);
}

void Synchronizable::notifyAll() {
  pthread_cond_broadcast(&m_cond);
}

///////////////////////////////////////////////////////////////////////////////
}
