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

#pragma once

#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/base/resource-data.h"
#include "hphp/runtime/base/req-ptr.h"
#include "hphp/runtime/base/sweepable.h"

#include <algorithm>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Resource type wrapping around ResourceData to implement reference count.
 * May be null; corresponds to the hack type `?resource`.
 */
struct OptResource {
private:
  using Ptr = req::ptr<ResourceHdr>;
  using NoIncRef = Ptr::NoIncRef;

  Ptr m_res;

public:
  OptResource() {}

  void reset(ResourceData* res = nullptr) {
    m_res.reset(safehdr(res));
  }
  void reset(ResourceHdr* res = nullptr) {
    m_res.reset(res);
  }

  ResourceData* operator->() const {
    return safedata(m_res.get());
  }

  ResourceHdr* hdr() const {
    return m_res.get();
  }

  ResourceHdr* detachHdr() {
    return m_res.detach();
  }

  /**
   * Constructors
   */
  explicit OptResource(ResourceData *data)
    : m_res(safehdr(data)) {}
  explicit OptResource(ResourceHdr *hdr)
    : m_res(hdr) {}

  /* implicit */ OptResource(const OptResource& src)
    : m_res(src.m_res) { }

  template <typename T> // T must extend ResourceData
  explicit OptResource(req::ptr<T>&& src)
  : m_res(safehdr(src.detach()), NoIncRef{})
  {}

  template <typename T> // T must extend resourceData
  explicit OptResource(const req::ptr<T>& src)
  : m_res(safehdr(src.get())) // causes incref
  {}

  // Move ctor
  OptResource(OptResource&& src) noexcept : m_res(std::move(src.m_res)) { }

  // Regular assign
  OptResource& operator=(const OptResource& src) {
    m_res = src.m_res;
    return *this;
  }
  template <typename T>
  OptResource& operator=(const req::ptr<T>& src) {
    m_res = src;
    return *this;
  }
  // Move assign
  OptResource& operator=(OptResource&& src) {
    m_res = std::move(src.m_res);
    return *this;
  }
  template <typename T>
  OptResource& operator=(req::ptr<T>&& src) {
    m_res = std::move(src);
    return *this;
  }

  ~OptResource();

  /**
   * Informational
   */
  explicit operator bool() const { return (bool)m_res; }

  bool isNull() const {
    return !m_res;
  }

  bool isInvalid() const {
    return !m_res || m_res->data()->isInvalid();
  }

  /**
   * Type conversions
   */
  int64_t toInt64() const {
    return m_res ? m_res->data()->o_toInt64() : 0;
  }
  double toDouble() const {
    return m_res ? m_res->data()->o_toDouble() : 0;
  }
  String toString() const;
  Array toArray() const;

private:
  //
  // The deref and detach functions are only for use by the heap tracer,
  // cast functions and Variant.  They should not be used anywhere else.
  //
  template <typename T>
  friend typename std::enable_if<
    std::is_base_of<ResourceData,T>::value,
    ResourceData*
  >::type deref(const OptResource& r) {
    return safedata(r.m_res.get());
  }

  template <typename T>
  friend typename std::enable_if<
    std::is_base_of<ResourceData,T>::value,
    ResourceData*
  >::type detach(OptResource&& r) {
    return safedata(r.m_res.detach());
  }

  static void compileTimeAssertions();
};

extern const OptResource null_resource;

///////////////////////////////////////////////////////////////////////////////
}

