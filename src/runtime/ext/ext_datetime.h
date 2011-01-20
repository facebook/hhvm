/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef __EXT_DATETIME_H__
#define __EXT_DATETIME_H__

#include <runtime/base/base_includes.h>
#include <runtime/base/time/timestamp.h>
#include <runtime/base/time/datetime.h>
#include <runtime/base/time/timezone.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// class DateTime

extern const StaticString q_DateTime_ATOM;
extern const StaticString q_DateTime_COOKIE;
extern const StaticString q_DateTime_ISO8601;
extern const StaticString q_DateTime_RFC822;
extern const StaticString q_DateTime_RFC850;
extern const StaticString q_DateTime_RFC1036;
extern const StaticString q_DateTime_RFC1123;
extern const StaticString q_DateTime_RFC2822;
extern const StaticString q_DateTime_RFC3339;
extern const StaticString q_DateTime_RSS;
extern const StaticString q_DateTime_W3C;

FORWARD_DECLARE_CLASS_BUILTIN(DateTime);
class c_DateTime : public ExtObjectData {
 public:
  BEGIN_CLASS_MAP(DateTime)
  END_CLASS_MAP(DateTime)
  DECLARE_CLASS(DateTime, DateTime, ObjectData)

  // need to implement
  public: c_DateTime();
  public: ~c_DateTime();
  public: void t___construct(CStrRef time = "now",
                             CObjRef timezone = null_object);
  DECLARE_METHOD_INVOKE_HELPERS(__construct);
  public: static Object ti_createfromformat(const char* cls , CStrRef format,
                                            CStrRef time,
                                            CObjRef timezone = null_object);
  public: static Object t_createfromformat(CStrRef format, CStrRef time,
                                           CObjRef timezone = null_object) {
    return ti_createfromformat("datetime", format, time, timezone);
  }
  DECLARE_METHOD_INVOKE_HELPERS(createfromformat);
  public: Object t_add(CObjRef interval);
  DECLARE_METHOD_INVOKE_HELPERS(add);
  public: Object t_diff(CObjRef datetime2, bool absolute = false);
  DECLARE_METHOD_INVOKE_HELPERS(diff);
  public: String t_format(CStrRef format);
  DECLARE_METHOD_INVOKE_HELPERS(format);
  public: static Variant ti_getlasterrors(const char* cls );
  public: static Variant t_getlasterrors() {
    return ti_getlasterrors("datetime");
  }
  DECLARE_METHOD_INVOKE_HELPERS(getlasterrors);
  public: int64 t_getoffset();
  DECLARE_METHOD_INVOKE_HELPERS(getoffset);
  public: Variant t_gettimezone();
  DECLARE_METHOD_INVOKE_HELPERS(gettimezone);
  public: Object t_modify(CStrRef modify);
  DECLARE_METHOD_INVOKE_HELPERS(modify);
  public: Object t_setdate(int64 year, int64 month, int64 day);
  DECLARE_METHOD_INVOKE_HELPERS(setdate);
  public: Object t_setisodate(int64 year, int64 week, int64 day = 1);
  DECLARE_METHOD_INVOKE_HELPERS(setisodate);
  public: Object t_settime(int64 hour, int64 minute, int64 second = 0);
  DECLARE_METHOD_INVOKE_HELPERS(settime);
  public: Variant t_gettimestamp();
  DECLARE_METHOD_INVOKE_HELPERS(gettimestamp);
  public: Object t_settimestamp(int64 unixtimestamp);
  DECLARE_METHOD_INVOKE_HELPERS(settimestamp);
  public: Object t_settimezone(CObjRef timezone);
  DECLARE_METHOD_INVOKE_HELPERS(settimezone);
  public: Object t_sub(CObjRef interval);
  DECLARE_METHOD_INVOKE_HELPERS(sub);
  public: Variant t___destruct();
  DECLARE_METHOD_INVOKE_HELPERS(__destruct);

  // implemented by HPHP
  public: c_DateTime *create(String time = "now",
                             Object timezone = null_object);
  public: void dynConstruct(CArrRef Params);
  public: void getConstructor(MethodCallPackage &mcp);
  

  // Helper for DateTime -> c_DateTime conversion
  public: static Object wrap(SmartObject<DateTime> dt) {
    c_DateTime *cdt = NEWOBJ(c_DateTime)();
    Object ret(cdt);
    cdt->m_dt = dt;
    return ret;
  }

  // Helper for c_DateTime -> DateTime conversion
  public: static SmartObject<DateTime> unwrap(CObjRef datetime) {
    SmartObject<c_DateTime> cdt = datetime.getTyped<c_DateTime>(true);
    if (cdt.get() == NULL)
      return SmartObject<DateTime>();
    return cdt->m_dt;
  }

 private:
  SmartObject<DateTime> m_dt;
 public:
  virtual ObjectData *clone();
};

///////////////////////////////////////////////////////////////////////////////
// class DateTimeZone

extern const int64 q_DateTimeZone_AFRICA;
extern const int64 q_DateTimeZone_AMERICA;
extern const int64 q_DateTimeZone_ANTARCTICA;
extern const int64 q_DateTimeZone_ARCTIC;
extern const int64 q_DateTimeZone_ASIA;
extern const int64 q_DateTimeZone_ATLANTIC;
extern const int64 q_DateTimeZone_AUSTRALIA;
extern const int64 q_DateTimeZone_EUROPE;
extern const int64 q_DateTimeZone_INDIAN;
extern const int64 q_DateTimeZone_PACIFIC;
extern const int64 q_DateTimeZone_UTC;
extern const int64 q_DateTimeZone_ALL;
extern const int64 q_DateTimeZone_ALL_WITH_BC;
extern const int64 q_DateTimeZone_PER_COUNTRY;

FORWARD_DECLARE_CLASS_BUILTIN(DateTimeZone);
class c_DateTimeZone : public ExtObjectData {
 public:
  BEGIN_CLASS_MAP(DateTimeZone)
  END_CLASS_MAP(DateTimeZone)
  DECLARE_CLASS(DateTimeZone, DateTimeZone, ObjectData)

  // need to implement
  public: c_DateTimeZone();
  public: ~c_DateTimeZone();
  public: void t___construct(CStrRef timezone);
  DECLARE_METHOD_INVOKE_HELPERS(__construct);
  public: Array t_getlocation();
  DECLARE_METHOD_INVOKE_HELPERS(getlocation);
  public: String t_getname();
  DECLARE_METHOD_INVOKE_HELPERS(getname);
  public: int64 t_getoffset(CObjRef datetime);
  DECLARE_METHOD_INVOKE_HELPERS(getoffset);
  public: Array t_gettransitions(int64 timestamp_begin = LLONG_MIN,
                                 int64 timestamp_end = LLONG_MAX);
  DECLARE_METHOD_INVOKE_HELPERS(gettransitions);
  public: static Array ti_listabbreviations(const char* cls );
  public: static Array t_listabbreviations() {
    return ti_listabbreviations("datetimezone");
  }
  DECLARE_METHOD_INVOKE_HELPERS(listabbreviations);
  public: static Array ti_listidentifiers(const char* cls, int64 what = 2047,
                                          CStrRef country = null_string);
  public: static Array t_listidentifiers(int64 what = 2047,
                                         CStrRef country = null_string) {
    return ti_listidentifiers("datetimezone", what, country);
  }
  DECLARE_METHOD_INVOKE_HELPERS(listidentifiers);
  public: Variant t___destruct();
  DECLARE_METHOD_INVOKE_HELPERS(__destruct);

  // implemented by HPHP
  public: c_DateTimeZone *create(String timezone);
  public: void dynConstruct(CArrRef Params);
  public: void getConstructor(MethodCallPackage &mcp);
  

  // Helper for TimeZone -> c_DateTimeZone conversion
  public: static Object wrap(SmartObject<TimeZone> tz) {
    c_DateTimeZone *ctz = NEWOBJ(c_DateTimeZone)();
    Object ret(ctz);
    ctz->m_tz = tz;
    return ret;
  }

  // Helper for c_DateTimeZone -> TimeZone conversion
  public: static SmartObject<TimeZone> unwrap(CObjRef timezone) {
    SmartObject<c_DateTimeZone> ctz = timezone.getTyped<c_DateTimeZone>(true);
    if (ctz.get() == NULL)
      return SmartObject<TimeZone>();
    return ctz->m_tz;
  }

 private:
  SmartObject<TimeZone> m_tz;
 public:
  virtual ObjectData *clone();
};

///////////////////////////////////////////////////////////////////////////////
// class DateInterval

FORWARD_DECLARE_CLASS_BUILTIN(DateInterval);
class c_DateInterval : public ExtObjectData {
 public:
  BEGIN_CLASS_MAP(DateInterval)
  END_CLASS_MAP(DateInterval)
  DECLARE_CLASS(DateInterval, DateInterval, ObjectData)

  // properties
  public: int64 m_y;
  public: int64 m_m;
  public: int64 m_d;
  public: int64 m_h;
  public: int64 m_i;
  public: int64 m_s;
  public: int m_invert;
  public: Variant m_days;

  // need to implement
  public: c_DateInterval();
  public: ~c_DateInterval();
  public: void t___construct(CStrRef interval_spec);
  DECLARE_METHOD_INVOKE_HELPERS(__construct);
  public: static Object ti_createfromdatestring(const char* cls , CStrRef time);
  public: static Object t_createfromdatestring(CStrRef time) {
    return ti_createfromdatestring("dateinterval", time);
  }
  DECLARE_METHOD_INVOKE_HELPERS(createfromdatestring);
  public: String t_format(CStrRef format);
  DECLARE_METHOD_INVOKE_HELPERS(format);
  public: Variant t___destruct();
  DECLARE_METHOD_INVOKE_HELPERS(__destruct);

  // implemented by HPHP
  public: c_DateInterval *create(String interval_spec);
  public: void dynConstruct(CArrRef Params);
  public: void getConstructor(MethodCallPackage &mcp);

};

///////////////////////////////////////////////////////////////////////////////
// timestamp

inline Variant f_gettimeofday(bool return_float = false) {
  if (return_float) {
    return TimeStamp::CurrentSecond();
  }
  return TimeStamp::CurrentTime();
}

inline Variant f_microtime(bool get_as_float = false) {
  if (get_as_float) {
    return TimeStamp::CurrentSecond();
  }
  return TimeStamp::CurrentMicroTime();
}

inline int f_time() {
  return time(0);
}

inline Variant f_mktime(int hour = INT_MAX, int minute = INT_MAX,
                        int second = INT_MAX, int month = INT_MAX,
                        int day = INT_MAX, int year = INT_MAX) {
  bool error;
  int64 ts = TimeStamp::Get(error, hour, minute, second, month, day, year,
                            false);
  if (error) return false;
  return ts;
}

inline Variant f_gmmktime(int hour = INT_MAX, int minute = INT_MAX,
                          int second = INT_MAX,
                          int month = INT_MAX, int day = INT_MAX,
                          int year = INT_MAX) {
  bool error;
  int64 ts = TimeStamp::Get(error, hour, minute, second, month, day, year,
                            true);
  if (error) return false;
  return ts;
}

inline Variant f_idate(CStrRef format, int64 timestamp = TimeStamp::Current()) {
  if (format.size() != 1) {
    throw_invalid_argument("format: %s", format.data());
    return false;
  }
  int64 ret = DateTime(timestamp, false).toInteger(*format.data());
  if (ret == -1) return false;
  return ret;
}

inline Variant f_date(CStrRef format, int64 timestamp = TimeStamp::Current()) {
  if (format.empty()) return "";
  String ret = DateTime(timestamp, false).toString(format, false);
  if (ret.isNull()) return false;
  return ret;
}

inline Variant f_gmdate(CStrRef format,
                       int64 timestamp = TimeStamp::Current()) {
  String ret = DateTime(timestamp, true).toString(format, false);
  if (ret.isNull()) return false;
  return ret;
}

inline Variant f_strftime(CStrRef format,
                         int64 timestamp = TimeStamp::Current()) {
  String ret = DateTime(timestamp, false).toString(format, true);
  if (ret.isNull()) return false;
  return ret;
}

inline String f_gmstrftime(CStrRef format,
                           int64 timestamp = TimeStamp::Current()) {
  String ret = DateTime(timestamp, true).toString(format, true);
  if (ret.isNull()) return false;
  return ret;
}

inline Array f_getdate(int64 timestamp = TimeStamp::Current()) {
  return DateTime(timestamp, false).toArray(DateTime::TimeMap);
}

inline Array f_localtime(int64 timestamp = TimeStamp::Current(),
                         bool is_associative = false) {
  DateTime::ArrayFormat format =
    is_associative ? DateTime::TmMap : DateTime::TmVector;
  return DateTime(timestamp, false).toArray(format);
}

inline Variant f_strptime(CStrRef date, CStrRef format) {
  Array ret = DateTime::ParseCLib(date, format);
  if (ret.empty()) {
    return false;
  }
  return ret;
}

inline Variant f_strtotime(CStrRef input,
                           int64 timestamp = TimeStamp::Current()) {
  if (input.empty()) {
    return false;
  }

  DateTime dt(timestamp);
  if (!dt.fromString(input, SmartObject<TimeZone>())) {
    return false;
  }
  bool error;
  return dt.toTimeStamp(error);
}

///////////////////////////////////////////////////////////////////////////////
// timezone

inline String f_date_default_timezone_get() {
  return TimeZone::Current()->name();
}

inline bool f_date_default_timezone_set(CStrRef name) {
  return TimeZone::SetCurrent(name);
}

inline Array f_timezone_identifiers_list(int64 what = 2047,
                                         CStrRef country = null_string) {
  return c_DateTimeZone::t_listidentifiers(what, country);
}

inline Array f_timezone_abbreviations_list() {
  return c_DateTimeZone::t_listabbreviations();
}

inline Array f_timezone_location_get(CObjRef object) {
  return object.getTyped<c_DateTimeZone>()->t_getlocation();
}

inline Variant f_timezone_name_from_abbr(CStrRef abbr, int gmtoffset = -1,
                                         bool isdst = true) {
  String ret = TimeZone::AbbreviationToName(abbr, gmtoffset, isdst);
  if (ret.isNull()) {
    return false;
  }
  return ret;
}

inline Object f_timezone_open(CStrRef timezone) {
  c_DateTimeZone *ctz = NEWOBJ(c_DateTimeZone)();
  Object ret(ctz);
  ctz->t___construct(timezone);
  return ret;
}

inline String f_timezone_name_get(CObjRef object) {
  return object.getTyped<c_DateTimeZone>()->t_getname();
}

inline int f_timezone_offset_get(CObjRef object, CObjRef dt) {
  return object.getTyped<c_DateTimeZone>()->t_getoffset(dt);
}

inline Array f_timezone_transitions_get(CObjRef object,
    int64 timestamp_begin = LLONG_MIN, int64 timestamp_end = LLONG_MAX) {
  return object.getTyped<c_DateTimeZone>()->t_gettransitions(timestamp_begin,
                                                             timestamp_end);
}

///////////////////////////////////////////////////////////////////////////////
// datetime

inline bool f_checkdate(int month, int day, int year) {
  return DateTime::IsValid(year, month, day);
}

inline Object f_date_add(CObjRef object, CObjRef interval) {
  return object.getTyped<c_DateTime>()->t_add(interval);
}

inline Object f_date_create_from_format(CStrRef format, CStrRef time,
                                        CObjRef timezone = null_object) {
  return c_DateTime::t_createfromformat(format, time, timezone);
}

inline Object f_date_create(CStrRef time = null_string,
                            CObjRef timezone = null_object) {
  c_DateTime *cdt = NEWOBJ(c_DateTime)();
  Object ret(cdt);
  cdt->t___construct(time, timezone);
  return ret;
}

inline void f_date_date_set(CObjRef object, int year, int month, int day) {
  object.getTyped<c_DateTime>()->t_setdate(year, month, day);
}

inline void f_date_isodate_set(CObjRef object, int year, int week,
                               int day = 1) {
  object.getTyped<c_DateTime>()->t_setisodate(year, week, day);
}

inline Object f_date_diff(CObjRef datetime1, CObjRef datetime2,
                          bool absolute = false) {
  return datetime1.getTyped<c_DateTime>()->t_diff(datetime2, absolute);
}

inline String f_date_format(CObjRef object, CStrRef format) {
  return object.getTyped<c_DateTime>()->t_format(format);
}

inline Variant f_date_get_last_errors() {
  return c_DateTime::t_getlasterrors();
}

inline Object f_date_interval_create_from_date_string(CStrRef time) {
  return c_DateInterval::t_createfromdatestring(time);
}

inline String f_date_interval_format(CObjRef object, CStrRef format) {
  return object.getTyped<c_DateInterval>()->t_format(format);
}

inline void f_date_modify(CObjRef object, CStrRef modify) {
  object.getTyped<c_DateTime>()->t_modify(modify);
}

inline int f_date_offset_get(CObjRef object) {
  return object.getTyped<c_DateTime>()->t_getoffset();
}

inline Array f_date_parse_from_format(CStrRef format, CStrRef date) {
  return DateTime::Parse(date, format);
}

inline Variant f_date_parse(CStrRef date) {
  return DateTime::Parse(date);
}

inline Object f_date_sub(CObjRef object, CObjRef interval) {
  return object.getTyped<c_DateTime>()->t_sub(interval);
}

inline void f_date_time_set(CObjRef object, int hour, int minute,
                            int second = 0) {
  object.getTyped<c_DateTime>()->t_settime(hour, minute, second);
}

inline Variant f_date_timestamp_get(CObjRef object) {
  return object.getTyped<c_DateTime>()->t_gettimestamp();
}

inline Object f_date_timestamp_set(CObjRef object, int64 unixtimestamp) {
  object.getTyped<c_DateTime>()->t_settimestamp(unixtimestamp);
  return object;
}

inline Variant f_date_timezone_get(CObjRef object) {
  return object.getTyped<c_DateTime>()->t_gettimezone();
}

inline void f_date_timezone_set(CObjRef object, CObjRef timezone) {
  object.getTyped<c_DateTime>()->t_settimezone(timezone);
}

///////////////////////////////////////////////////////////////////////////////
// sun

inline Array f_date_sun_info(int64 ts, double latitude, double longitude) {
  return DateTime(ts, false).getSunInfo(latitude, longitude);
}

inline Variant f_date_sunrise(int64 timestamp, int format = 0,
                              double latitude = 0.0, double longitude = 0.0,
                              double zenith = 0.0,
                              double gmt_offset = 99999.0) {
  return DateTime(timestamp, false).getSunInfo
    (static_cast<DateTime::SunInfoFormat>(format), latitude, longitude,
     zenith, gmt_offset, false);
}

inline Variant f_date_sunset(int64 timestamp, int format = 0,
                             double latitude = 0.0, double longitude = 0.0,
                             double zenith = 0.0,
                             double gmt_offset = 99999.0) {
  return DateTime(timestamp, false).getSunInfo
    (static_cast<DateTime::SunInfoFormat>(format), latitude, longitude,
     zenith, gmt_offset, true);
}

///////////////////////////////////////////////////////////////////////////////
}

#endif // __EXT_DATETIME_H__
