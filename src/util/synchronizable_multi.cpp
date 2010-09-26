/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include "synchronizable_multi.h"
#include "compatibility.h"

using namespace std;

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

SynchronizableMulti::SynchronizableMulti(int size) {
  ASSERT(size > 0);
  m_conds.resize(size);
  for (unsigned int i = 0; i < m_conds.size(); i++) {
    pthread_cond_init(&m_conds[i], NULL);
  }
}

SynchronizableMulti::~SynchronizableMulti() {
  for (unsigned int i = 0; i < m_conds.size(); i++) {
    pthread_cond_destroy(&m_conds[i]);
  }
}

void SynchronizableMulti::wait(int id, bool front) {
  waitImpl(id, front, NULL);
}

bool SynchronizableMulti::wait(int id, bool front, long seconds) {
  return wait(id, front, seconds, 0);
}

bool SynchronizableMulti::wait(int id, bool front, long seconds,
                               long long nanosecs) {
  struct timespec ts;
  gettime(ts);
  ts.tv_sec += seconds;
  ts.tv_nsec += nanosecs;
  return waitImpl(id, front, &ts);
}

bool SynchronizableMulti::waitImpl(int id, bool front, timespec *ts) {
  ASSERT(id >= 0);
  int index = id % m_conds.size();
  pthread_cond_t *cond = &m_conds[index];

  if (front) {
    m_cond_list.push_front(cond);
  } else {
    m_cond_list.push_back(cond);
  }

  int ret;
  if (ts) {
    ret = pthread_cond_timedwait(cond, &m_mutex.getRaw(), ts);
  } else {
    ret = pthread_cond_wait(cond, &m_mutex.getRaw());
  }
  ASSERT(ret != EPERM); // did you lock the mutex?

  if (ret) {
    for (std::list<pthread_cond_t*>::iterator iter = m_cond_list.begin();
         iter != m_cond_list.end(); ++iter) {
      if (*iter == cond) {
        m_cond_list.erase(iter);
        break;
      }
    }
  }

  return ret != ETIMEDOUT;
}

void SynchronizableMulti::notify() {
  if (!m_cond_list.empty()) {
    pthread_cond_signal(m_cond_list.front());
    m_cond_list.pop_front();
  }
}

void SynchronizableMulti::notifyAll() {
  while (!m_cond_list.empty()) {
    pthread_cond_signal(m_cond_list.front());
    m_cond_list.pop_front();
  }
}

///////////////////////////////////////////////////////////////////////////////
}
