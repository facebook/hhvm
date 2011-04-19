/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef __SYNCHRONIZABLE_MULTI_H__
#define __SYNCHRONIZABLE_MULTI_H__

#include "base.h"
#include "mutex.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * A Synchronizable object that has multiple conditional variables. The benefit
 * is, notify() can choose to notify the most recently waited conditional
 * variable for an altered scheduling that potentially wakes up a thread with
 * better thread caching.
 */
class SynchronizableMulti {
public:
  SynchronizableMulti(int size);
  virtual ~SynchronizableMulti();

  /**
   * "id" is an arbitrary number that locates the conditional variable to wait
   * on.
   *
   * "front" means adding this thread to front of the queue while waiting.
   * Otherwise, the thread is pushed to the back of the queue, being the last
   * to wake up, when notify() is called.
   */
  void wait(int id, bool front);
  bool wait(int id, bool front, long seconds); // false if timed out
  bool wait(int id, bool front, long seconds, long long nanosecs);
  void notify();
  void notifyAll();

  Mutex &getMutex() { return m_mutex;}

 private:
  Mutex m_mutex;
  std::vector<pthread_cond_t> m_conds;
  std::list<pthread_cond_t*> m_cond_list;

  // iterators in std::list are valid even after element removal
  typedef hphp_hash_map<pthread_cond_t*, std::list<pthread_cond_t*>::iterator,
                        pointer_hash<pthread_cond_t> > CondIterMap;
  CondIterMap m_cond_map;

  bool waitImpl(int id, bool front, timespec *ts);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __SYNCHRONIZABLE_MULTI_H__
