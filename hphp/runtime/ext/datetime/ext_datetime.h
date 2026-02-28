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
#include "hphp/runtime/base/timestamp.h"
#include "hphp/runtime/base/datetime.h"
#include "hphp/runtime/base/timezone.h"
#include "hphp/runtime/base/dateinterval.h"
#include "hphp/runtime/ext/std/ext_std_misc.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// class DateTime

struct DateTimeData : SystemLib::ClassLoader<"DateTime"> {
  DateTimeData() {}
  DateTimeData(const DateTimeData&) = delete;
  DateTimeData& operator=(const DateTimeData& other) {
    m_dt = other.m_dt ? other.m_dt->cloneDateTime() : req::ptr<DateTime>{};
    return *this;
  }
  Variant sleep() const {
    return init_null();
  }
  void wakeup(const Variant& /*content*/, ObjectData* /*obj*/) {}
  int64_t getTimestamp() const {
    assertx(m_dt);
    bool err = false;
    return m_dt->toTimeStamp(err);
  }
  String format(const String& format) const {
    assertx(m_dt);
    return m_dt->toString(format, false);
  }
  Array getDebugInfo() const;

  static int64_t getTimestamp(const Object& obj);
  static int64_t getTimestamp(const ObjectData* od);
  static int compare(const Object& left, const Object& right);
  static int compare(const ObjectData* left, const ObjectData* right);
  static Object wrap(req::ptr<DateTime> dt);
  static req::ptr<DateTime> unwrap(const Object& datetime);

  req::ptr<DateTime> m_dt;
};

void HHVM_METHOD(DateTime, __construct,
                 const String& time = "now",
                 const Variant& timezone = uninit_variant);

///////////////////////////////////////////////////////////////////////////////
// class DateTimeZone

struct DateTimeZoneData : SystemLib::ClassLoader<"DateTimeZone"> {
  DateTimeZoneData() {}
  DateTimeZoneData(const DateTimeZoneData&) = delete;
  DateTimeZoneData& operator=(const DateTimeZoneData& other) {
    m_tz = other.m_tz ? other.m_tz->cloneTimeZone() : req::ptr<TimeZone>{};
    return *this;
  }

  String getName() const {
    assertx(m_tz);
    return m_tz->name();
  }

  Array getDebugInfo() const;

  static Object wrap(req::ptr<TimeZone> tz);
  static req::ptr<TimeZone> unwrap(const Object& timezone);

  req::ptr<TimeZone> m_tz;

  static const int64_t AFRICA = 1;
  static const int64_t AMERICA = 2;
  static const int64_t ANTARCTICA = 4;
  static const int64_t ARCTIC = 8;
  static const int64_t ASIA = 16;
  static const int64_t ATLANTIC = 32;
  static const int64_t AUSTRALIA = 64;
  static const int64_t EUROPE = 128;
  static const int64_t INDIAN = 256;
  static const int64_t PACIFIC = 512;
  static const int64_t UTC = 1024;
  static const int64_t ALL = 2047;
  static const int64_t ALL_WITH_BC = 4095;
  static const int64_t PER_COUNTRY = 4096;
};

void HHVM_METHOD(DateTimeZone, __construct,
                 const String& timezone);

///////////////////////////////////////////////////////////////////////////////
// class DateInterval

struct DateIntervalData : SystemLib::ClassLoader<"DateInterval"> {
  DateIntervalData() {}
  DateIntervalData(const DateIntervalData&) = delete;
  DateIntervalData& operator=(const DateIntervalData& other) {
    m_di =
      other.m_di ? other.m_di->cloneDateInterval() : req::ptr<DateInterval>{};
    return *this;
  }

  static Object wrap(req::ptr<DateInterval> di);
  static req::ptr<DateInterval> unwrap(const Object& di);

  req::ptr<DateInterval> m_di;
};

///////////////////////////////////////////////////////////////////////////////
// timezone

String HHVM_FUNCTION(date_default_timezone_get);

///////////////////////////////////////////////////////////////////////////////
}
