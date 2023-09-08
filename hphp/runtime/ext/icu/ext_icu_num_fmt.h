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

#include <unicode/unum.h>

namespace HPHP::Intl {
/////////////////////////////////////////////////////////////////////////////
extern const StaticString s_NumberFormatter;

struct NumberFormatter : IntlError, SystemLib::ClassLoader<"NumberFormatter"> {
  NumberFormatter() {}
  NumberFormatter(const NumberFormatter&) = delete;
  NumberFormatter& operator=(const NumberFormatter& src) {
    setNumberFormatter(&src);
    return *this;
  }
  ~NumberFormatter() {
    if (m_formatter) {
      unum_close(m_formatter);
      m_formatter = nullptr;
    }
  }

  void setNumberFormatter(const String& locale,
                          int64_t style,
                          const String& pattern);
  void setNumberFormatter(const NumberFormatter *orig);

  bool isValid() const {
    return m_formatter;
  }

  static Object newInstance() {
    return Object{ classof() };
  }
  static NumberFormatter* Get(ObjectData* obj) {
    return GetData<NumberFormatter>(obj, className());
  }

  UNumberFormat *formatter() const { return m_formatter; }

private:
  UNumberFormat *m_formatter = nullptr;
};

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP::Intl

