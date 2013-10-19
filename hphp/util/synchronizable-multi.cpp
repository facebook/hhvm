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

#include "hphp/util/synchronizable-multi.h"
#include "hphp/util/compatibility.h"
#include "hphp/util/timer.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

SynchronizableMulti::SynchronizableMulti(int size, int groups) :
    m_mutex(RankLeaf), m_group(0) {
  assert(size > 0 && groups > 0);
  m_conds.resize(size);
  for (unsigned int i = 0; i < m_conds.size(); i++) {
    pthread_cond_init(&m_conds[i], nullptr);
  }
  m_cond_list_vec.resize(groups);
}

SynchronizableMulti::~SynchronizableMulti() {
  for (unsigned int i = 0; i < m_conds.size(); i++) {
    pthread_cond_destroy(&m_conds[i]);
  }
}

void SynchronizableMulti::wait(int id, int q, bool front) {
  waitImpl(id, q, front, nullptr);
}

bool SynchronizableMulti::wait(int id, int q, bool front, long seconds) {
  return wait(id, q, front, seconds, 0);
}

bool SynchronizableMulti::wait(int id, int q, bool front, long seconds,
                               long long nanosecs) {
  struct timespec ts;
  Timer::GetRealtimeTime(ts);
  ts.tv_sec += seconds;
  ts.tv_nsec += nanosecs;
  return waitImpl(id, q, front, &ts);
}

bool SynchronizableMulti::waitImpl(int id, int q, bool front, timespec *ts) {
  assert(id >= 0);
  int index = id % m_conds.size();
  pthread_cond_t *cond = &m_conds[index];

  assert(q >= 0);
  auto& cond_list = m_cond_list_vec[q % m_cond_list_vec.size()];

  if (front) {
    cond_list.push_front(cond);
    m_cond_map[cond] = make_pair(&cond_list, cond_list.begin());
  } else {
    cond_list.push_back(cond);
    m_cond_map[cond] = make_pair(&cond_list, --cond_list.end());
  }

  int ret;
  if (ts) {
    ret = pthread_cond_timedwait(cond, &m_mutex.getRaw(), ts);
  } else {
    ret = pthread_cond_wait(cond, &m_mutex.getRaw());
  }
  assert(ret != EPERM); // did you lock the mutex?

  if (ret) {
    CondIterMap::iterator iter = m_cond_map.find(cond);
    if (iter != m_cond_map.end()) {
      iter->second.first->erase(iter->second.second);
      m_cond_map.erase(iter);
    }
  }

  return ret != ETIMEDOUT;
}

void SynchronizableMulti::notify() {
  for (int i = 0, s = m_cond_list_vec.size(); i < s; i++) {
    assert(m_group < s);
    auto& cond_list = m_cond_list_vec[m_group++];
    if (m_group == s) m_group = 0;
    if (!cond_list.empty()) {
      pthread_cond_t *cond = cond_list.front();
      pthread_cond_signal(cond);
      cond_list.pop_front();
      m_cond_map.erase(cond);
      break;
    }
  }
}

void SynchronizableMulti::notifyAll() {
  for (auto& cond_list : m_cond_list_vec) {
    while (!cond_list.empty()) {
      pthread_cond_signal(cond_list.front());
      cond_list.pop_front();
    }
  }
  m_cond_map.clear();
}

///////////////////////////////////////////////////////////////////////////////
}
