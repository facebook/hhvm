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

#ifndef incl_HPHP_APC_STRING_H_
#define incl_HPHP_APC_STRING_H_

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/apc-handle.h"
#include "hphp/runtime/base/apc-typed-value.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/*
 * APCString holds the data to create a PHP string from APC.
 * This object only covers KindOfString. KindOfStaticString are handled
 * via APCTypedValue.
 */
struct APCString {
  // Entry point to create an APCString
  static APCHandle::Pair MakeSharedString(DataType type, StringData* s);

  // Return the PHP string from the APC one
  static Variant MakeString(const APCHandle* handle) {
    assert(handle->type() == KindOfString);
    if (handle->isUncounted()) {
      return APCTypedValue::fromHandle(handle)->getStringData();
    }
    return Variant::attach(StringData::Make(APCString::fromHandle(handle)));
  }

  static APCString* fromHandle(APCHandle* handle) {
    static_assert(
      offsetof(APCString, m_handle) == 0,
      "m_handle must appear first in APCString"
    );
    return reinterpret_cast<APCString*>(handle);
  }

  static const APCString* fromHandle(const APCHandle* handle) {
    static_assert(
      offsetof(APCString, m_handle) == 0,
      "m_handle must appear first in APCString"
    );
    return reinterpret_cast<const APCString*>(handle);
  }

  const APCHandle* getHandle() const {
    return &m_handle;
  }

  APCHandle* getHandle() {
    return &m_handle;
  }

  StringData* getStringData() {
    return &m_str;
  }

  const StringData* getStringData() const {
    return &m_str;
  }

private:
  APCString(const APCString&) = delete;
  APCString& operator=(const APCString&) = delete;

  explicit APCString(DataType type) : m_handle(type) {}
  ~APCString() {}

  void *operator new(size_t sz, int size) {
    assert(sz == sizeof(APCString));
    return malloc(sizeof(APCString) + size);
  }
  void operator delete(void* ptr) { free(ptr); }
  // just to keep the compiler happy; used if the constructor throws
  void operator delete(void* ptr, int size) { free(ptr); }

  friend struct APCHandle;
  friend struct APCArray;
  friend struct APCObject;

private:
  APCHandle m_handle;
  union {
    StringData m_str;
    uintptr_t dummy[sizeof(StringData) / sizeof(uintptr_t)];
  };
};

///////////////////////////////////////////////////////////////////////////////
}

#endif /* incl_HPHP_APC_TYPED_VALUE_H_ */
