/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/base/timestamp.h"
#include "hphp/runtime/base/datetime.h"
#include "hphp/runtime/base/timezone.h"
#include "hphp/runtime/base/dateinterval.h"

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

FORWARD_DECLARE_CLASS(DateTime);
class c_DateTime : public ExtObjectDataFlags<ObjectData::HasClone> {
 public:
  DECLARE_CLASS_NO_SWEEP(DateTime)

  // need to implement
  c_DateTime(Class* cls = c_DateTime::classof())
    : ExtObjectDataFlags(cls)
  {}
  ~c_DateTime() {}

  public: Object t_add(CObjRef interval);
  public: void t___construct(const String& time = "now",
                             CObjRef timezone = null_object);
  public: static Object ti_createfromformat(
    const String& format, const String& time, CObjRef timezone = null_object);
  public: Object t_diff(CObjRef datetime2, bool absolute = false);
  public: String t_format(const String& format);
  public: static Array ti_getlasterrors();
  public: int64_t t_getoffset();
  public: int64_t t_gettimestamp();
  public: Variant t_gettimezone();
  public: Object t_modify(const String& modify);
  public: Object t_setdate(int64_t year, int64_t month, int64_t day);
  public: Object t_setisodate(int64_t year, int64_t week, int64_t day = 1);
  public: Object t_settime(int64_t hour, int64_t minute, int64_t second = 0);
  public: Object t_settimestamp(int64_t unixtimestamp);
  public: Object t_settimezone(CObjRef timezone);
  public: Object t_sub(CObjRef interval);
  public: void t___wakeup();
  public: Array t___sleep();

  int64_t gettimestamp() const;

  // Helper for DateTime -> c_DateTime conversion
  public: static Object wrap(SmartResource<DateTime> dt) {
    c_DateTime *cdt = NEWOBJ(c_DateTime)();
    Object ret(cdt);
    cdt->m_dt = dt;
    return ret;
  }

  // Helper for c_DateTime -> DateTime conversion
  public: static SmartResource<DateTime> unwrap(CObjRef datetime) {
    SmartObject<c_DateTime> cdt = datetime.getTyped<c_DateTime>(true);
    if (cdt.get() == NULL)
      return SmartResource<DateTime>();
    return cdt->m_dt;
  }

 private:
  SmartResource<DateTime> m_dt;
 public:
  static c_DateTime* Clone(ObjectData* obj);
};

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

FORWARD_DECLARE_CLASS(DateTimeZone);
class c_DateTimeZone : public ExtObjectDataFlags<ObjectData::HasClone> {
 public:
  DECLARE_CLASS_NO_SWEEP(DateTimeZone)

  // need to implement
  c_DateTimeZone(Class* cls = c_DateTimeZone::classof())
    : ExtObjectDataFlags(cls)
  {}
  ~c_DateTimeZone() {}

  public: void t___construct(const String& timezone);
  public: Array t_getlocation();
  public: String t_getname();
  public: int64_t t_getoffset(CObjRef datetime);
  public: Array t_gettransitions();
  public: static Array ti_listabbreviations();
  public: static Array ti_listidentifiers();

  // Helper for TimeZone -> c_DateTimeZone conversion
  public: static Object wrap(SmartResource<TimeZone> tz) {
    c_DateTimeZone *ctz = NEWOBJ(c_DateTimeZone)();
    Object ret(ctz);
    ctz->m_tz = tz;
    return ret;
  }

  // Helper for c_DateTimeZone -> TimeZone conversion
  public: static SmartResource<TimeZone> unwrap(CObjRef timezone) {
    SmartObject<c_DateTimeZone> ctz = timezone.getTyped<c_DateTimeZone>(true);
    if (ctz.get() == NULL)
      return SmartResource<TimeZone>();
    return ctz->m_tz;
  }

 private:
  SmartResource<TimeZone> m_tz;
 public:
  static c_DateTimeZone* Clone(ObjectData* obj);
};

///////////////////////////////////////////////////////////////////////////////
// class DateInterval

FORWARD_DECLARE_CLASS(DateInterval);
class c_DateInterval : public ExtObjectDataFlags<ObjectData::UseGet|
                                                 ObjectData::UseSet|
                                                 ObjectData::HasClone> {
 public:
  DECLARE_CLASS_NO_SWEEP(DateInterval)

  // need to implement
  c_DateInterval(Class* cls = c_DateInterval::classof())
    : ExtObjectDataFlags(cls)
  {}
  ~c_DateInterval() {}

  public: void t___construct(const String& interval_spec);
  public: Variant t___get(Variant member);
  public: Variant t___set(Variant member, Variant value);
  public: static Object ti_createfromdatestring(const String& time);
  public: String t_format(const String& format);


  public: static Object wrap(SmartResource<DateInterval> di) {
    c_DateInterval *cdi = NEWOBJ(c_DateInterval)();
    Object ret(cdi);
    cdi->m_di = di;
    return ret;
  }

  public: static SmartResource<DateInterval> unwrap(CObjRef dateinterval) {
    SmartObject<c_DateInterval>
      cdi = dateinterval.getTyped<c_DateInterval>(true);
    if (cdi.get() == NULL)
      return SmartResource<DateInterval>();
    return cdi->m_di;
  }

 private:
  SmartResource<DateInterval> m_di;
 public:
  static c_DateInterval* Clone(ObjectData* obj);

};

///////////////////////////////////////////////////////////////////////////////
// timestamp

Variant f_gettimeofday(bool return_float = false);
Variant f_microtime(bool get_as_float = false);
int64_t f_time();
Variant f_mktime(int hour = INT_MAX, int minute = INT_MAX,
                 int second = INT_MAX, int month = INT_MAX,
                 int day = INT_MAX, int year = INT_MAX);
Variant f_gmmktime(int hour = INT_MAX, int minute = INT_MAX,
                   int second = INT_MAX,
                   int month = INT_MAX, int day = INT_MAX,
                   int year = INT_MAX);
Variant f_idate(const String& format, int64_t timestamp = TimeStamp::Current());
Variant f_date(const String& format, int64_t timestamp = TimeStamp::Current());
Variant f_gmdate(const String& format,
                 int64_t timestamp = TimeStamp::Current());
Variant f_strftime(const String& format,
                   int64_t timestamp = TimeStamp::Current());
String f_gmstrftime(const String& format,
                    int64_t timestamp = TimeStamp::Current());
Array f_getdate(int64_t timestamp = TimeStamp::Current());
Array f_localtime(int64_t timestamp = TimeStamp::Current(),
                  bool is_associative = false);
Variant f_strptime(const String& date, const String& format);
Variant f_strtotime(const String& input,
                    int64_t timestamp = TimeStamp::Current());

///////////////////////////////////////////////////////////////////////////////
// timezone

String f_date_default_timezone_get();
bool f_date_default_timezone_set(const String& name);
Array f_timezone_identifiers_list();
Array f_timezone_abbreviations_list();
Variant f_timezone_name_from_abbr(const String& abbr, int gmtoffset = -1,
                                  bool isdst = true);
Object f_timezone_open(const String& timezone);
Array f_timezone_location_get(CObjRef timezone);
String f_timezone_name_get(CObjRef object);
int64_t f_timezone_offset_get(CObjRef object, CObjRef dt);
Array f_timezone_transitions_get(CObjRef object);
String f_timezone_version_get();

///////////////////////////////////////////////////////////////////////////////
// datetime

bool f_checkdate(int month, int day, int year);
Object f_date_add(CObjRef datetime, CObjRef interval);
Object f_date_create_from_format(const String& format,
                                 const String& time,
                                 CObjRef timezone = null_object);
Variant f_date_create(const String& time = null_string,
                      CObjRef timezone = null_object);
void f_date_date_set(CObjRef object, int year, int month, int day);
Object f_date_diff(CObjRef datetime,
                   CObjRef datetime2,
                   bool absolute = false);
void f_date_isodate_set(CObjRef object, int year, int week,
                        int day = 1);
String f_date_format(CObjRef object, const String& format);
Array f_date_get_last_errors();
Object f_date_interval_create_from_date_string(const String& time);
String f_date_interval_format(CObjRef interval, const String& format_spec);
void f_date_modify(CObjRef object, const String& modify);
int64_t f_date_offset_get(CObjRef object);
Variant f_date_parse(const String& date);
void f_date_time_set(CObjRef object, int hour, int minute,
                     int second = 0);
int64_t f_date_timestamp_get(CObjRef datetime);
Object f_date_timestamp_set(CObjRef datetime, int64_t timestamp);
Variant f_date_timezone_get(CObjRef object);
void f_date_timezone_set(CObjRef object, CObjRef timezone);
Object f_date_sub(CObjRef datetime, CObjRef interval);

///////////////////////////////////////////////////////////////////////////////
// sun

Array f_date_sun_info(int64_t ts, double latitude, double longitude);
Variant f_date_sunrise(int64_t timestamp, int format = 0,
                       double latitude = 0.0, double longitude = 0.0,
                       double zenith = 0.0,
                       double gmt_offset = 99999.0);
Variant f_date_sunset(int64_t timestamp, int format = 0,
                      double latitude = 0.0, double longitude = 0.0,
                      double zenith = 0.0,
                      double gmt_offset = 99999.0);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_DATETIME_H_
