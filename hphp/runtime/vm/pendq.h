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

#ifndef incl_HPHP_PENDQ_H_
#define incl_HPHP_PENDQ_H_

#include <queue>

namespace HPHP {

/*
 * We sometimes have work to do that must be deferred to a lower-ranked
 * context. Do so via this work queue.
 *
 * Clients can derive from DeferredWorkItem. operator() does the work that
 * needs doing from base rank.
 */
struct DeferredWorkItem {
  virtual void operator()() = 0;
  virtual ~DeferredWorkItem() { }
};

template<typename T>
struct DeferredDeleter : public DeferredWorkItem {
  DeferredDeleter(T* obj) : m_obj(obj) {}
  virtual void operator()() {
    delete m_obj;
  }
private:
  T* m_obj;
};

struct PendQ : private std::queue<DeferredWorkItem*> {
  static void defer(DeferredWorkItem* i);
  static void drain();
};

}

#endif

