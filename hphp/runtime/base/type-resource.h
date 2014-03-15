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
class Resource : protected SmartPtr<ResourceData> {
  typedef SmartPtr<ResourceData> ResourceBase;

public:
  Resource() {}

  static const Resource s_nullResource;

  ResourceData* get() const { return m_px; }
  void reset() { ResourceBase::reset(); }

  ResourceData* operator->() const {
    if (!m_px) throw_null_pointer_exception();
    return m_px;
  }

  /**
   * Constructors
   */
  /* implicit */ Resource(ResourceData *data) : ResourceBase(data) { }
  /* implicit */ Resource(const Resource& src) : ResourceBase(src.m_px) { }

  // Move ctor
  Resource(Resource&& src) : ResourceBase(std::move(src)) { }

  // Regular assign
  Resource& operator=(const Resource& src) {
    ResourceBase::operator=(src);
    return *this;
  }
  // Move assign
  Resource& operator=(Resource&& src) {
    ResourceBase::operator=(std::move(src));
    return *this;
  }

  ~Resource();

  /**
   * Informational
   */
  bool isNull() const {
    return m_px == nullptr;
  }

  bool isInvalid() const {
    return m_px == nullptr || m_px->isInvalid();
  }

  template <class T> T& cast() { return *static_cast<T*>(this); }
  template <class T> const T& cast() const {
    return *static_cast<const T*>(this);
  }

  /**
   * getTyped() and is() are intended for use with C++ classes that derive
   * from ResourceData.
   */
  template<typename T>
  T* getTyped(bool nullOkay = false, bool badTypeOkay = false) const {
    static_assert(std::is_base_of<ResourceData, T>::value, "");

    ResourceData *cur = m_px;
    if (!cur) {
      if (!nullOkay) {
        throw_null_pointer_exception();
      }
      return nullptr;
    }
    T *px = dynamic_cast<T*>(cur);
    if (!px) {
      if (!badTypeOkay) {
        throw InvalidObjectTypeException(classname_cstr());
      }
      return nullptr;
    }

    // Assert that casting does not adjust the 'this' pointer
    assert((void*)px == (void*)cur);
    return px;
  }

  template<typename T>
  bool is() const {
    return getTyped<T>(true, true) != nullptr;
  }

  template<typename T>
  T *cast() const {
    return getTyped<T>();
  }

  /**
   * Type conversions
   */
  bool   toBoolean() const { return m_px ? m_px->o_toBoolean() : false;}
  char   toByte   () const { return m_px ? m_px->o_toInt64() : 0;}
  short  toInt16  () const { return m_px ? m_px->o_toInt64() : 0;}
  int    toInt32  () const { return m_px ? m_px->o_toInt64() : 0;}
  int64_t toInt64 () const { return m_px ? m_px->o_toInt64() : 0;}
  double toDouble () const { return m_px ? m_px->o_toDouble() : 0;}
  String toString () const;
  Array  toArray  () const;

  /**
   * Comparisons
   */
  bool same (const Resource& v2) const { return m_px == v2.get();}
  bool equal(const Resource& v2) const { return m_px == v2.get();}
  bool less (const Resource& v2) const { return toInt64() < v2.toInt64();}
  bool more (const Resource& v2) const { return toInt64() > v2.toInt64();}

  // Transfer ownership of our reference to this resource.
  ResourceData *detach() {
    ResourceData *ret = m_px;
    m_px = nullptr;
    return ret;
  }
private:
  static void compileTimeAssertions();

  const char* classname_cstr() const;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_RESOURCE_DATA_H_
