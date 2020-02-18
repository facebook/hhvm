/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#ifndef incl_HPHP_SYNCHRONIZABLE_MULTI_H_
#define incl_HPHP_SYNCHRONIZABLE_MULTI_H_

#include "hphp/util/mutex.h"
#include <pthread.h>
#include <folly/IntrusiveList.h>
#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * A Synchronizable object that has multiple conditional variables. The benefit
 * is, notify() can choose to wake up a thread that is more favorable (e.g.,
 * one with stack/heap mapped on huge pages, or one that is recently active).
 */
struct SynchronizableMulti {
  explicit SynchronizableMulti(int size);
  virtual ~SynchronizableMulti() {}

  /*
   * Threads are notified based on their priority. The priority is decided when
   * calling wait().
   */
  enum Priority {
    Highest,
    High,
    Normal,
    Low
  };
  /**
   * "id" is an arbitrary number that locates the conditional variable to wait
   * on.
   */
  void wait(int id, int q, Priority pri);
  bool wait(int id, int q, Priority pri, long seconds); // false if timed out
  bool wait(int id, int q, Priority pri, long seconds, long long nanosecs);
  void notifySpecific(int id);
  void notify();
  void notifyAll();
  void setNumGroups(int num_groups);

  Mutex &getMutex() { return m_mutex;}

 private:
  Mutex m_mutex;
  int m_group;

  struct alignas(64) CondVarNode {
    pthread_cond_t m_cond;
    folly::IntrusiveListHook m_listHook;

    CondVarNode() {
      pthread_cond_init(&m_cond, nullptr);
    }
    ~CondVarNode() {
      pthread_cond_destroy(&m_cond);
    }
    /* implicit */ operator pthread_cond_t*() {
      return &m_cond;
    }
    void unlink() {
      if (m_listHook.is_linked()) m_listHook.unlink();
    }
  };

  std::vector<CondVarNode> m_conds;

  // List that supports four priorities, implemented using two intrusive lists.
  struct alignas(64) CondVarList {
    CondVarList() = default;

    bool empty() const {
      return m_highPriList.empty() && m_midLowPriList.empty();
    }
    CondVarNode& front() {
      if (!m_highPriList.empty()) return m_highPriList.front();
      return m_midLowPriList.front();
    }
    void pop_front() {
      if (!m_highPriList.empty()) m_highPriList.pop_front();
      else m_midLowPriList.pop_front();
    }

    void push(CondVarNode& c, SynchronizableMulti::Priority pri) {
      c.unlink();
      if (pri == Priority::Highest) {
        m_highPriList.push_front(c);
      } else if (pri == Priority::High) {
        m_highPriList.push_back(c);
      } else if (pri == Priority::Normal) {
        m_midLowPriList.push_front(c);
      } else {
        assertx(pri == Priority::Low);
        m_midLowPriList.push_back(c);
      }
    }

   private:
    using CondVarIList =
      folly::IntrusiveList<CondVarNode, &CondVarNode::m_listHook>;
    CondVarIList m_highPriList;
    CondVarIList m_midLowPriList;
  };
  std::vector<CondVarList> m_cond_list_vec;

  bool waitImpl(int id, int q, Priority pri, timespec *ts);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_SYNCHRONIZABLE_MULTI_H_
