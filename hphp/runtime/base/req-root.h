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

#ifndef incl_HPHP_REQ_ROOT_H_
#define incl_HPHP_REQ_ROOT_H_

#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/runtime-option.h"
#include <utility>

namespace HPHP { namespace req {

/*
 * An explicitly-tracked root, registered on construction and de-registered
 * on destruction. Subclasses of this implement tyindex() and detach() methods
 * so instances can be scanned as needed.
 * Warning: this extra tracking is expensive, and only necessary when
 * creating and destroying heap pointers in areas not already known
 * as roots (thread locals, stack, rds, ExecutionContext, etc).
 */
struct root_handle {
  static constexpr uint32_t INVALID = ~0;

  root_handle(uint16_t size, type_scan::Index tyindex)
    : m_id(addRootHandle()), m_size(size), m_tyindex(tyindex)
  {}

  // move construction takes over the old handle's id.
  root_handle(root_handle&& h) noexcept
    : m_id(h.m_id != INVALID ? stealRootHandle(&h) : INVALID)
    , m_size(h.m_size)
    , m_tyindex(h.m_tyindex)
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
  virtual void scan(type_scan::Scanner&) const = 0;
  virtual void detach() = 0;

  template<class Fn> void iterate(Fn fn) const {
    fn(this, m_size, m_tyindex);
  }
private:
  uint32_t addRootHandle();
  uint32_t stealRootHandle(root_handle*);
  void delRootHandle();
private:
  uint32_t m_id;
protected:
  const uint16_t m_size;
  const type_scan::Index m_tyindex;
};

// e.g. req::root<Array>, req::root<Variant>
template<class T>
struct root : T, root_handle {
  root() : T(),
    root_handle(sizeof(root<T>), type_scan::getIndexForScan<root<T>>())
  {
    static_assert(sizeof(root<T>) <= 0xffff, "");
  }

  // copy constructor
  root(const root<T>& r)
    : T(static_cast<const T&>(r)),
      root_handle(sizeof(root<T>), type_scan::getIndexForScan<root<T>>())
  {
    static_assert(sizeof(root<T>) <= 0xffff, "");
  }

  // conversion constructor
  template<class S> /* implicit */ root(const S& s)
    : T(s),
      root_handle(sizeof(root<T>), type_scan::getIndexForScan<root<T>>())
  {
    static_assert(sizeof(root<T>) <= 0xffff, "");
  }

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
    : T(std::move(t)),
      root_handle(sizeof(root<T>), type_scan::getIndexForScan<root<T>>())
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
  void scan(type_scan::Scanner&) const override;
  void detach() override;
};

template<> void root<TypedValue>::scan(type_scan::Scanner&) const;
template<> void root<TypedValue>::detach();

}}
#endif
