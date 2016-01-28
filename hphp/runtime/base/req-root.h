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

#ifndef incl_HPHP_REQ_ROOT_H_
#define incl_HPHP_REQ_ROOT_H_

#include "hphp/runtime/base/typed-value.h"

namespace HPHP {
struct IMarker;
namespace req {

/*
 * An explicitly-tracked root. subclasses of this can implement vscan()
 * and detach() methods so instances can be registered on construction,
 * scanned as needed, and de-registered on destruction.
 * Warning: this extra tracking is expensive, and only necessary when
 * creating and destroying heap pointers in areas not already known
 * as roots (thread locals, stack, rds, ExecutionContext, etc).
 */
struct root_handle {
  static constexpr size_t INVALID = -1LL;
  root_handle() : m_id(addRootHandle()) {}

  // root_handles must all be unique, so the copy-constructor just
  // registers the new handle, and copy-assign is a nop.
  root_handle(const root_handle&) : m_id(addRootHandle()) {}
  root_handle& operator=(const root_handle&) {
    return *this;
  }
  // move construction takes over the old handle's id.
  root_handle(root_handle&& h) noexcept
    : m_id(stealRootHandle(&h))
  {}
  // copy-assignment poaches the old handle's id if necessary.
  root_handle& operator=(root_handle&& h) {
    if (m_id == INVALID) m_id = stealRootHandle(&h);
    return *this;
  }
  virtual ~root_handle() {
    if (m_id != INVALID) delRootHandle();
  }
  void invalidate() {
    m_id = INVALID;
    detach();
  }
  template<class F> void scan(F&) const;
  virtual void vscan(IMarker&) const = 0;
  virtual void detach() = 0;
private:
  size_t addRootHandle();
  size_t stealRootHandle(root_handle*);
  void delRootHandle();
private:
  size_t m_id;
};

// e.g. req::root<Array>, req::root<Variant>
template<class T>
struct root : T, root_handle {
  root() = default;
  /*implicit*/ root(const root<T>& other) = default;
  template<class S> explicit root(S s)
    : T(s)
  {}
  template<class S> root<T>& operator=(const S& p) {
    T::operator=(p);
    return *this;
  }
  void vscan(IMarker& mark) const override;
  void detach() override;
};

template<> void root<TypedValue>::vscan(IMarker& mark) const;
template<> void root<TypedValue>::detach();

}}
#endif
