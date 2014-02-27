/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_ICU_TIMEZONE_H
#define incl_HPHP_ICU_TIMEZONE_H

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/ext/icu/icu.h"

#include <unicode/timezone.h>

namespace HPHP { namespace Intl {
/////////////////////////////////////////////////////////////////////////////
extern const StaticString s_IntlTimeZone;

class IntlTimeZone : public IntlResourceData {
 public:
  IntlTimeZone() {}
  IntlTimeZone(const IntlTimeZone&) = delete;
  IntlTimeZone& operator=(const IntlTimeZone& src) {
    setTimeZone(src.timezone()->clone());
    return *this;
  }
  ~IntlTimeZone() override {
    setTimeZone(nullptr);
  }

  static Object newInstance(icu::TimeZone *tz = nullptr, bool owned = true) {
    auto obj = NewInstance(s_IntlTimeZone);
    if (tz) {
      Native::data<IntlTimeZone>(obj.get())->setTimeZone(tz, owned);
    }
    return obj;
  }

  static IntlTimeZone* Get(Object obj) {
    return GetData<IntlTimeZone>(obj, s_IntlTimeZone);
  }

  void setTimeZone(icu::TimeZone *tz, bool owned = true) {
    if (m_timezone && m_owned) {
      delete m_timezone;
    }
    m_timezone = tz;
    m_owned = owned;
  }

  bool isValid() const override {
    return m_timezone;
  }

  static bool isValidStyle(int64_t style) {
    return (style == icu::TimeZone::SHORT) ||
           (style == icu::TimeZone::LONG) ||
#if U_ICU_VERSION_MAJOR_NUM * 10 + U_ICU_VERSION_MINOR_NUM >= 44
           (style == icu::TimeZone::SHORT_GENERIC) ||
           (style == icu::TimeZone::LONG_GENERIC) ||
           (style == icu::TimeZone::SHORT_GMT) ||
           (style == icu::TimeZone::LONG_GMT) ||
           (style == icu::TimeZone::SHORT_COMMONLY_USED) ||
           (style == icu::TimeZone::GENERIC_LOCATION) ||
#endif
           false;
  }

  icu::TimeZone* timezone() const { return m_timezone; }

  static icu::TimeZone* ParseArg(CVarRef arg, const String& funcname,
                                 intl_error &err);
 private:
  icu::TimeZone *m_timezone = nullptr;
  bool m_owned = false;
};

/////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Intl

#endif // incl_HPHP_ICU_TIMEZONE_H
