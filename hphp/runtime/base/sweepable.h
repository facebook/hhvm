/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_SWEEPABLE_H_
#define incl_HPHP_SWEEPABLE_H_

#include <boost/noncopyable.hpp>

#include "hphp/util/portability.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/*
 * Objects that need to do special clean up at the end of the request
 * may register themselves for this by deriving from Sweepable.  After
 * every request, MemoryManager::sweep() called each Sweepable::sweep()
 * method, allowing objects to clean up resources that are not othewise
 * owned by the current request, for example malloc'd-memory or file handles.
 */
struct Sweepable: private boost::noncopyable {

  /*
   * There is no default behavior. Make sure this function frees all
   * only non-request-allocated resources.
   */
  virtual void sweep() = 0;

  /*
   * Remove this object from the sweepable list, so it won't have
   * sweep() called at the next SweepAll.
   */
  void unregister() {
    delist();
    init(); // in case destructor runs later.
  }

  /*
   * List manipulation methods; mainly for use by MemoryManager.
   */
  bool empty() const {
    assert((this == m_prev) == (this == m_next)); // both==this or both!=this
    return this == m_next;
  }
  void init() { m_prev = m_next = this; }
  Sweepable* next() const { return m_next; }

  void delist() {
    auto n = m_next, p = m_prev;
    n->m_prev = p;
    p->m_next = n;
  }

  void enlist(Sweepable* head) {
    auto next = head->m_next;
    m_next = next;
    m_prev = head;
    head->m_next = next->m_prev = this;
  }

protected:
  Sweepable();
  enum class Init {};
  explicit Sweepable(Init) { init(); }
  virtual ~Sweepable() { delist(); }

private:
  Sweepable *m_next, *m_prev;
};

/*
 * SweepableMember is a Sweepable used as a member of an otherwise nonvirtual
 * class. The member must be named m_sweepable. If T is a derived class, it
 * should only have one m_sweepable member. Anything fancier than that voids
 * your warranty.
 */
template<class T>
struct SweepableMember: Sweepable {
  void sweep() {
    auto obj = reinterpret_cast<T*>(
      uintptr_t(this) - offsetof(T, m_sweepable)
    );
    obj->sweep();
  };
};

///////////////////////////////////////////////////////////////////////////////
}

#endif
