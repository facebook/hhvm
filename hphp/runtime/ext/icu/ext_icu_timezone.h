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

class IntlTimeZone : public IntlResourceData {
 public:
  DECLARE_RESOURCE_ALLOCATION_NO_SWEEP(IntlTimeZone);
  CLASSNAME_IS("IntlTimeZone");
  const String& o_getClassNameHook() const override { return classnameof(); }

  explicit IntlTimeZone(icu::TimeZone *tz, bool owned = true):
    m_timezone(tz), m_owned(owned) {}

  void sweep() override {
    if (m_timezone && m_owned) {
      delete m_timezone;
    }
    m_timezone = nullptr;
  }

  bool isInvalid() const override {
    return m_timezone == nullptr;
  }

  static IntlTimeZone *Get(Object obj);
  Object wrap();

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
  icu::TimeZone *m_timezone;
  bool m_owned;
};

/////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Intl

#endif // incl_HPHP_ICU_TIMEZONE_H
