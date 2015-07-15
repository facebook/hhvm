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

#ifndef incl_HPHP_RESOURCE_H_
#define incl_HPHP_RESOURCE_H_

#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/base/resource-data.h"
#include "hphp/runtime/base/req-ptr.h"
#include "hphp/runtime/base/sweepable.h"

#include <algorithm>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

#define null_resource Resource::s_nullResource

/**
 * Object type wrapping around ObjectData to implement reference count.
 */
class Resource {
  req::ptr<ResourceData> m_res;

public:
  Resource() {}

  static const Resource s_nullResource;

  void reset() { m_res.reset(); }

  ResourceData* operator->() const { return m_res.get(); }

  /**
   * Constructors
   */
  /* implicit */ Resource(ResourceData *data) : m_res(data) { }
  /* implicit */ Resource(const Resource& src) : m_res(src.m_res) { }
  template <typename T>
  explicit Resource(req::ptr<T>&& src) : m_res(std::move(src)) { }
  template <typename T>
  explicit Resource(const req::ptr<T>& src) : m_res(src) { }

  // Move ctor
  Resource(Resource&& src) noexcept : m_res(std::move(src.m_res)) { }

  // Regular assign
  Resource& operator=(const Resource& src) {
    m_res = src.m_res;
    return *this;
  }
  template <typename T>
  Resource& operator=(const req::ptr<T>& src) {
    m_res = src;
    return *this;
  }
  // Move assign
  Resource& operator=(Resource&& src) {
    m_res = std::move(src.m_res);
    return *this;
  }
  template <typename T>
  Resource& operator=(req::ptr<T>&& src) {
    m_res = std::move(src);
    return *this;
  }

  ~Resource();

  /**
   * Informational
   */
  explicit operator bool() const { return (bool)m_res; }

  bool isNull() const {
    return m_res == nullptr;
  }

  bool isInvalid() const {
    return m_res == nullptr || m_res->isInvalid();
  }

  /**
   * getTyped() and is() are intended for use with C++ classes that derive
   * from ResourceData.
   *
   * Prefer using the following functions instead of getTyped:
   * r.getTyped<T>(false, false) -> cast<T>(r)
   * r.getTyped<T>(true,  false) -> cast_or_null<T>(r)
   * r.getTyped<T>(false, true) -> dyn_cast<T>(r)
   * r.getTyped<T>(true,  true) -> dyn_cast_or_null<T>(r)
   */
  template<typename T>
  [[deprecated("Please use one of the cast family of functions instead.")]]
  req::ptr<T> getTyped(bool nullOkay = false, bool badTypeOkay = false) const {
    static_assert(std::is_base_of<ResourceData, T>::value, "");
    if (nullOkay) {
      return badTypeOkay ? dyn_cast_or_null<T>(m_res) : cast_or_null<T>(m_res);
    }
    return badTypeOkay ? dyn_cast<T>(m_res) : cast<T>(m_res);
  }

  template<typename T>
  bool is() const { return isa<T>(m_res); }

  /**
   * Type conversions
   */
  bool   toBoolean() const { return m_res ? m_res->o_toBoolean() : false;}
  char   toByte   () const { return m_res ? m_res->o_toInt64() : 0;}
  short  toInt16  () const { return m_res ? m_res->o_toInt64() : 0;}
  int    toInt32  () const { return m_res ? m_res->o_toInt64() : 0;}
  int64_t toInt64 () const { return m_res ? m_res->o_toInt64() : 0;}
  double toDouble () const { return m_res ? m_res->o_toDouble() : 0;}
  String toString () const;
  Array  toArray  () const;

  /**
   * Comparisons
   */
  bool same (const Resource& v2) const { return m_res == v2.m_res; }
  bool equal(const Resource& v2) const { return m_res == v2.m_res; }
  bool less(const Resource& v2) const { return toInt64() < v2.toInt64(); }
  bool more(const Resource& v2) const { return toInt64() > v2.toInt64(); }

private:
  //
  // The deref and detach functions are only for use by the heap tracer,
  // cast functions and Variant.  They should not be used anywhere else.
  //
  template <typename T>
  friend typename std::enable_if<
    std::is_base_of<ResourceData,T>::value,
    ResourceData*
  >::type deref(const Resource& r) { return r.m_res.get(); }

  template <typename T>
  friend typename std::enable_if<
    std::is_base_of<ResourceData,T>::value,
    ResourceData*
  >::type detach(Resource&& r) { return r.m_res.detach(); }

  static void compileTimeAssertions();

  const char* classname_cstr() const;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_RESOURCE_DATA_H_
