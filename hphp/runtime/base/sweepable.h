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
class Sweepable {
  /*
   * Sweepable objects are not supposed to be copied or assigned
   * naively.
   */
  Sweepable(const Sweepable&) = delete;
  Sweepable& operator=(const Sweepable&) = delete;

public:
  struct Node {
    Node *next, *prev;
    void enlist(Node& head);
    void delist();
    void init();
  };
  static unsigned SweepAll();
  static void InitList();
  static void FlushList();

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
  ~Sweepable();

private:
  Node m_sweepNode;
};

using Sweeper = void (*)(ObjectData*);
void registerSweepableObj(ObjectData* obj, Sweeper);
void unregisterSweepableObj(ObjectData* obj);

/*
 * Nonvirtual sweepable mixin for use with ObjectData. State is managed
 * completely in a thread-local hashtable.
 */
template<class Base> struct SweepableObj: Base {
protected:
  template<class... Args>
  explicit SweepableObj(Sweeper f, Args&&... args)
    : Base(std::forward<Args>(args)...) {
    registerSweepableObj(this, f);
    static_assert(sizeof(*this) == sizeof(Base), "");
  }
  ~SweepableObj() { unregister(); }
  void unregister() { unregisterSweepableObj(this); }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif
