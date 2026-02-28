/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/icu/icu.h"

#include <unicode/strenum.h>

namespace HPHP::Intl {
/////////////////////////////////////////////////////////////////////////////

struct IntlIterator : IntlError, SystemLib::ClassLoader<"IntlIterator"> {
  IntlIterator() {}
  IntlIterator(const IntlIterator&) = delete;
  IntlIterator& operator=(const IntlIterator& src) {
    setEnumeration(src.enumeration()->clone());
    return *this;
  }
  ~IntlIterator() {
    setEnumeration(nullptr);
  }

  void setEnumeration(icu::StringEnumeration *se) {
    if (m_enum) {
      delete m_enum;
    }
    m_enum = se;
  }

  bool isValid() const {
    return m_enum;
  }

  static Object newInstance(icu::StringEnumeration *se = nullptr) {
    Object obj{ classof() };
    if (se) {
      Native::data<IntlIterator>(obj)->setEnumeration(se);
    }
    return obj;
  }

  static IntlIterator* Get(ObjectData* obj) {
    return GetData<IntlIterator>(obj, className());
  }

  int64_t key() const { return m_key; }
  Variant current() const { return m_current; }
  bool valid() const { return m_current.isString(); }

  Variant next() {
    UErrorCode error = U_ZERO_ERROR;
    int32_t len;
    const char *e = m_enum->next(&len, error);
    if (U_FAILURE(error)) {
      s_intl_error->setError(error, "Error fetching next iteration element");
      m_current = uninit_null();
    } else {
      m_current = String(e, len, CopyString);
    }
    m_key++;
    return m_current;
  }

  bool rewind() {
    UErrorCode error = U_ZERO_ERROR;
    m_enum->reset(error);
    if (U_FAILURE(error)) {
      s_intl_error->setError(error, "Error resetting enumeration");
      m_current = uninit_null();
      return false;
    }
    m_key = -1;
    next();
    return true;
  }

  icu::StringEnumeration *enumeration() const { return m_enum; }

private:
  icu::StringEnumeration *m_enum = nullptr;
  int64_t m_key = -1;
  Variant m_current{Variant::NullInit()};
};

#if U_ICU_VERSION_MAJOR_NUM * 10 + U_ICU_VERSION_MINOR_NUM >= 42
// Proxy StringEnumeration for consistent behavior
struct BugStringCharEnumeration : icu::StringEnumeration {
  explicit BugStringCharEnumeration(UEnumeration* _uenum) : uenum(_uenum) {}
  ~BugStringCharEnumeration() { uenum_close(uenum); }

  int32_t count(UErrorCode& status) const override {
    return uenum_count(uenum, &status);
  }

  const icu::UnicodeString* snext(UErrorCode& status) override {
    int32_t length;
    const UChar* str = uenum_unext(uenum, &length, &status);
    if (str == 0 || U_FAILURE(status)) {
      return 0;
    }
    return &unistr.setTo(str, length);
  }

  const char* next(int32_t *resultLength, UErrorCode &status) override {
    int32_t length = -1;
    const char* str = uenum_next(uenum, &length, &status);
    if (str == 0 || U_FAILURE(status)) {
      return 0;
    }
    if (resultLength) {
      //the bug is that uenum_next doesn't set the length
      *resultLength = (length == -1) ? strlen(str) : length;
    }

    return str;
  }

  void reset(UErrorCode& status) override {
    uenum_reset(uenum, &status);
  }

  // Defined by UOBJECT_DEFINE_RTTI_IMPLEMENTATION
  UClassID getDynamicClassID() const override;
  static UClassID U_EXPORT2 getStaticClassID();

 private:
  UEnumeration *uenum;
};
#endif // icu >= 4.2

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP::Intl

