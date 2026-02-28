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

#include <unicode/udat.h>
#include <unicode/datefmt.h>

namespace HPHP::Intl {
/////////////////////////////////////////////////////////////////////////////

struct IntlDateFormatter : IntlError,
                           SystemLib::ClassLoader<"IntlDateFormatter"> {
  IntlDateFormatter() {}
  IntlDateFormatter(const IntlDateFormatter&) = delete;
  IntlDateFormatter& operator=(const IntlDateFormatter& src) {
    setDateFormatter(&src);
    return *this;
  }
  ~IntlDateFormatter() {
    if (m_date_fmt) {
      udat_close((UDateFormat*)m_date_fmt);
      m_date_fmt = nullptr;
    }
  }

  void setDateFormatter(const String& locale,
                        int64_t datetype, int64_t timetype,
                        const Variant& timezone, const Variant& calendar,
                        const String& pattern);
  void setDateFormatter(const IntlDateFormatter *orig);

  bool isValid() const {
    return m_date_fmt;
  }

  static Object newInstance() {
    return Object{ classof() };
  }
  static IntlDateFormatter* Get(ObjectData* obj) {
    return GetData<IntlDateFormatter>(obj, className());
  }

  // Zend seems to think casting UDateFormat* to icu::DateFormat*
  // is a good idea.  Sounds dodgy as heck to me though...
  icu::DateFormat *datefmtObject() const {return (icu::DateFormat*)m_date_fmt;}
  UDateFormat *datefmt() const { return m_date_fmt; }
  int64_t dateType() const { return m_date_type; }
  int64_t timeType() const { return m_time_type; }
  int64_t calendar() const { return m_calendar; }

  int64_t getArrayElemInt(const Array& arr, const StaticString &key);
  double getTimestamp(const Variant& arg);

 private:
  UDateFormat *m_date_fmt = nullptr;

  int64_t m_date_type;
  int64_t m_time_type;
  int64_t m_calendar;
};

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP::Intl
