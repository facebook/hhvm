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

#ifndef incl_HPHP_APC_TYPED_VALUE_H_
#define incl_HPHP_APC_TYPED_VALUE_H_

#include "hphp/runtime/base/apc-handle.h"
#include "hphp/runtime/base/apc-handle-defs.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct APCTypedValue {
  // Int or bool
  enum class Bool {};
  APCTypedValue(Bool, bool data)
    : m_handle(APCKind::Bool, KindOfBoolean) {
    m_data.num = data;
  }

  explicit APCTypedValue(int64_t data)
    : m_handle(APCKind::Int, KindOfInt64) {
    m_data.num = data;
  }

  explicit APCTypedValue(double data)
    : m_handle(APCKind::Double, KindOfDouble) {
    m_data.dbl = data;
  }

  enum class StaticStr {};
  APCTypedValue(StaticStr, StringData* data)
    : m_handle(APCKind::StaticString, KindOfPersistentString) {
    assert(data->isStatic());
    m_data.str = data;
    assert(checkInvariants());
  }

  enum class UncountedStr {};
  APCTypedValue(UncountedStr, StringData* data)
    : m_handle(APCKind::UncountedString, KindOfPersistentString) {
    assert(data->isUncounted());
    m_data.str = data;
    assert(checkInvariants());
  }

  enum class StaticArr {};
  APCTypedValue(StaticArr, ArrayData* data)
    : m_handle(APCKind::StaticArray, KindOfPersistentArray) {
    assert(data->isStatic());
    m_data.arr = data;
    assert(checkInvariants());
  }

  enum class UncountedArr {};
  APCTypedValue(UncountedArr, ArrayData* data)
    : m_handle(APCKind::UncountedArray, KindOfPersistentArray) {
    assert(data->isUncounted());
    m_data.arr = data;
    assert(checkInvariants());
  }

  explicit APCTypedValue(DataType type)
    : m_handle(type == KindOfUninit ? APCKind::Uninit : APCKind::Null, type) {
    assert(isNullType(type)); // Uninit or Null
    m_data.num = 0;
  }

  static APCTypedValue* fromHandle(APCHandle* handle) {
    assert(handle->checkInvariants() && !handle->isAtomicCounted());
    static_assert(offsetof(APCTypedValue, m_handle) == sizeof(APCHandle), "");
    return reinterpret_cast<APCTypedValue*>(handle - 1);
  }

  static const APCTypedValue* fromHandle(const APCHandle* handle) {
    assert(handle->checkInvariants() && !handle->isAtomicCounted());
    static_assert(offsetof(APCTypedValue, m_handle) == sizeof(APCHandle), "");
    return reinterpret_cast<const APCTypedValue*>(handle - 1);
  }

  APCHandle* getHandle() {
    return &m_handle;
  }

  bool getBoolean() const {
    assert(checkInvariants() && m_handle.kind() == APCKind::Bool);
    return m_data.num != 0;
  }

  int64_t getInt64() const {
    assert(checkInvariants() && m_handle.kind() == APCKind::Int);
    return m_data.num;
  }

  double getDouble() const {
    assert(checkInvariants() && m_handle.kind() == APCKind::Double);
    return m_data.dbl;
  }

  StringData* getStringData() const {
    assert(checkInvariants());
    assert(m_handle.kind() == APCKind::StaticString ||
           m_handle.kind() == APCKind::UncountedString);
    return m_data.str;
  }

  ArrayData* getArrayData() const {
    assert(checkInvariants());
    assert(m_handle.kind() == APCKind::StaticArray ||
           m_handle.kind() == APCKind::UncountedArray);
    return m_data.arr;
  }

  static APCTypedValue* tvUninit();
  static APCTypedValue* tvNull();
  static APCTypedValue* tvTrue();
  static APCTypedValue* tvFalse();

  void deleteUncounted();

private:
  APCTypedValue(const APCTypedValue&) = delete;
  APCTypedValue& operator=(const APCTypedValue&) = delete;
  bool checkInvariants() const;

private:
  union {
    int64_t num;
    double dbl;
    StringData* str;
    ArrayData* arr;
  } m_data;
  APCHandle m_handle;
};

//////////////////////////////////////////////////////////////////////

}

#endif
