/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct APCTypedValue {
  // Int or bool
  enum class Bool {};
  APCTypedValue(Bool, bool data) : m_handle(KindOfBoolean) {
    m_data.num = data;
  }

  explicit APCTypedValue(int64_t data) : m_handle(KindOfInt64) {
    m_data.num = data;
  }

  explicit APCTypedValue(double data) : m_handle(KindOfDouble) {
    m_data.dbl = data;
  }

  enum class StaticStr {};
  APCTypedValue(StaticStr, StringData* data) : m_handle(KindOfStaticString) {
    assert(data->isStatic());
    m_data.str = data;
  }

  enum class UncountedStr {};
  APCTypedValue(UncountedStr, StringData* data) : m_handle(KindOfString) {
    assert(data->isUncounted());
    m_handle.setUncounted();
    m_data.str = data;
  }

  explicit APCTypedValue(ArrayData* data) : m_handle(KindOfArray) {
    assert(data->isUncounted());
    m_handle.setUncounted();
    m_data.arr = data;
  }

  explicit APCTypedValue(DataType type) : m_handle(type) {
    assert(isNullType(type)); // Uninit or Null
    m_data.num = 0;
  }

  static APCTypedValue* fromHandle(APCHandle* handle) {
    return reinterpret_cast<APCTypedValue*>(handle - 1);
  }

  static const APCTypedValue* fromHandle(const APCHandle* handle) {
    return reinterpret_cast<const APCTypedValue*>(handle - 1);
  }

  APCHandle* getHandle() {
    return &m_handle;
  }

  bool getBoolean() const {
    assert(m_handle.type() == KindOfBoolean);
    return m_data.num != 0;
  }

  int64_t getInt64() const {
    assert(m_handle.type() == KindOfInt64);
    return m_data.num;
  }

  double getDouble() const {
    assert(m_handle.type() == KindOfDouble);
    return m_data.dbl;
  }

  StringData* getStringData() const {
    assert(m_handle.type() == KindOfStaticString ||
           (m_handle.isUncounted() && m_handle.type() == KindOfString));
    return m_data.str;
  }

  ArrayData* getArrayData() const {
    assert(m_handle.isUncounted() && m_handle.type() == KindOfArray);
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
