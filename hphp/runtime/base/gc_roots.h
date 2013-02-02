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

#ifndef incl_BASE_GC_ROOTS_H_
#define incl_BASE_GC_ROOTS_H_

#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/static_assert.hpp>
#include "util/base.h"
#include "util/util.h"
#include "runtime/base/util/smart_ptr.h"
#include "runtime/base/types.h"

namespace HPHP {

#ifdef HHVM_GC

//////////////////////////////////////////////////////////////////////

/*
 * Variants aren't smart pointers and behave a bit differently from
 * the others.  This base is used to change that behavior based on
 * type.
 *
 * TODO: right now we are "over-tracking" Variants, since we still
 * track the ones allocated inside smart_allocators.
 */
template<class T> struct GCRootTrackerBase {
  typedef SmartPtr<T> type;
};
template<> struct GCRootTrackerBase<Variant> {
  template<class T>
  struct EmptyBase {
    explicit EmptyBase(T* = 0) {}
  };

  typedef EmptyBase<Variant> type;
};

/*
 * This is a base class for smart pointers that need to be tracked as
 * C++ roots for tracing GC.
 *
 * The roots are tracked differently depending on whether they live on
 * the stack or on the heap.  For roots on the stack, pointers are
 * stored in a sorted array, for the heap they currently reside in a
 * hash set.
 */
template<class T>
class GCRootTracker : protected GCRootTrackerBase<T>::type {
  typedef typename GCRootTrackerBase<T>::type Base;

public:
  BOOST_STATIC_ASSERT((boost::is_same<T,ObjectData>::value ||
                       boost::is_same<T,StringData>::value ||
                       boost::is_same<T,ArrayData>::value ||
                       boost::is_same<T,Variant>::value));

  typedef hphp_hash_set<void*,pointer_hash<void> > HeapSet;

  explicit GCRootTracker(T* t = 0) : Base(t) { track(); }
  GCRootTracker(const GCRootTracker& o) : Base(o) {
    track();
  }
  ~GCRootTracker() { untrack(); }

  GCRootTracker& operator=(const GCRootTracker& o) {
    Base::operator=(o);
    return *this;
  }

  GCRootTracker& operator=(T* t) {
    Base::operator=(t);
    return *this;
  }

  static std::vector<void*>& getStack() {
    static __thread std::vector<void*>* stack;
    if (!stack) stack = new std::vector<void*>();
    return *stack;
  }

  static HeapSet& getHeap() {
    static __thread HeapSet* heap;
    if (!heap) heap = new HeapSet();
    return *heap;
  }

  // Discards all pointers below a given address.  (This is currently
  // used in the bridge with ext_hhvm.cpp; see vm/gc.h's
  // DiscardDeepRoots().)
  static void discardDeepRoots(char* frontier) {
    std::vector<void*>& stack = getStack();
    while (!stack.empty() && stack.back() < frontier) {
      stack.pop_back();
    }
  }

  /*
   * Discard all known roots.  This is called at sweep() time in
   * memory_manager, since many String/Array/etc objects will not have
   * their destructors run.
   */
  static void clear() {
    getStack().clear();
    getHeap().clear();
  }

private:
  void track() {
    if (!is_stack_ptr(this)) return trackHeap();
    trackStack();
  }

  void untrack() {
    if (!is_stack_ptr(this)) return untrackHeap();
    untrackStack();
  }

  void trackHeap() {
    getHeap().insert(this);
  }

  void untrackHeap() {
    getHeap().erase(this);
  }

private:
  struct InvariantChecker : boost::noncopyable {
    explicit InvariantChecker(GCRootTracker* thiz)
      : m_thiz(thiz)
    {
      m_thiz->checkInvariants();
    }
    ~InvariantChecker()
    {
      m_thiz->checkInvariants();
    }
    GCRootTracker* m_thiz;
  };

  void checkInvariants() {
    if (!debug) return;

    const std::vector<void*>& stack = getStack();
    std::vector<void*>::const_reverse_iterator it = stack.rbegin();
    void* last = 0;
    for (; it != stack.rend(); ++it) {
      assert(*it > last);
      last = *it;
    }
  }

  void trackStack() {
    InvariantChecker check(this);

    std::vector<void*>& stack = getStack();
    if (stack.empty()) {
      stack.push_back(this);
      return;
    }

    std::vector<void*>::iterator it = stack.end() - 1;
    while (it != stack.begin() && *it < this) {
      --it;
    }
    if (it == stack.begin()) {
      if (*it < this) {
        stack.insert(stack.begin(), this);
        return;
      }
    }

    assert(*it > this);
    ++it;
    assert(it == stack.end() || *it < this);
    stack.insert(it, this);
  }

  void untrackStack() {
    InvariantChecker check(this);

    /*
     * Note that objects on the C++ stack are not necessarily
     * destroyed in reverse order of their creation, so this search to
     * find the correct this pointer is necessary.
     *
     * For example, assume NRVO doesn't happen in the following:
     *
     *    extern String f();
     *    String s(f());
     *
     * Then the return value of f is constructed, followed by s being
     * copy-constructed from it, then ~f happens before ~s.
     */
    std::vector<void*>& stack = getStack();
    std::vector<void*>::reverse_iterator it = stack.rbegin();
    while (it != stack.rend() && *it < this) {
      ++it;
    }
    if (it != stack.rend() && *it == this) {
      stack.erase(it.base() - 1);
    }
  }
};

//////////////////////////////////////////////////////////////////////

/*
 * In some cases we want to register roots on the stack without all
 * the machinery of the smart pointers that use GCRootTracker<> above.
 * These roots must be popped in the opposite order they are pushed,
 * so this is mainly appropriate for holding a root for some C++
 * lexical scope.
 *
 * The specific cases where this can be necessary is during creation,
 * of smart allocated data, where the object has a ref count of zero
 * and no root yet lives anywhere, but a GC may happen during the
 * constructor if it makes allocations.  (See for e.g. the
 * VM::Instance constructor.)
 *
 * To use this class, use DECLARE_STACK_GC_ROOT() instead of
 * instantiating it directly.
 */
template<class T>
struct GCRoot : private boost::noncopyable {
  BOOST_STATIC_ASSERT((boost::is_same<T,ObjectData>::value ||
                       boost::is_same<T,StringData>::value ||
                       boost::is_same<T,ArrayData>::value ||
                       boost::is_same<T,Variant>::value));

  static std::vector<T*>& getStack() {
    static __thread std::vector<T*>* stack;
    if (!stack) stack = new std::vector<T*>();
    return *stack;
  }

  static void clear() {
    // Nothing should be left at end of request time or this class is
    // being misused.
    assert(getStack().empty());
  }

#ifdef DEBUG
  explicit GCRoot(T* t) : m_debugSavedPtr(t) { getStack().push_back(t); }
  ~GCRoot() {
    assert(!getStack().empty() && getStack().back() == m_debugSavedPtr);
    getStack().pop_back();
  }

private:
  T* const m_debugSavedPtr;
#else
  // Non debug version:
  explicit GCRoot(T* t) { getStack().push_back(t); }
  ~GCRoot() { getStack().pop_back(); }
#endif
};

#define DECLARE_STACK_GC_ROOT(type, root) \
  GCRoot<type> gcr##__LINE__((root))

//////////////////////////////////////////////////////////////////////

#else

//////////////////////////////////////////////////////////////////////

#define DECLARE_STACK_GC_ROOT(type, root) /* */

#endif

}

#endif
