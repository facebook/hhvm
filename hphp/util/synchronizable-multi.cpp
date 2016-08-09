/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/util/lock.h"
#include "hphp/util/rank.h"
#include "hphp/util/timer.h"

#ifndef _MSC_VER
#include <sys/errno.h>
#endif

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

SynchronizableMulti::SynchronizableMulti(int size) :
  m_mutex(RankLeaf), m_group(0), m_conds(size), m_cond_list_vec(1) {
  assert(size > 0);
}

inline
bool SynchronizableMulti::waitImpl(int id, int q, Priority pri, timespec *ts) {
  assert(id >= 0 && id < m_conds.size());
  auto& cond = m_conds[id];

  assert(q >= 0);
  const uint32_t num_lists = m_cond_list_vec.size();
  uint32_t list_index = 0;
  if (num_lists == 2) list_index = q & 1;
  else if (num_lists > 2) list_index = q % num_lists;
  auto& cond_list = m_cond_list_vec[list_index];

  if (pri == Priority::High) {
    cond_list.push_front(cond);
  } else if (pri == Priority::Middle) {
    cond_list.push_middle(cond);
  } else {
    assert(pri == Priority::Low);
    cond_list.push_back(cond);
  }

  int ret;
  if (ts) {
    ret = pthread_cond_timedwait(cond, &m_mutex.getRaw(), ts);
  } else {
    ret = pthread_cond_wait(cond, &m_mutex.getRaw());
  }
  assert(ret != EPERM); // did you lock the mutex?

  if (ret) {
    cond.unlink();
  }

  return ret != ETIMEDOUT;
}

void SynchronizableMulti::wait(int id, int q, Priority pri) {
  waitImpl(id, q, pri, nullptr);
}

bool SynchronizableMulti::wait(int id, int q, Priority pri, long seconds) {
  return wait(id, q, pri, seconds, 0);
}

bool SynchronizableMulti::wait(int id, int q, Priority pri, long seconds,
                               long long nanosecs) {
  struct timespec ts;
  Timer::GetRealtimeTime(ts);
  ts.tv_sec += seconds;
  ts.tv_nsec += nanosecs;
  return waitImpl(id, q, pri, &ts);
}

void SynchronizableMulti::setNumGroups(int num_groups) {
  Lock l(this);
  if (num_groups != m_cond_list_vec.size()) {
    assert(num_groups > m_cond_list_vec.size());
    m_cond_list_vec.resize(num_groups);
  }
}

void SynchronizableMulti::notify() {
  for (int i = 0, s = m_cond_list_vec.size(); i < s; i++) {
    assert(m_group < s);
    auto& cond_list = m_cond_list_vec[m_group++];
    if (m_group == s) m_group = 0;
    if (!cond_list.empty()) {
      auto& cond = cond_list.front();
      pthread_cond_signal(cond);
      cond_list.pop_front();
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
}

///////////////////////////////////////////////////////////////////////////////
}
