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

#include "hphp/runtime/ext/ext_datetime.h"
#include "hphp/runtime/base/ini-setting.h"

#include "hphp/system/systemlib.h"

namespace HPHP {
static class DateExtension : public Extension {
 public:
  DateExtension() : Extension("date") { }
  void moduleInit() {
    IniSetting::Bind(
      "date.timezone",
      g_context->getDefaultTimeZone().c_str(),
      dateTimezoneIniUpdate, dateTimezoneIniGet,
      nullptr
    );
  }

 private:
  static bool dateTimezoneIniUpdate(const HPHP::String& value, void *p) {
    assert(p == nullptr);
    if (value.empty()) {
      return false;
    }
    return f_date_default_timezone_set(value);
  }

  static String dateTimezoneIniGet(void* p) {
    auto ret = g_context->getTimeZone();
    if (ret.isNull()) {
      return empty_string;
    }
    return ret;
  }
} s_date_extension;

///////////////////////////////////////////////////////////////////////////////
// constants

const StaticString q_DateTime$$ATOM(LITSTR_INIT("Y-m-d\\TH:i:sP"));
const StaticString q_DateTime$$COOKIE(LITSTR_INIT("l, d-M-y H:i:s T"));
const StaticString q_DateTime$$ISO8601(LITSTR_INIT("Y-m-d\\TH:i:sO"));
const StaticString q_DateTime$$RFC822(LITSTR_INIT("D, d M y H:i:s O"));
const StaticString q_DateTime$$RFC850(LITSTR_INIT("l, d-M-y H:i:s T"));
const StaticString q_DateTime$$RFC1036(LITSTR_INIT("D, d M y H:i:s O"));
const StaticString q_DateTime$$RFC1123(LITSTR_INIT("D, d M Y H:i:s O"));
const StaticString q_DateTime$$RFC2822(LITSTR_INIT("D, d M Y H:i:s O"));
const StaticString q_DateTime$$RFC3339(LITSTR_INIT("Y-m-d\\TH:i:sP"));
const StaticString q_DateTime$$RSS(LITSTR_INIT("D, d M Y H:i:s O"));
const StaticString q_DateTime$$W3C(LITSTR_INIT("Y-m-d\\TH:i:sP"));

const int64_t q_DateTimeZone$$AFRICA = 1;
const int64_t q_DateTimeZone$$AMERICA = 2;
const int64_t q_DateTimeZone$$ANTARCTICA = 4;
const int64_t q_DateTimeZone$$ARCTIC = 8;
const int64_t q_DateTimeZone$$ASIA = 16;
const int64_t q_DateTimeZone$$ATLANTIC = 32;
const int64_t q_DateTimeZone$$AUSTRALIA = 64;
const int64_t q_DateTimeZone$$EUROPE = 128;
const int64_t q_DateTimeZone$$INDIAN = 256;
const int64_t q_DateTimeZone$$PACIFIC = 512;
const int64_t q_DateTimeZone$$UTC = 1024;
const int64_t q_DateTimeZone$$ALL = 2047;
const int64_t q_DateTimeZone$$ALL_WITH_BC = 4095;
const int64_t q_DateTimeZone$$PER_COUNTRY = 4096;

///////////////////////////////////////////////////////////////////////////////
// methods

Object c_DateTime::t_add(CObjRef interval) {
  m_dt->add(c_DateInterval::unwrap(interval));
  return this;
}

void c_DateTime::t___construct(const String& time /*= "now"*/,
                               CObjRef timezone /*= null_object*/) {
  m_dt = NEWOBJ(DateTime)(TimeStamp::Current());
  if (!time.empty()) {
    m_dt->fromString(time, c_DateTimeZone::unwrap(timezone));
  } else if (!timezone.isNull()) {
    // We still have to tell the underlying DateTime the timezone incase they
    // call setTimestamp or something else later
    m_dt->setTimezone(c_DateTimeZone::unwrap(timezone));
  }
}

Object c_DateTime::ti_createfromformat(const String& format, const String& time,
                                       CObjRef timezone /*= null_object */) {
  c_DateTime *datetime = NEWOBJ(c_DateTime);
  datetime->m_dt = NEWOBJ(DateTime);
  datetime->m_dt->fromString(time, c_DateTimeZone::unwrap(timezone),
                             format.data(), false);
  return datetime;
}

Object c_DateTime::t_diff(CObjRef datetime2, bool absolute) {
  return c_DateInterval::wrap(m_dt->diff(c_DateTime::unwrap(datetime2),
                                         absolute));
}

String c_DateTime::t_format(const String& format) {
  return m_dt->toString(format, false);
}

const StaticString
  s_warning_count("warning_count"),
  s_warnings("warnings"),
  s_error_count("error_count"),
  s_errors("errors");

Array c_DateTime::ti_getlasterrors() {
  Array errors = DateTime::getLastErrors();
  Array warnings = DateTime::getLastWarnings();
  Array ret = Array::Create();

  ret.add(s_warning_count, warnings.size());
  ret.add(s_warnings, warnings);
  ret.add(s_error_count, errors.size());
  ret.add(s_errors, errors);

  return ret;
}

int64_t c_DateTime::t_getoffset() {
  return m_dt->offset();
}

int64_t c_DateTime::t_gettimestamp() {
  return gettimestamp();
}

int64_t c_DateTime::gettimestamp() const {
  bool err = false;
  return m_dt->toTimeStamp(err);
}

Variant c_DateTime::t_gettimezone() {
  SmartResource<TimeZone> tz = m_dt->timezone();
  if (tz->isValid()) {
    return c_DateTimeZone::wrap(tz);
  }
  return false;
}

Object c_DateTime::t_modify(const String& modify) {
  m_dt->modify(modify);
  return this;
}

Object c_DateTime::t_setdate(int64_t year, int64_t month, int64_t day) {
  m_dt->setDate(year, month, day);
  return this;
}

Object c_DateTime::t_setisodate(int64_t year, int64_t week,
                                int64_t day /*= 1*/) {
  m_dt->setISODate(year, week, day);
  return this;
}

Object c_DateTime::t_settime(int64_t hour, int64_t minute,
                             int64_t second /*= 0*/) {
  m_dt->setTime(hour, minute, second);
  return this;
}

Object c_DateTime::t_settimestamp(int64_t unixtimestamp) {
  m_dt->fromTimeStamp(unixtimestamp, false);
  return this;
}

Object c_DateTime::t_settimezone(CObjRef timezone) {
  m_dt->setTimezone(c_DateTimeZone::unwrap(timezone));
  return this;
}

Object c_DateTime::t_sub(CObjRef interval) {
  m_dt->sub(c_DateInterval::unwrap(interval));
  return this;
}

const StaticString
  s_c("c"),
  s__date_time("_date_time");

Array c_DateTime::t___sleep() {
  o_set(s__date_time, t_format(s_c));
  return make_packed_array(s__date_time);
}

void c_DateTime::t___wakeup() {
  t___construct(o_get(s__date_time));
  unsetProp(getVMClass(), s__date_time.get());
}

c_DateTime* c_DateTime::Clone(ObjectData* obj) {
  c_DateTime* dt = static_cast<c_DateTime*>(obj->cloneImpl());
  dt->m_dt = static_cast<c_DateTime*>(obj)->m_dt->cloneDateTime();
  return dt;
}

void c_DateTimeZone::t___construct(const String& timezone) {
  m_tz = NEWOBJ(TimeZone)(timezone);
  if (!m_tz->isValid()) {
    std::string msg = "DateTimeZone::__construct(): Unknown or bad timezone (";
    msg += timezone.data();
    msg += ")";
    throw Object(SystemLib::AllocExceptionObject(msg));
  }
}

Array c_DateTimeZone::t_getlocation() {
  return m_tz->getLocation();
}

String c_DateTimeZone::t_getname() {
  return m_tz->name();
}

int64_t c_DateTimeZone::t_getoffset(CObjRef datetime) {
  bool error;
  int64_t ts = c_DateTime::unwrap(datetime)->toTimeStamp(error);
  return m_tz->offset(ts);
}

Array c_DateTimeZone::t_gettransitions() {
  return m_tz->transitions();
}

Array c_DateTimeZone::ti_listabbreviations() {
  return TimeZone::GetAbbreviations();
}

Array c_DateTimeZone::ti_listidentifiers() {
  return TimeZone::GetNames();
}

c_DateTimeZone* c_DateTimeZone::Clone(ObjectData* obj) {
  c_DateTimeZone* dtz = static_cast<c_DateTimeZone*>(obj->cloneImpl());
  dtz->m_tz = static_cast<c_DateTimeZone*>(obj)->m_tz->cloneTimeZone();
  return dtz;
}

void c_DateInterval::t___construct(const String& interval_spec) {
  m_di = NEWOBJ(DateInterval)(interval_spec);
  if (!m_di->isValid()) {
    std::string msg = "DateInterval::__construct: Invalid interval (";
    msg += interval_spec.data();
    msg += ")";
    throw Object(SystemLib::AllocExceptionObject(msg));
  }
}

const StaticString
  s_y("y"),
  s_m("m"),
  s_d("d"),
  s_h("h"),
  s_i("i"),
  s_s("s"),
  s_invert("invert"),
  s_days("days");

Variant c_DateInterval::t___get(Variant member) {
  if (member.isString()) {
    if (same(member, s_y))      return m_di->getYears();
    if (same(member, s_m))      return m_di->getMonths();
    if (same(member, s_d))      return m_di->getDays();
    if (same(member, s_h))      return m_di->getHours();
    if (same(member, s_i))      return m_di->getMinutes();
    if (same(member, s_s))      return m_di->getSeconds();
    if (same(member, s_invert)) return m_di->isInverted();
    if (same(member, s_days)) {
      if (m_di->haveTotalDays()) {
        return m_di->getTotalDays();
      } else {
        return false;
      }
    }
  }
  std::string msg = "Undefined property '";
  msg += member.toString().data();
  msg += ") on DateInterval object";
  throw Object(SystemLib::AllocExceptionObject(msg));
}

Variant c_DateInterval::t___set(Variant member, Variant value) {
  if (member.isString()) {
    if (same(member, s_y)) {
      m_di->setYears(value.toInt64());
      return uninit_null();
    }
    if (same(member, s_m)) {
      m_di->setMonths(value.toInt64());
      return uninit_null();
    }
    if (same(member, s_d)) {
      m_di->setDays(value.toInt64());
      return uninit_null();
    }
    if (same(member, s_h)) {
      m_di->setHours(value.toInt64());
      return uninit_null();
    }
    if (same(member, s_i)) {
      m_di->setMinutes(value.toInt64());
      return uninit_null();
    }
    if (same(member, s_s)) {
      m_di->setSeconds(value.toInt64());
      return uninit_null();
    }
    if (same(member, s_invert)) {
      m_di->setInverted(value.toBoolean());
      return uninit_null();
    }
    if (same(member, s_days)) {
      m_di->setTotalDays(value.toInt64());
      return uninit_null();
    }
  }

  std::string msg = "Undefined property '";
  msg += member.toString().data();
  msg += ") on DateInterval object";
  throw Object(SystemLib::AllocExceptionObject(msg));
}

Object c_DateInterval::ti_createfromdatestring(const String& time) {
  SmartResource<DateInterval> di(NEWOBJ(DateInterval)(time, true));
  return c_DateInterval::wrap(di);
}

String c_DateInterval::t_format(const String& format) {
  return m_di->format(format);
}

c_DateInterval* c_DateInterval::Clone(ObjectData* obj) {
  c_DateInterval *di = static_cast<c_DateInterval*>(obj->cloneImpl());
  di->m_di = static_cast<c_DateInterval*>(obj)->m_di->cloneDateInterval();
  return di;
}

///////////////////////////////////////////////////////////////////////////////
// timestamp

Variant f_gettimeofday(bool return_float /* = false */) {
  if (return_float) {
    return TimeStamp::CurrentSecond();
  }
  return TimeStamp::CurrentTime();
}

Variant f_microtime(bool get_as_float /* = false */) {
  if (get_as_float) {
    return TimeStamp::CurrentSecond();
  }
  return TimeStamp::CurrentMicroTime();
}

int64_t f_time() {
  return time(0);
}

Variant f_mktime(int hour /* = INT_MAX */, int minute /* = INT_MAX */,
                 int second /* = INT_MAX */, int month /* = INT_MAX */,
                 int day /* = INT_MAX */, int year /* = INT_MAX */) {
  bool error;
  int64_t ts = TimeStamp::Get(error, hour, minute, second, month, day, year,
                              false);
  if (error) return false;
  return ts;
}

Variant f_gmmktime(int hour /* = INT_MAX */, int minute /* = INT_MAX */,
                   int second /* = INT_MAX */,
                   int month /* = INT_MAX */, int day /* = INT_MAX */,
                   int year /* = INT_MAX */) {
  bool error;
  int64_t ts = TimeStamp::Get(error, hour, minute, second, month, day, year,
                              true);
  if (error) return false;
  return ts;
}

Variant f_idate(const String& format,
                int64_t timestamp /* = TimeStamp::Current() */) {
  if (format.size() != 1) {
    throw_invalid_argument("format: %s", format.data());
    return false;
  }
  int64_t ret = DateTime(timestamp, false).toInteger(*format.data());
  if (ret == -1) return false;
  return ret;
}

Variant f_date(const String& format,
               int64_t timestamp /* = TimeStamp::Current() */) {
  if (format.empty()) return "";
  String ret = DateTime(timestamp, false).toString(format, false);
  if (ret.isNull()) return false;
  return ret;
}

Variant f_gmdate(const String& format,
                 int64_t timestamp /* = TimeStamp::Current() */) {
  String ret = DateTime(timestamp, true).toString(format, false);
  if (ret.isNull()) return false;
  return ret;
}

Variant f_strftime(const String& format,
                   int64_t timestamp /* = TimeStamp::Current() */) {
  String ret = DateTime(timestamp, false).toString(format, true);
  if (ret.isNull()) return false;
  return ret;
}

String f_gmstrftime(const String& format,
                    int64_t timestamp /* = TimeStamp::Current() */) {
  String ret = DateTime(timestamp, true).toString(format, true);
  if (ret.isNull()) return false;
  return ret;
}

Array f_getdate(int64_t timestamp /* = TimeStamp::Current() */) {
  return DateTime(timestamp, false).toArray(DateTime::ArrayFormat::TimeMap);
}

Array f_localtime(int64_t timestamp /* = TimeStamp::Current() */,
                  bool is_associative /* = false */) {
  DateTime::ArrayFormat format =
    is_associative ? DateTime::ArrayFormat::TmMap :
                     DateTime::ArrayFormat::TmVector;
  return DateTime(timestamp, false).toArray(format);
}

Variant f_strptime(const String& date, const String& format) {
  Array ret = DateTime::ParseAsStrptime(format, date);
  if (ret.empty()) {
    return false;
  }
  return ret;
}

Variant f_strtotime(const String& input,
                    int64_t timestamp /* = TimeStamp::Current() */) {
  if (input.empty()) {
    return false;
  }

  DateTime dt(timestamp);
  if (!dt.fromString(input, SmartResource<TimeZone>(), nullptr, false)) {
    return false;
  }
  bool error;
  return dt.toTimeStamp(error);
}

///////////////////////////////////////////////////////////////////////////////
// timezone

String f_date_default_timezone_get() {
  return TimeZone::Current()->name();
}

bool f_date_default_timezone_set(const String& name) {
  return TimeZone::SetCurrent(name);
}

Array f_timezone_identifiers_list() {
  return c_DateTimeZone::ti_listidentifiers();
}

Array f_timezone_abbreviations_list() {
  return c_DateTimeZone::ti_listabbreviations();
}

Variant f_timezone_name_from_abbr(const String& abbr, int gmtoffset /* = -1 */,
                                  bool isdst /* = true */) {
  String ret = TimeZone::AbbreviationToName(abbr, gmtoffset, isdst);
  if (ret.isNull()) {
    return false;
  }
  return ret;
}

Object f_timezone_open(const String& timezone) {
  c_DateTimeZone *ctz = NEWOBJ(c_DateTimeZone)();
  Object ret(ctz);
  ctz->t___construct(timezone);
  return ret;
}

Array f_timezone_location_get(CObjRef timezone) {
  return timezone.getTyped<c_DateTimeZone>()->t_getlocation();
}

String f_timezone_name_get(CObjRef object) {
  return object.getTyped<c_DateTimeZone>()->t_getname();
}

int64_t f_timezone_offset_get(CObjRef object, CObjRef dt) {
  return object.getTyped<c_DateTimeZone>()->t_getoffset(dt);
}

Array f_timezone_transitions_get(CObjRef object) {
  return object.getTyped<c_DateTimeZone>()->t_gettransitions();
}

String f_timezone_version_get() {
  return TimeZone::getVersion();
}

///////////////////////////////////////////////////////////////////////////////
// datetime

bool f_checkdate(int month, int day, int year) {
  return DateTime::IsValid(year, month, day);
}

Object f_date_add(CObjRef datetime, CObjRef interval) {
  return datetime.getTyped<c_DateTime>()->
    t_add(interval.getTyped<c_DateInterval>());
}

Object f_date_create_from_format(const String& format,
                                 const String& time,
                                 CObjRef timezone /* = null_object */) {
  return c_DateTime::ti_createfromformat(format, time, timezone);
}

Variant f_date_parse_from_format(const String& format, const String& date) {
  Array ret = DateTime::Parse(format, date);
  if (ret.empty()) {
    return false;
  }
  return ret;
}

Variant f_date_create(const String& time /* = null_string */,
                      CObjRef timezone /* = null_object */) {
  c_DateTime *cdt = NEWOBJ(c_DateTime)();
  Object ret(cdt);
  // Don't set the time here because it will throw if it is bad
  cdt->t___construct();
  auto dt = c_DateTime::unwrap(ret);
  if (!dt->fromString(time, c_DateTimeZone::unwrap(timezone), nullptr, false)) {
    return false;
  }
  return ret;
}

void f_date_date_set(CObjRef object, int year, int month, int day) {
  object.getTyped<c_DateTime>()->t_setdate(year, month, day);
}

Object f_date_diff(CObjRef datetime,
                   CObjRef datetime2,
                   bool absolute /* = false */) {
  return datetime.getTyped<c_DateTime>()->
    t_diff(datetime2.getTyped<c_DateTime>(), absolute);
}

void f_date_isodate_set(CObjRef object, int year, int week,
                        int day /* = 1 */) {
  object.getTyped<c_DateTime>()->t_setisodate(year, week, day);
}

String f_date_format(CObjRef object, const String& format) {
  return object.getTyped<c_DateTime>()->t_format(format);
}

Array f_date_get_last_errors() {
  return c_DateTime::ti_getlasterrors();
}

Object f_date_interval_create_from_date_string(const String& time) {
  return c_DateInterval::ti_createfromdatestring(time);
}

String f_date_interval_format(CObjRef interval, const String& format_spec) {
  return interval.getTyped<c_DateInterval>()->t_format(format_spec);
}

void f_date_modify(CObjRef object, const String& modify) {
  object.getTyped<c_DateTime>()->t_modify(modify);
}

int64_t f_date_offset_get(CObjRef object) {
  return object.getTyped<c_DateTime>()->t_getoffset();
}

Variant f_date_parse(const String& date) {
  Array ret = DateTime::Parse(date);
  if (ret.empty()) {
    return false;
  }
  return ret;
}

void f_date_time_set(CObjRef object, int hour, int minute,
                     int second /* = 0 */) {
  object.getTyped<c_DateTime>()->t_settime(hour, minute, second);
}

int64_t f_date_timestamp_get(CObjRef datetime) {
  return datetime.getTyped<c_DateTime>()->t_gettimestamp();
}

Object f_date_timestamp_set(CObjRef datetime, int64_t timestamp) {
  return datetime.getTyped<c_DateTime>()->
    t_settimestamp(timestamp);
}

Variant f_date_timezone_get(CObjRef object) {
  return object.getTyped<c_DateTime>()->t_gettimezone();
}

void f_date_timezone_set(CObjRef object, CObjRef timezone) {
  object.getTyped<c_DateTime>()->t_settimezone(timezone);
}

Object f_date_sub(CObjRef datetime, CObjRef interval) {
  return datetime.getTyped<c_DateTime>()->
    t_sub(interval.getTyped<c_DateInterval>());
}

///////////////////////////////////////////////////////////////////////////////
// sun

Array f_date_sun_info(int64_t ts, double latitude, double longitude) {
  return DateTime(ts, false).getSunInfo(latitude, longitude);
}

Variant f_date_sunrise(int64_t timestamp, int format /* = 0 */,
                       double latitude /* = 0.0 */, double longitude /* = 0.0 */,
                       double zenith /* = 0.0 */,
                       double gmt_offset /* = 99999.0 */) {
  return DateTime(timestamp, false).getSunInfo
    (static_cast<DateTime::SunInfoFormat>(format), latitude, longitude,
     zenith, gmt_offset, false);
}

Variant f_date_sunset(int64_t timestamp, int format /* = 0 */,
                      double latitude /* = 0.0 */, double longitude /* = 0.0 */,
                      double zenith /* = 0.0 */,
                      double gmt_offset /* = 99999.0 */) {
  return DateTime(timestamp, false).getSunInfo
    (static_cast<DateTime::SunInfoFormat>(format), latitude, longitude,
     zenith, gmt_offset, true);
}

///////////////////////////////////////////////////////////////////////////////
}
