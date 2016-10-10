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
#include "hphp/runtime/base/runtime-option.h"
#include <utility>

namespace HPHP {
struct IMarker;
namespace req {

/*
 * An explicitly-tracked root, registered on construction and de-registered
 * on destruction. Subclasses of this implement vscan() and detach() methods
 * so instances can be scanned as needed.
 * Warning: this extra tracking is expensive, and only necessary when
 * creating and destroying heap pointers in areas not already known
 * as roots (thread locals, stack, rds, ExecutionContext, etc).
 */
struct root_handle {
  static constexpr auto INVALID = ~size_t(0);

  root_handle()
    : m_id(addRootHandle())
  {}

  // move construction takes over the old handle's id.
  root_handle(root_handle&& h) noexcept
    : m_id(h.m_id != INVALID ? stealRootHandle(&h) : INVALID)
  {}

  // move-assignment poaches the old handle's id if necessary.
  root_handle& operator=(root_handle&& h) {
    if (m_id == INVALID) m_id = stealRootHandle(&h);
    return *this;
  }

  // root_handles must all be unique; remove copy-construct and copy-assign
  root_handle(const root_handle&) = delete;
  root_handle& operator=(const root_handle&) = delete;

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
  root() : T(), root_handle() {}

  // copy constructor
  root(const root<T>& r)
    : T(static_cast<const T&>(r)), root_handle()
  {}

  // conversion constructor
  template<class S> /* implicit */ root(const S& s)
    : T(s), root_handle()
  {}

  // copy assign
  root<T>& operator=(const root<T>& r) {
    T::operator=(static_cast<const T&>(r));
    return *this;
  }

  // converting assign from any type only deals with T
  template<class S> root<T>& operator=(const S& s) {
    T::operator=(s);
    return *this;
  }

  // move ctors
  root(root<T>&& r) noexcept
    : T(std::move(r)), root_handle(std::move(r))
  {}
  /* implicit */ root(T&& t) noexcept
    : T(std::move(t)), root_handle()
  {}

  // move assign
  root<T>& operator=(root<T>&& r) {
    T::operator=(std::move(static_cast<T&&>(r)));
    root_handle::operator=(std::move(r));
    return *this;
  }
  root<T>& operator=(T&& t) {
    T::operator=(std::move(t));
    return *this;
  }

  // implement root_handle
  void vscan(IMarker& mark) const override;
  void detach() override;
};

template<> void root<TypedValue>::vscan(IMarker& mark) const;
template<> void root<TypedValue>::detach();

}}
#endif
