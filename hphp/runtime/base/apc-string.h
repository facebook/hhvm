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

#ifndef incl_HPHP_APC_STRING_H_
#define incl_HPHP_APC_STRING_H_

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/apc-handle.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/*
 * APCString holds the data to create a PHP string from APC.
 * This object only covers KindOfString. KindOfStaticString are handled
 * via APCTypedValue.
 */
struct APCString {
  // Entry point to create an APCString
  static APCHandle* MakeShared(DataType type, StringData* s);

  // Return the PHP string from the APC one
  static Variant MakeString(APCHandle* handle) {
    assert(handle->getType() == KindOfString);
    return StringData::Make(APCString::fromHandle(handle));
  }

  static APCString* fromHandle(APCHandle* handle) {
    assert(offsetof(APCString, m_handle) == 0);
    return reinterpret_cast<APCString*>(handle);
  }

  APCHandle* getHandle() {
    return &m_handle;
  }

  StringData* getStringData() {
    return &m_data;
  }

  //
  // APCHandle forward API
  //

  void incRef() {
    m_handle.incRef();
  }

  void decRef() {
    m_handle.decRef();
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
    StringData m_data;
    uintptr_t dummy[sizeof(StringData) / sizeof(uintptr_t)];
  };
};

///////////////////////////////////////////////////////////////////////////////
}

#endif /* incl_HPHP_APC_TYPED_VALUE_H_ */
