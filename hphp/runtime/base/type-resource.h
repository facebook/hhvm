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

#include "hphp/runtime/base/resource-data.h"
#include "hphp/runtime/base/smart-ptr.h"
#include "hphp/runtime/base/sweepable.h"

#include <algorithm>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

#define null_resource Resource::s_nullResource

/**
 * Object type wrapping around ObjectData to implement reference count.
 */
class Resource {
  SmartPtr<ResourceData> m_res;

public:
  Resource() {}

  static const Resource s_nullResource;

  ResourceData* get() const { return m_res.get(); }
  void reset() { m_res.reset(); }

  ResourceData* operator->() const { return get(); }

  /**
   * Constructors
   */
  /* implicit */ Resource(ResourceData *data) : m_res(data) { }
  /* implicit */ Resource(const Resource& src) : m_res(src.m_res) { }
  template <typename T>
  explicit Resource(SmartPtr<T>&& src) : m_res(std::move(src)) { }
  template <typename T>
  explicit Resource(const SmartPtr<T>& src) : m_res(src) { }

  // Move ctor
  Resource(Resource&& src) noexcept : m_res(std::move(src.m_res)) { }

  // Regular assign
  Resource& operator=(const Resource& src) {
    m_res = src.m_res;
    return *this;
  }
  template <typename T>
  Resource& operator=(const SmartPtr<T>& src) {
    m_res = src;
    return *this;
  }
  // Move assign
  Resource& operator=(Resource&& src) {
    m_res = std::move(src.m_res);
    return *this;
  }
  template <typename T>
  Resource& operator=(SmartPtr<T>&& src) {
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
  T* getTyped(bool nullOkay = false, bool badTypeOkay = false) const {
    static_assert(std::is_base_of<ResourceData, T>::value, "");

    ResourceData *cur = get();
    if (!cur) {
      if (!nullOkay) {
        throw_null_pointer_exception();
      }
      return nullptr;
    }
    T *px = dynamic_cast<T*>(cur);
    if (!px) {
      if (!badTypeOkay) {
        throw_invalid_object_type(classname_cstr());
      }
      return nullptr;
    }

    // Assert that casting does not adjust the 'this' pointer
    assert((void*)px == (void*)cur);
    return px;
  }

  template<typename T>
  bool is() const {
    return dynamic_cast<T*>(get()) != nullptr;
  }

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
  bool same (const Resource& v2) const { return m_res == v2.get();}
  bool equal(const Resource& v2) const { return m_res == v2.get();}
  bool less (const Resource& v2) const { return toInt64() < v2.toInt64();}
  bool more (const Resource& v2) const { return toInt64() > v2.toInt64();}

  // Transfer ownership of our reference to this resource.
  ResourceData *detach() { return m_res.detach(); }
private:
  static void compileTimeAssertions();

  const char* classname_cstr() const;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_RESOURCE_DATA_H_
