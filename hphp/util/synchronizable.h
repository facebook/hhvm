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

#ifndef incl_HPHP_SYNCHRONIZABLE_H_
#define incl_HPHP_SYNCHRONIZABLE_H_

#include "hphp/util/mutex.h"
#include <pthread.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Java-like base class for synchronization between object methods within the
 * same class. Check pool.h for a typical example.
 */
class Synchronizable {
 public:
  Synchronizable();
  virtual ~Synchronizable();

  void wait();
  bool wait(long seconds); // false if timed out
  bool wait(long seconds, long long nanosecs); // false if timed out
  void notify();
  void notifyAll();

  Mutex &getMutex() { return m_mutex;}

 private:
  Mutex m_mutex;
  pthread_cond_t m_cond;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_SYNCHRONIZABLE_H_
