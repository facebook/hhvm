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

#ifndef incl_HPHP_APC_STRING_H_
#define incl_HPHP_APC_STRING_H_

#include "hphp/runtime/base/apc-handle.h"
#include "hphp/runtime/base/apc-typed-value.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * APCString holds the data to create a PHP string from APC.
 * This object only covers KindOfString. KindOfPersistentString are handled
 * via APCTypedValue.
 */
struct APCString {

  static APCHandle::Pair MakeSharedString(StringData* str) {
    return MakeSharedString(APCKind::SharedString, str);
  }

  static APCHandle::Pair MakeSerializedArray(StringData* str) {
    return MakeSharedString(APCKind::SerializedArray, str);
  }

  static APCHandle::Pair MakeSerializedObject(const String& str) {
    return MakeSharedString(APCKind::SerializedObject, str.get());
  }

  static void Delete(APCString* s) {
    s->~APCString();
    std::free(s);
  }

  static APCString* fromHandle(APCHandle* handle) {
    assert(handle->checkInvariants());
    assert(handle->kind() == APCKind::SharedString ||
           handle->kind() == APCKind::SerializedArray ||
           handle->kind() == APCKind::SerializedObject);
    static_assert(
      offsetof(APCString, m_handle) == 0,
      "m_handle must appear first in APCString"
    );
    return reinterpret_cast<APCString*>(handle);
  }

  static const APCString* fromHandle(const APCHandle* handle) {
    assert(handle->checkInvariants());
    assert(handle->kind() == APCKind::SharedString ||
           handle->kind() == APCKind::SerializedArray ||
           handle->kind() == APCKind::SerializedObject);
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
  static APCHandle::Pair MakeSharedString(APCKind, StringData*);
  explicit APCString(APCKind kind) : m_handle(kind) {}
  ~APCString() {}
  APCString(const APCString&) = delete;
  APCString& operator=(const APCString&) = delete;

private:
  APCHandle m_handle;
  union {
    StringData m_str;
    uintptr_t dummy[sizeof(StringData) / sizeof(uintptr_t)];
  };
};

//////////////////////////////////////////////////////////////////////

}

#endif
