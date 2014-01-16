/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_APC_TYPED_VALUE_H_
#define incl_HPHP_APC_TYPED_VALUE_H_

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/apc-handle.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/*
 * APCTypedValue is the APC object that holds non refcounted data. That is,
 * primitive values and static strings.
 * APCTypedValue has the same layout as TypedValue so it can be returned in
 * some cases and be used directly without any other allocation (see
 * APCLocalArray::getValueRef()). See comment below for more details.
 */
class APCTypedValue {
public:
  APCTypedValue(DataType type, int64_t data) : m_handle(type) {
    m_data.num = data;
  }

  APCTypedValue(DataType type, double data) : m_handle(type) {
    m_data.dbl = data;
  }

  APCTypedValue(DataType type, StringData* data) : m_handle(type) {
    m_data.str = data;
  }

  explicit APCTypedValue(DataType type) : m_handle(type) {
    m_data.num = 0;
  }

  static APCTypedValue* fromHandle(APCHandle* handle) {
#if PACKED_TV
    assert(offsetof(APCTypedValue, m_handle) == 0);
    return reinterpret_cast<APCTypedValue*>(handle);
#else
    assert(offsetof(APCTypedValue, m_handle) == sizeof(SharedData));
    return reinterpret_cast<APCTypedValue*>(handle - 1);
#endif
  }

  APCHandle* getHandle() {
    return &m_handle;
  }

  bool getBoolean() const {
    assert(m_handle.is(KindOfBoolean));
    return m_data.num != 0;
  }

  int64_t getInt64() const {
    assert(m_handle.is(KindOfInt64));
    return m_data.num;
  }

  double getDouble() const {
    assert(m_handle.is(KindOfDouble));
    return m_data.dbl;
  }

  StringData *getStringData() const {
    assert(m_handle.is(KindOfStaticString));
    return m_data.str;
  }

  CVarRef asCVarRef() const {
    // Must be non-refcounted types
    assert(m_handle.m_shouldCache == false);
    assert(m_handle.m_flags == 0);
    assert(!IS_REFCOUNTED_TYPE(m_handle.m_type));
    return tvAsCVarRef(reinterpret_cast<const TypedValue*>(this));
  }

private:
  APCTypedValue(const APCTypedValue&) = delete;
  APCTypedValue& operator=(const APCTypedValue&) = delete;

private:

  /*
   * Keep the object layout binary compatible with Variant for primitive types.
   * For non-refcounted types, m_shouldCache and m_flags are guaranteed to be 0,
   * and other parts of runtime will not touch the count.
   *
   * Note that this is assuming a little-endian system: m_shouldCache and
   * m_flags have to overlay the higher-order bits of TypedValue::m_type.
   */

  union SharedData {
    int64_t num;
    double dbl;
    StringData *str;
  };

#if PACKED_TV
  APCHandle m_handle;
  SharedData m_data;
#else
  SharedData m_data;
  APCHandle m_handle;
#endif

  static void compileTimeAssertions() {
    static_assert(
        offsetof(APCTypedValue, m_data) == offsetof(TypedValue, m_data),
        "Offset of m_data must be equal in APCHandle and TypedValue");
    static_assert(
        offsetof(APCTypedValue, m_handle) + offsetof(APCHandle, m_count) ==
            TypedValueAux::auxOffset,
        "Offset of m_count must equal offset of TV.m_aux");
    static_assert(
        offsetof(APCTypedValue, m_handle) + offsetof(APCHandle, m_type) ==
            offsetof(TypedValue, m_type),
        "Offset of m_type must be equal in APCHandle and TypedValue");
    static_assert(
        sizeof(APCTypedValue) == sizeof(TypedValue),
        "Be careful with field layout");
  }
};

///////////////////////////////////////////////////////////////////////////////
}

#endif /* incl_HPHP_APC_TYPED_VALUE_H_ */
