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

#include "hphp/util/portability.h"
#include "hphp/runtime/base/memory-manager.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/*
 * Objects that need to do special clean up at the end of the request
 * may register themselves for this by deriving from Sweepable.  After
 * every request, Sweepable::SweepAll is called so the objects may
 * clear out request-local allocations that are not smart-allocated.
 */
struct Sweepable: private boost::noncopyable {
  struct Node {
    Node *next, *prev;
    void enlist(Node& head);
    void delist();
    void init();
  };
  static unsigned SweepAll();
  static void InitList();

  /*
   * There is no default behavior. Make sure this function frees all
   * NON-SMART-ALLOCATED resources ONLY.
   */
  virtual void sweep() = 0;

  /*
   * Remove this object from the sweepable list, so it won't have
   * sweep() called at the next SweepAll.
   */
  void unregister();

protected:
  Sweepable();
  virtual ~Sweepable();

private:
  Node m_sweepNode;
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
