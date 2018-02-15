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
    assertx(data->isStatic());
    m_data.str = data;
    assertx(checkInvariants());
  }

  enum class UncountedStr {};
  APCTypedValue(UncountedStr, StringData* data)
    : m_handle(APCKind::UncountedString, KindOfPersistentString) {
    assertx(data->isUncounted());
    m_data.str = data;
    assertx(checkInvariants());
  }

  enum class StaticArr {};
  APCTypedValue(StaticArr, ArrayData* data)
    : m_handle(APCKind::StaticArray, KindOfPersistentArray) {
    assertx(data->isPHPArray());
    assertx(data->isStatic());
    m_data.arr = data;
    assertx(checkInvariants());
  }

  enum class UncountedArr {};
  APCTypedValue(UncountedArr, ArrayData* data)
    : m_handle(APCKind::UncountedArray, KindOfPersistentArray) {
    assertx(data->isPHPArray());
    assertx(data->isUncounted());
    m_data.arr = data;
    assertx(checkInvariants());
  }

  enum class StaticVec {};
  APCTypedValue(StaticVec, ArrayData* data)
    : m_handle(APCKind::StaticVec, KindOfPersistentVec) {
    assertx(data->isVecArray());
    assertx(data->isStatic());
    m_data.vec = data;
    assertx(checkInvariants());
  }

  enum class UncountedVec {};
  APCTypedValue(UncountedVec, ArrayData* data)
    : m_handle(APCKind::UncountedVec, KindOfPersistentVec) {
    assertx(data->isVecArray());
    assertx(data->isUncounted());
    m_data.vec = data;
    assertx(checkInvariants());
  }

  enum class StaticDict {};
  APCTypedValue(StaticDict, ArrayData* data)
    : m_handle(APCKind::StaticDict, KindOfPersistentDict) {
    assertx(data->isDict());
    assertx(data->isStatic());
    m_data.dict = data;
    assertx(checkInvariants());
  }

  enum class UncountedDict {};
  APCTypedValue(UncountedDict, ArrayData* data)
    : m_handle(APCKind::UncountedDict, KindOfPersistentDict) {
    assertx(data->isDict());
    assertx(data->isUncounted());
    m_data.dict = data;
    assertx(checkInvariants());
  }

  enum class StaticKeyset {};
  APCTypedValue(StaticKeyset, ArrayData* data)
    : m_handle(APCKind::StaticKeyset, KindOfPersistentKeyset) {
    assertx(data->isKeyset());
    assertx(data->isStatic());
    m_data.keyset = data;
    assertx(checkInvariants());
  }

  enum class UncountedKeyset {};
  APCTypedValue(UncountedKeyset, ArrayData* data)
    : m_handle(APCKind::UncountedKeyset, KindOfPersistentKeyset) {
    assertx(data->isKeyset());
    assertx(data->isUncounted());
    m_data.keyset = data;
    assertx(checkInvariants());
  }

  explicit APCTypedValue(DataType type)
    : m_handle(type == KindOfUninit ? APCKind::Uninit : APCKind::Null, type) {
    assertx(isNullType(type)); // Uninit or Null
    m_data.num = 0;
  }

  static APCTypedValue* fromHandle(APCHandle* handle) {
    assertx(handle->checkInvariants() && !handle->isAtomicCounted());
    static_assert(offsetof(APCTypedValue, m_handle) == sizeof(APCHandle), "");
    return reinterpret_cast<APCTypedValue*>(handle - 1);
  }

  static const APCTypedValue* fromHandle(const APCHandle* handle) {
    assertx(handle->checkInvariants() && !handle->isAtomicCounted());
    static_assert(offsetof(APCTypedValue, m_handle) == sizeof(APCHandle), "");
    return reinterpret_cast<const APCTypedValue*>(handle - 1);
  }

  APCHandle* getHandle() {
    return &m_handle;
  }

  bool getBoolean() const {
    assertx(checkInvariants() && m_handle.kind() == APCKind::Bool);
    return m_data.num != 0;
  }

  int64_t getInt64() const {
    assertx(checkInvariants() && m_handle.kind() == APCKind::Int);
    return m_data.num;
  }

  double getDouble() const {
    assertx(checkInvariants() && m_handle.kind() == APCKind::Double);
    return m_data.dbl;
  }

  StringData* getStringData() const {
    assertx(checkInvariants());
    assertx(m_handle.kind() == APCKind::StaticString ||
           m_handle.kind() == APCKind::UncountedString);
    return m_data.str;
  }

  ArrayData* getArrayData() const {
    assertx(checkInvariants());
    assertx(m_handle.kind() == APCKind::StaticArray ||
           m_handle.kind() == APCKind::UncountedArray);
    return m_data.arr;
  }

  ArrayData* getVecData() const {
    assertx(checkInvariants());
    assertx(m_handle.kind() == APCKind::StaticVec ||
           m_handle.kind() == APCKind::UncountedVec);
    return m_data.vec;
  }

  ArrayData* getDictData() const {
    assertx(checkInvariants());
    assertx(m_handle.kind() == APCKind::StaticDict ||
           m_handle.kind() == APCKind::UncountedDict);
    return m_data.dict;
  }

  ArrayData* getKeysetData() const {
    assertx(checkInvariants());
    assertx(m_handle.kind() == APCKind::StaticKeyset ||
           m_handle.kind() == APCKind::UncountedKeyset);
    return m_data.keyset;
  }

  static APCTypedValue* tvUninit();
  static APCTypedValue* tvNull();
  static APCTypedValue* tvTrue();
  static APCTypedValue* tvFalse();

  void deleteUncounted();
  // Recursively register all {allocation, root} with APCGCManager
  void registerUncountedAllocations();

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
    ArrayData* vec;
    ArrayData* dict;
    ArrayData* keyset;
  } m_data;
  APCHandle m_handle;
};

//////////////////////////////////////////////////////////////////////

}

#endif
