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

#ifndef incl_HPHP_EXT_DATETIME_H_
#define incl_HPHP_EXT_DATETIME_H_

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/timestamp.h"
#include "hphp/runtime/base/datetime.h"
#include "hphp/runtime/base/timezone.h"
#include "hphp/runtime/base/dateinterval.h"
#include "hphp/system/constants.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// class DateTime

extern const StaticString q_DateTime$$ATOM;
extern const StaticString q_DateTime$$COOKIE;
extern const StaticString q_DateTime$$ISO8601;
extern const StaticString q_DateTime$$RFC822;
extern const StaticString q_DateTime$$RFC850;
extern const StaticString q_DateTime$$RFC1036;
extern const StaticString q_DateTime$$RFC1123;
extern const StaticString q_DateTime$$RFC2822;
extern const StaticString q_DateTime$$RFC3339;
extern const StaticString q_DateTime$$RSS;
extern const StaticString q_DateTime$$W3C;

class DateTimeData {
public:
  DateTimeData() {}
  DateTimeData(const DateTimeData&) = delete;
  DateTimeData& operator=(const DateTimeData& other) {
    m_dt = other.m_dt->cloneDateTime();
    return *this;
  }
  Variant sleep() const {
    return init_null();
  }
  void wakeup(const Variant& content, ObjectData* obj) {}
  int64_t getTimestamp() const {
    bool err = false;
    return m_dt->toTimeStamp(err);
  }
  String format(const String& format) const {
    return m_dt->toString(format, false);
  }
  Array getDebugInfo() const;

  static int64_t getTimestamp(const Object& obj);
  static int64_t getTimestamp(const ObjectData* od);
  static Object wrap(req::ptr<DateTime> dt);
  static req::ptr<DateTime> unwrap(const Object& datetime);
  static Class* getClass();

  req::ptr<DateTime> m_dt;
  static Class* s_class;
  static const StaticString s_className;
};

Object HHVM_METHOD(DateTime, add,
                   const Object& interval);
void HHVM_METHOD(DateTime, __construct,
                 const String& time = "now",
                 const Variant& timezone = null_variant);
Variant HHVM_STATIC_METHOD(DateTime, createFromFormat,
                           const String& format,
                           const String& time,
                           const Variant& timezone /*= null_variant */);
Object HHVM_METHOD(DateTime, diff,
                   const Variant& datetime2,
                   const Variant& absolute);
String HHVM_METHOD(DateTime, format,
                   const Variant& format);
Array HHVM_STATIC_METHOD(DateTime, getLastErrors);
int64_t HHVM_METHOD(DateTime, getOffset);
int64_t HHVM_METHOD(DateTime, gettimestamp);
Variant HHVM_METHOD(DateTime, getTimezone);
Object HHVM_METHOD(DateTime, modify,
                   const String& modify);
Object HHVM_METHOD(DateTime, setDate,
                   int64_t year,
                   int64_t month,
                   int64_t day);
Object HHVM_METHOD(DateTime, setISODate,
                   int64_t year,
                   int64_t week,
                   int64_t day /*= 1*/);
Object HHVM_METHOD(DateTime, setTime,
                   int64_t hour,
                   int64_t minute,
                   int64_t second /*= 0*/);
Object HHVM_METHOD(DateTime, setTimestamp,
                   int64_t unixtimestamp);
Object HHVM_METHOD(DateTime, setTimezone,
                   const Object& timezone);
Object HHVM_METHOD(DateTime, sub,
                   const Object& interval);
Array HHVM_METHOD(DateTime, __sleep);
void HHVM_METHOD(DateTime, __wakeup);
Array HHVM_METHOD(DateTime, __debuginfo);

///////////////////////////////////////////////////////////////////////////////
// class DateTimeZone

extern const int64_t q_DateTimeZone$$AFRICA;
extern const int64_t q_DateTimeZone$$AMERICA;
extern const int64_t q_DateTimeZone$$ANTARCTICA;
extern const int64_t q_DateTimeZone$$ARCTIC;
extern const int64_t q_DateTimeZone$$ASIA;
extern const int64_t q_DateTimeZone$$ATLANTIC;
extern const int64_t q_DateTimeZone$$AUSTRALIA;
extern const int64_t q_DateTimeZone$$EUROPE;
extern const int64_t q_DateTimeZone$$INDIAN;
extern const int64_t q_DateTimeZone$$PACIFIC;
extern const int64_t q_DateTimeZone$$UTC;
extern const int64_t q_DateTimeZone$$ALL;
extern const int64_t q_DateTimeZone$$ALL_WITH_BC;
extern const int64_t q_DateTimeZone$$PER_COUNTRY;

class DateTimeZoneData {
public:
  DateTimeZoneData() {}
  DateTimeZoneData(const DateTimeZoneData&) = delete;
  DateTimeZoneData& operator=(const DateTimeZoneData& other) {
    m_tz = other.m_tz->cloneTimeZone();
    return *this;
  }
  String getName() const {
    return m_tz->name();
  }

  static Object wrap(req::ptr<TimeZone> tz);
  static req::ptr<TimeZone> unwrap(const Object& timezone);
  static Class* getClass();

  req::ptr<TimeZone> m_tz;
  static Class* s_class;
  static const StaticString s_className;
};

void HHVM_METHOD(DateTimeZone, __construct,
                 const String& timezone);
Array HHVM_METHOD(DateTimeZone, getLocation);
String HHVM_METHOD(DateTimeZone, getName);
int64_t HHVM_METHOD(DateTimeZone, getOffset,
                    const Object& datetime);
Array HHVM_METHOD(DateTimeZone, getTransitions,
                  int64_t timestamp_begin = k_PHP_INT_MIN,
                  int64_t timestamp_end = k_PHP_INT_MAX);
Array HHVM_STATIC_METHOD(DateTimeZone, listAbbreviations);
Variant HHVM_STATIC_METHOD(DateTimeZone, listIdentifiers,
                           int64_t what,
                           const String& country);

///////////////////////////////////////////////////////////////////////////////
// class DateInterval

class DateIntervalData {
public:
  DateIntervalData() {}
  DateIntervalData(const DateIntervalData&) = delete;
  DateIntervalData& operator=(const DateIntervalData& other) {
    m_di = other.m_di->cloneDateInterval();
    return *this;
  }

  static Object wrap(req::ptr<DateInterval> di);
  static req::ptr<DateInterval> unwrap(const Object& di);
  static Class* getClass();

  req::ptr<DateInterval> m_di;
  static Class* s_class;
  static const StaticString s_className;
};

void HHVM_METHOD(DateInterval, __construct,
                 const String& interval_spec);
Variant HHVM_METHOD(DateInterval, __get,
                    const Variant& member);
Variant HHVM_METHOD(DateInterval, __set,
                    const Variant& member,
                    const Variant& value);
Object HHVM_STATIC_METHOD(DateInterval, createFromDateString,
                          const String& time);
String HHVM_METHOD(DateInterval, format,
                   const String& format);

///////////////////////////////////////////////////////////////////////////////
// timestamp

Variant HHVM_FUNCTION(gettimeofday,
                      bool return_float = false);
Variant HHVM_FUNCTION(microtime,
                      bool get_as_float = false);
int64_t HHVM_FUNCTION(time);
Variant HHVM_FUNCTION(mktime,
                      int64_t hour,
                      int64_t minute,
                      int64_t second,
                      int64_t month,
                      int64_t day,
                      int64_t year);
Variant HHVM_FUNCTION(gmmktime,
                      int64_t hour,
                      int64_t minute,
                      int64_t second,
                      int64_t month,
                      int64_t day,
                      int64_t year);
TypedValue* HHVM_FN(idate)(ActRec* ar);
TypedValue* HHVM_FN(date)(ActRec* ar);
TypedValue* HHVM_FN(gmdate)(ActRec* ar);
TypedValue* HHVM_FN(strftime)(ActRec* ar);
TypedValue* HHVM_FN(gmstrftime)(ActRec* ar);
TypedValue* HHVM_FN(getdate)(ActRec* ar);
TypedValue* HHVM_FN(localtime)(ActRec* ar);
Variant HHVM_FUNCTION(strptime,
                      const String& date,
                      const String& format);
TypedValue* HHVM_FN(strtotime)(ActRec* ar);

///////////////////////////////////////////////////////////////////////////////
// timezone

String HHVM_FUNCTION(date_default_timezone_get);
bool HHVM_FUNCTION(date_default_timezone_set,
                   const String& name);
Variant HHVM_FUNCTION(timezone_name_from_abbr,
                      const String& abbr,
                      int gmtoffset = -1,
                      int isdst = 1);
String HHVM_FUNCTION(timezone_version_get);

///////////////////////////////////////////////////////////////////////////////
// datetime

bool HHVM_FUNCTION(checkdate,
                   int month,
                   int day,
                   int year);
Variant HHVM_FUNCTION(date_create,
                      const Variant& time = null_variant,
                      const Variant& timezone = null_variant);
String HHVM_FUNCTION(date_format,
                     const Object& datetime,
                     const String& format);
Variant HHVM_FUNCTION(date_parse,
                      const String& date);
Object HHVM_FUNCTION(date_sub,
                     const Object& datetime,
                     const Object& interval);

///////////////////////////////////////////////////////////////////////////////
// sun

double get_date_default_latitude();
double get_date_default_longitude();
double get_date_default_sunset_zenith();
double get_date_default_sunrise_zenith();
double get_date_default_gmt_offset();

Array HHVM_FUNCTION(date_sun_info,
                    int64_t ts,
                    double latitude,
                    double longitude);
TypedValue* HHVM_FN(date_sunrise)(ActRec* ar);
TypedValue* HHVM_FN(date_sunset)(ActRec* ar);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_DATETIME_H_
