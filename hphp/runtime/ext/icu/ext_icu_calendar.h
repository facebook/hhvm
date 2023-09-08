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

#include <unicode/calendar.h>
#include <unicode/gregocal.h>

namespace HPHP::Intl {
/////////////////////////////////////////////////////////////////////////////

struct IntlCalendar : IntlError, SystemLib::ClassLoader<"IntlCalendar"> {
  IntlCalendar() {}
  IntlCalendar(const IntlCalendar&) = delete;
  IntlCalendar& operator=(const IntlCalendar& src) {
    setCalendar(src.calendar()->clone());
    return *this;
  }
  ~IntlCalendar() {
    setCalendar(nullptr);
  }

  void setCalendar(icu::Calendar *cal) {
    if (m_cal) {
      delete m_cal;
    }
    m_cal = cal;
  }

  static Object newInstance(icu::Calendar *cal) {
    Object ret{ classof() };
    if (cal) {
      Native::data<IntlCalendar>(ret)->setCalendar(cal);
    }
    return ret;
  }

  static IntlCalendar* Get(ObjectData* obj) {
    return GetData<IntlCalendar>(obj, className());
  }

  bool isValid() const {
    return m_cal;
  }

  icu::Calendar *calendar() const { return m_cal; }
  icu::GregorianCalendar *gcal() const {
    return dynamic_cast<icu::GregorianCalendar*>(m_cal);
  }

  static const icu::Calendar* ParseArg(const Variant& cal, const icu::Locale &locale,
                                       const String &funcname, IntlError *err,
                                       int64_t &calType, bool &calOwned);
 protected:
  icu::Calendar *m_cal = nullptr;
};

/////////////////////////////////////////////////////////////////////////////
} // namespace HPHP::Intl

