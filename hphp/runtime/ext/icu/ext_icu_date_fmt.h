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
#ifndef incl_HPHP_ICU_DATE_FMT_H
#define incl_HPHP_ICU_DATE_FMT_H

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/ext/icu/icu.h"

#include <unicode/udat.h>
#include <unicode/datefmt.h>

namespace HPHP { namespace Intl {
/////////////////////////////////////////////////////////////////////////////

class IntlDateFormatter : public IntlResourceData {
 public:
  DECLARE_RESOURCE_ALLOCATION_NO_SWEEP(IntlDateFormatter);
  CLASSNAME_IS("IntlDateFormatter");
  const String& o_getClassNameHook() const override { return classnameof(); }

  IntlDateFormatter(const String& locale, int64_t datetype, int64_t timetype,
                    CVarRef timezone, CVarRef calendar, const String& pattern);
  explicit IntlDateFormatter(const IntlDateFormatter *orig);

  void sweep() override {
    if (m_date_fmt) {
      udat_close((UDateFormat*)m_date_fmt);
      m_date_fmt = nullptr;
    }
  }

  bool isInvalid() const override {
    return m_date_fmt == nullptr;
  }

  static IntlDateFormatter *Get(Object obj);
  Object wrap();

  // Zend seems to think casting UDateFormat* to icu::DateFormat*
  // is a good idea.  Sounds dodgy as heck to me though...
  icu::DateFormat *datefmtObject() const {return (icu::DateFormat*)m_date_fmt;}
  UDateFormat *datefmt() const { return m_date_fmt; }
  int64_t dateType() const { return m_date_type; }
  int64_t timeType() const { return m_time_type; }
  int64_t calendar() const { return m_calendar; }

  int64_t getArrayElemInt(CArrRef arr, const StaticString &key);
  double getTimestamp(CVarRef arg);

 private:
  UDateFormat *m_date_fmt = nullptr;

  int64_t m_date_type;
  int64_t m_time_type;
  int64_t m_calendar;
};

/////////////////////////////////////////////////////////////////////////////
}} // namespace HPHP::Intl
#endif // incl_HPHP_ICU_DATE_FMT_H
