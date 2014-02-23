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
#ifndef incl_HPHP_INTL_CALENDAR_H
#define incl_HPHP_INTL_CALENDAR_H

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/ext/icu/icu.h"

#include <unicode/calendar.h>
#include <unicode/gregocal.h>

namespace HPHP { namespace Intl {
/////////////////////////////////////////////////////////////////////////////
extern const StaticString s_IntlCalendar;

class IntlCalendar : public IntlResourceData {
 public:
  IntlCalendar() {}
  IntlCalendar(const IntlCalendar&) = delete;
  IntlCalendar& operator=(const IntlCalendar& src) {
    setCalendar(src.calendar()->clone());
    return *this;
  }
  ~IntlCalendar() override {
    setCalendar(nullptr);
  }

  void setCalendar(icu::Calendar *cal) {
    if (m_cal) {
      delete m_cal;
    }
    m_cal = cal;
  }

  static Object newInstance(icu::Calendar *cal) {
    auto ret = NewInstance(s_IntlCalendar);
    if (cal) {
      Native::data<IntlCalendar>(ret.get())->setCalendar(cal);
    }
    return ret;
  }

  static IntlCalendar* Get(Object obj) {
    return GetData<IntlCalendar>(obj, s_IntlCalendar);
  }

  bool isValid() const override {
    return m_cal;
  }

  icu::Calendar *calendar() const { return m_cal; }
  icu::GregorianCalendar *gcal() const {
    return dynamic_cast<icu::GregorianCalendar*>(m_cal);
  }

  static const icu::Calendar* ParseArg(CVarRef cal, const icu::Locale &locale,
                                       const String &funcname, intl_error &err,
                                       int64_t &calType, bool &calOwned);
 protected:
  icu::Calendar *m_cal = nullptr;
};

/////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Intl

#endif // incl_HPHP_INTL_CALENDAR_H
