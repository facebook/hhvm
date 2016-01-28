/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/ext/datetime/ext_datetime.h"

#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/system/systemlib.h"

namespace HPHP {

static int check_id_allowed(const String& id, long bf) {
  if (bf & DateTimeZoneData::AFRICA && id.find("Africa/") == 0) return 1;
  if (bf & DateTimeZoneData::AMERICA && id.find("America/") == 0) return 1;
  if (bf & DateTimeZoneData::ANTARCTICA && id.find("Antarctica/") == 0)
    return 1;
  if (bf & DateTimeZoneData::ARCTIC && id.find("Arctic/") == 0) return 1;
  if (bf & DateTimeZoneData::ASIA && id.find("Asia/") == 0) return 1;
  if (bf & DateTimeZoneData::ATLANTIC && id.find("Atlantic/") == 0) return 1;
  if (bf & DateTimeZoneData::AUSTRALIA && id.find("Australia/") == 0) return 1;
  if (bf & DateTimeZoneData::EUROPE && id.find("Europe/") == 0) return 1;
  if (bf & DateTimeZoneData::INDIAN && id.find("Indian/") == 0) return 1;
  if (bf & DateTimeZoneData::PACIFIC && id.find("Pacific/") == 0) return 1;
  if (bf & DateTimeZoneData::UTC && id.find("UTC") == 0) return 1;
  return 0;
}

const StaticString
  s_formatOffset("P"),
  s_formatID("e"),
  s_formatAbbr("T");

static String zone_type_to_string(int zoneType, req::ptr<DateTime> dt) {
  switch (zoneType) {
    case TIMELIB_ZONETYPE_OFFSET:
      return dt->toString(s_formatOffset);
    case TIMELIB_ZONETYPE_ID:
      return dt->toString(s_formatID);
    case TIMELIB_ZONETYPE_ABBR:
      return dt->toString(s_formatAbbr);
  }

  always_assert(!"Bad zone type");
}

struct DateGlobals {
  double default_latitude;
  double default_longitude;
  double sunset_zenith;
  double sunrise_zenith;
};
IMPLEMENT_THREAD_LOCAL(DateGlobals, s_date_globals);

#define IMPLEMENT_GET_CLASS(cls)                                               \
  Class* cls::getClass() {                                                     \
    if (s_class == nullptr) {                                                  \
      s_class = Unit::lookupClass(s_className.get());                          \
      assert(s_class);                                                         \
    }                                                                          \
    return s_class;                                                            \
  }                                                                            \

///////////////////////////////////////////////////////////////////////////////
// constants

const StaticString s_data("data");
const StaticString s_getTimestamp("getTimestamp");

///////////////////////////////////////////////////////////////////////////////
// DateTime

Class* DateTimeData::s_class = nullptr;
const StaticString DateTimeData::s_className("DateTime");

Object HHVM_METHOD(DateTime, add,
                   const Object& interval) {
  DateTimeData* data = Native::data<DateTimeData>(this_);
  data->m_dt->add(DateIntervalData::unwrap(interval));
  return Object(this_);
}

void HHVM_METHOD(DateTime, __construct,
                 const String& time /*= "now"*/,
                 const Variant& timezone /*= null_variant*/) {
  const Object& obj_timezone = timezone.isNull()
                             ? null_object
                             : timezone.toObject();
  DateTimeData* data = Native::data<DateTimeData>(this_);
  data->m_dt = req::make<DateTime>(TimeStamp::Current(),
                                      DateTimeZoneData::unwrap(obj_timezone));
  if (!time.empty()) {
    data->m_dt->fromString(time, DateTimeZoneData::unwrap(obj_timezone));
  } else if (!timezone.isNull()) {
    // We still have to tell the underlying DateTime the timezone incase they
    // call setTimestamp or something else later
    data->m_dt->setTimezone(DateTimeZoneData::unwrap(obj_timezone));
  }
}

Variant HHVM_STATIC_METHOD(DateTime, createFromFormat,
                           const String& format,
                           const String& time,
                           const Variant& timezone /*= null_variant */) {
  const Object& obj_timezone = timezone.isNull()
                             ? null_object
                             : timezone.toObject();
  Object obj{DateTimeData::getClass()};
  DateTimeData* data = Native::data<DateTimeData>(obj);
  const auto curr = (format.find("!") != String::npos) ? 0 : f_time() ;
  data->m_dt = req::make<DateTime>(curr, false);
  if (!data->m_dt->fromString(time, DateTimeZoneData::unwrap(obj_timezone),
                              format.data(), false)) {
    return false;
  }

  return obj;
}

static const StaticString s_DateTimeInterface("DateTimeInterface");

Object HHVM_METHOD(DateTime, diff,
                   const Variant& datetime2,
                   const Variant& absolute) {
  DateTimeData* data = Native::data<DateTimeData>(this_);
  const Object obj_datetime2 = datetime2.toObject();
  if (!obj_datetime2.instanceof(s_DateTimeInterface)) {
    throw_invalid_object_type(obj_datetime2->getClassName().data());
  }
  return DateIntervalData::wrap(data->m_dt->diff(
                                DateTimeData::unwrap(obj_datetime2),
                                absolute.toBoolean()));
}

String HHVM_METHOD(DateTime, format,
                   const Variant& format) {
  DateTimeData* data = Native::data<DateTimeData>(this_);
  return data->format(format.toString());
}

static const StaticString s_warning_count("warning_count");
static const StaticString s_warnings("warnings");
static const StaticString s_error_count("error_count");
static const StaticString s_errors("errors");

Array HHVM_STATIC_METHOD(DateTime, getLastErrors) {
  Array errors = DateTime::getLastErrors();
  Array warnings = DateTime::getLastWarnings();
  Array ret = Array::Create();

  ret.add(s_warning_count, warnings.size());
  ret.add(s_warnings, warnings);
  ret.add(s_error_count, errors.size());
  ret.add(s_errors, errors);

  return ret;
}

int64_t HHVM_METHOD(DateTime, getOffset) {
  DateTimeData* data = Native::data<DateTimeData>(this_);
  return data->m_dt->offset();
}

int64_t HHVM_METHOD(DateTime, getTimestamp) {
  DateTimeData* data = Native::data<DateTimeData>(this_);
  return data->getTimestamp();
}

Variant HHVM_METHOD(DateTime, getTimezone) {
  DateTimeData* data = Native::data<DateTimeData>(this_);
  req::ptr<TimeZone> tz = data->m_dt->timezone();
  if (tz->isValid()) {
    return DateTimeZoneData::wrap(tz);
  }
  return false;
}

Variant HHVM_METHOD(DateTime, modify,
                   const String& modify) {
  DateTimeData* data = Native::data<DateTimeData>(this_);
  if (!data->m_dt->modify(modify)) {
    return false;
  }
  return Object(this_);
}

Object HHVM_METHOD(DateTime, setDate,
                   int64_t year,
                   int64_t month,
                   int64_t day) {
  DateTimeData* data = Native::data<DateTimeData>(this_);
  data->m_dt->setDate(year, month, day);
  return Object(this_);
}

Object HHVM_METHOD(DateTime, setISODate,
                   int64_t year,
                   int64_t week,
                   int64_t day /*= 1*/) {
  DateTimeData* data = Native::data<DateTimeData>(this_);
  data->m_dt->setISODate(year, week, day);
  return Object(this_);
}

Object HHVM_METHOD(DateTime, setTime,
                   int64_t hour,
                   int64_t minute,
                   int64_t second /*= 0*/) {
  DateTimeData* data = Native::data<DateTimeData>(this_);
  data->m_dt->setTime(hour, minute, second);
  return Object(this_);
}

Object HHVM_METHOD(DateTime, setTimestamp,
                   int64_t unixtimestamp) {
  DateTimeData* data = Native::data<DateTimeData>(this_);
  data->m_dt->fromTimeStamp(unixtimestamp, false);
  return Object(this_);
}

Object HHVM_METHOD(DateTime, setTimezone,
                   const Object& timezone) {
  DateTimeData* data = Native::data<DateTimeData>(this_);
  data->m_dt->setTimezone(DateTimeZoneData::unwrap(timezone));
  return Object(this_);
}

Object HHVM_METHOD(DateTime, sub,
                   const Object& interval) {
  DateTimeData* data = Native::data<DateTimeData>(this_);
  data->m_dt->sub(DateIntervalData::unwrap(interval));
  return Object(this_);
}

const StaticString
  s_date("date"),
  s_timezone_type("timezone_type"),
  s_timezone("timezone"),
  s_ISOformat("Y-m-d H:i:s.u");

Array HHVM_METHOD(DateTime, __sleep) {
  DateTimeData* data = Native::data<DateTimeData>(this_);
  int zoneType = data->m_dt->zoneType();

  this_->o_set(s_date, data->format(s_ISOformat));
  this_->o_set(s_timezone_type, zoneType);
  this_->o_set(s_timezone, zone_type_to_string(zoneType, data->m_dt));
  return make_packed_array(s_date, s_timezone_type, s_timezone);
}

void HHVM_METHOD(DateTime, __wakeup) {
  Object dtz_obj{DateTimeZoneData::getClass()};
  HHVM_MN(DateTimeZone, __construct)(dtz_obj.get(),
                                     this_->o_get(s_timezone).toString());
  HHVM_MN(DateTime, __construct)(this_, this_->o_get(s_date).toString(),
                                 std::move(dtz_obj));

  // cleanup
  Class* cls = this_->getVMClass();
  this_->unsetProp(cls, s_date.get());
  this_->unsetProp(cls, s_timezone_type.get());
  this_->unsetProp(cls, s_timezone.get());
}

Array HHVM_METHOD(DateTime, __debuginfo) {
  DateTimeData* data = Native::data<DateTimeData>(this_);
  return data->getDebugInfo();
}

Array DateTimeData::getDebugInfo() const {
  return make_map_array(
    s_date, format(s_ISOformat),
    s_timezone_type, m_dt->zoneType(),
    s_timezone, zone_type_to_string(m_dt->zoneType(), m_dt)
  );
}

///////////////////////////////////////////////////////////////////////////////
// DateTime helpers

int64_t DateTimeData::getTimestamp(const Object& obj) {
  if (LIKELY(obj.instanceof(getClass()))) {
    return Native::data<DateTimeData>(obj)->getTimestamp();
  }
  assert(obj->instanceof(SystemLib::s_DateTimeInterfaceClass));
  Variant result = obj->o_invoke(s_getTimestamp, Array::Create());
  return result.toInt64();
}

int64_t DateTimeData::getTimestamp(const ObjectData* od) {
  return getTimestamp(Object(const_cast<ObjectData*>(od)));
}

Object DateTimeData::wrap(req::ptr<DateTime> dt) {
  Object obj{getClass()};
  DateTimeData* data = Native::data<DateTimeData>(obj);
  data->m_dt = dt;
  return obj;
}

req::ptr<DateTime> DateTimeData::unwrap(const Object& datetime) {
  if (LIKELY(datetime.instanceof(getClass()))) {
    DateTimeData* data = Native::data<DateTimeData>(datetime);
    return data->m_dt;
  }
  if (datetime->instanceof(SystemLib::s_DateTimeImmutableClass)) {
    auto lookup = datetime->getProp(
      SystemLib::s_DateTimeImmutableClass,
      s_data.get()
    );
    auto tv = lookup.prop;
    assert(tv->m_type == KindOfObject);
    Object impl(tv->m_data.pobj);
    return unwrap(impl);
  }
  return req::ptr<DateTime>();
}

IMPLEMENT_GET_CLASS(DateTimeData)

///////////////////////////////////////////////////////////////////////////////
// DateTimeZone

Class* DateTimeZoneData::s_class = nullptr;
const StaticString DateTimeZoneData::s_className("DateTimeZone");

void HHVM_METHOD(DateTimeZone, __construct,
                 const String& timezone) {
  DateTimeZoneData* data = Native::data<DateTimeZoneData>(this_);
  data->m_tz = req::make<TimeZone>(timezone);
  if (!data->m_tz->isValid()) {
    std::string msg = "DateTimeZone::__construct(): Unknown or bad timezone (";
    msg += timezone.data();
    msg += ")";
    SystemLib::throwExceptionObject(msg);
  }
}

Array HHVM_METHOD(DateTimeZone, getLocation) {
  DateTimeZoneData* data = Native::data<DateTimeZoneData>(this_);
  return data->m_tz->getLocation();
}

String HHVM_METHOD(DateTimeZone, getName) {
  DateTimeZoneData* data = Native::data<DateTimeZoneData>(this_);
  return data->getName();
}

int64_t HHVM_METHOD(DateTimeZone, getOffset,
                    const Object& datetime) {
  DateTimeZoneData* data = Native::data<DateTimeZoneData>(this_);
  bool error;
  int64_t ts = DateTimeData::unwrap(datetime)->toTimeStamp(error);
  return data->m_tz->offset(ts);
}

Array HHVM_METHOD(DateTimeZone, getTransitions,
                  int64_t timestamp_begin, /*=k_PHP_INT_MIN*/
                  int64_t timestamp_end /*=k_PHP_INT_MAX*/) {
  DateTimeZoneData* data = Native::data<DateTimeZoneData>(this_);
  return data->m_tz->transitions(timestamp_begin, timestamp_end);
}

Array HHVM_STATIC_METHOD(DateTimeZone, listAbbreviations) {
  return TimeZone::GetAbbreviations();
}

Variant HHVM_STATIC_METHOD(DateTimeZone, listIdentifiers,
                           int64_t what,
                           const String& country) {
  // This is the same check that PHP5 performs, no validation needed.
  // See ext/date/php_date.c lines 4496-4499
  if (what == DateTimeZoneData::PER_COUNTRY && country.length() != 2) {
    raise_notice("A two-letter ISO 3166-1 compatible country code is expected");
    return false;
  }

  const timelib_tzdb *tzdb = timezone_get_builtin_tzdb();
  int item_count = tzdb->index_size;
  const timelib_tzdb_index_entry *table = tzdb->index;

  Array ret = Array::Create();
  for (int i = 0; i < item_count; ++i) {
    // This string is what PHP considers as "data" or "info" which is basically
    // the string of "PHP1xx" where xx is country code that uses this timezone.
    // When country code is unknown or not in use anymore, ?? is used instead.
    // There is no known better way to extract this information out.
    const char* infoString = (const char*)&tzdb->data[table[i].pos];
    String countryCode = String(&infoString[5], 2, CopyString);
    if ((what == DateTimeZoneData::PER_COUNTRY && equal(country, countryCode))
        || what == DateTimeZoneData::ALL_WITH_BC
        || (check_id_allowed(table[i].id, what)
            && tzdb->data[table[i].pos + 4] == '\1')) {

      ret.append(String(table[i].id, CopyString));
    }
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// DateTimeZone helpers

Object DateTimeZoneData::wrap(req::ptr<TimeZone> tz) {
  Object obj{getClass()};
  DateTimeZoneData* data = Native::data<DateTimeZoneData>(obj);
  data->m_tz = tz;
  return obj;
}

req::ptr<TimeZone> DateTimeZoneData::unwrap(const Object& timezone) {
  if (timezone.instanceof(getClass())) {
    DateTimeZoneData* data = Native::data<DateTimeZoneData>(timezone);
    return data->m_tz;
  }
  return req::ptr<TimeZone>();
}

IMPLEMENT_GET_CLASS(DateTimeZoneData)

///////////////////////////////////////////////////////////////////////////////
// DateInterval

Class* DateIntervalData::s_class = nullptr;
const StaticString DateIntervalData::s_className("DateInterval");

void HHVM_METHOD(DateInterval, __construct,
                 const String& interval_spec) {
  DateIntervalData* data = Native::data<DateIntervalData>(this_);
  data->m_di = req::make<DateInterval>(interval_spec);
  if (!data->m_di->isValid()) {
    std::string msg = "DateInterval::__construct: Invalid interval (";
    msg += interval_spec.data();
    msg += ")";
    SystemLib::throwExceptionObject(msg);
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

Variant HHVM_METHOD(DateInterval, __get,
                    const Variant& member) {
  DateIntervalData* data = Native::data<DateIntervalData>(this_);
  if (member.isString()) {
    if (same(member, s_y)) {
      return data->m_di->getYears();
    }
    if (same(member, s_m)) {
      return data->m_di->getMonths();
    }
    if (same(member, s_d)) {
      return data->m_di->getDays();
    }
    if (same(member, s_h)) {
      return data->m_di->getHours();
    }
    if (same(member, s_i)) {
      return data->m_di->getMinutes();
    }
    if (same(member, s_s)) {
      return data->m_di->getSeconds();
    }
    if (same(member, s_invert)) {
      return data->m_di->isInverted() ? 1 : 0;
    }
    if (same(member, s_days)) {
      if (data->m_di->haveTotalDays()) {
        return data->m_di->getTotalDays();
      } else {
        return false;
      }
    }
  }
  std::string msg = "Undefined property '";
  msg += member.toString().data();
  msg += ") on DateInterval object";
  SystemLib::throwExceptionObject(msg);
}

Variant HHVM_METHOD(DateInterval, __set,
                    const Variant& member,
                    const Variant& value) {
  DateIntervalData* data = Native::data<DateIntervalData>(this_);
  if (member.isString()) {
    if (same(member, s_y)) {
      data->m_di->setYears(value.toInt64());
      return init_null();
    }
    if (same(member, s_m)) {
      data->m_di->setMonths(value.toInt64());
      return init_null();
    }
    if (same(member, s_d)) {
      data->m_di->setDays(value.toInt64());
      return init_null();
    }
    if (same(member, s_h)) {
      data->m_di->setHours(value.toInt64());
      return init_null();
    }
    if (same(member, s_i)) {
      data->m_di->setMinutes(value.toInt64());
      return init_null();
    }
    if (same(member, s_s)) {
      data->m_di->setSeconds(value.toInt64());
      return init_null();
    }
    if (same(member, s_invert)) {
      data->m_di->setInverted(value.toBoolean());
      return init_null();
    }
    if (same(member, s_days)) {
      data->m_di->setTotalDays(value.toInt64());
      return init_null();
    }
  }

  std::string msg = "Undefined property '";
  msg += member.toString().data();
  msg += ") on DateInterval object";
  SystemLib::throwExceptionObject(msg);
}

Object HHVM_STATIC_METHOD(DateInterval, createFromDateString,
                          const String& time) {
  return DateIntervalData::wrap(req::make<DateInterval>(time, true));
}

String HHVM_METHOD(DateInterval, format,
                   const String& format) {
  DateIntervalData* data = Native::data<DateIntervalData>(this_);
  return data->m_di->format(format);
}

///////////////////////////////////////////////////////////////////////////////
// DateInterval helpers

Object DateIntervalData::wrap(req::ptr<DateInterval> di) {
  Object obj{getClass()};
  DateIntervalData* data = Native::data<DateIntervalData>(obj);
  data->m_di = di;
  return obj;
}

req::ptr<DateInterval> DateIntervalData::unwrap(const Object& obj) {
  if (obj.instanceof(getClass())) {
    DateIntervalData* data = Native::data<DateIntervalData>(obj);
    return data->m_di;
  }

  return req::ptr<DateInterval>();
}

IMPLEMENT_GET_CLASS(DateIntervalData)

///////////////////////////////////////////////////////////////////////////////
// timestamp

Variant HHVM_FUNCTION(gettimeofday,
                      bool return_float /* = false */) {
  if (return_float) {
    return TimeStamp::CurrentSecond();
  }
  return TimeStamp::CurrentTime();
}

Variant HHVM_FUNCTION(microtime,
                      bool get_as_float /* = false */) {
  if (get_as_float) {
    return TimeStamp::CurrentSecond();
  }
  return TimeStamp::CurrentMicroTime();
}

int64_t HHVM_FUNCTION(time) {
  return time(0);
}

Variant HHVM_FUNCTION(mktime,
                      int64_t hour /* = PHP_INT_MAX */,
                      int64_t minute /* = PHP_INT_MAX */,
                      int64_t second /* = PHP_INT_MAX */,
                      int64_t month /* = PHP_INT_MAX */,
                      int64_t day /* = PHP_INT_MAX */,
                      int64_t year /* = PHP_INT_MAX */) {
  hour = hour < INT_MAX ? hour : INT_MAX;
  minute = minute < INT_MAX ? minute : INT_MAX;
  second = second < INT_MAX ? second : INT_MAX;
  month = month < INT_MAX ? month : INT_MAX;
  day = day < INT_MAX ? day : INT_MAX;
  year = year < INT_MAX ? year : INT_MAX;
  if (hour == INT_MAX && minute == INT_MAX && second == INT_MAX &&
      month == INT_MAX && day == INT_MAX && year == INT_MAX) {
    raise_strict_warning("mktime(): You should be using "
                         "the time() function instead");
  }
  bool error;
  int64_t ts = TimeStamp::Get(error, hour, minute, second, month, day, year,
                              false);
  if (error) return false;
  return ts;
}

Variant HHVM_FUNCTION(gmmktime,
                      int64_t hour /* = PHP_INT_MAX */,
                      int64_t minute /* = PHP_INT_MAX */,
                      int64_t second /* = PHP_INT_MAX */,
                      int64_t month /* = PHP_INT_MAX */,
                      int64_t day /* = PHP_INT_MAX */,
                      int64_t year /* = PHP_INT_MAX */) {
  hour = hour < INT_MAX ? hour : INT_MAX;
  minute = minute < INT_MAX ? minute : INT_MAX;
  second = second < INT_MAX ? second : INT_MAX;
  month = month < INT_MAX ? month : INT_MAX;
  day = day < INT_MAX ? day : INT_MAX;
  year = year < INT_MAX ? year : INT_MAX;
  bool error;
  int64_t ts = TimeStamp::Get(error, hour, minute, second, month, day, year,
                              true);
  if (error) return false;
  return ts;
}

static Variant HHVM_FUNCTION(idate, int64_t argc,
                             const String& fmt, int64_t timestamp) {
  if (fmt.size() != 1) {
    throw_invalid_argument("format: %s", fmt.data());
    return false;
  }
  if (argc < 2) {
    timestamp = TimeStamp::Current();
  }
  int64_t ret = req::make<DateTime>(timestamp, false)->toInteger(*fmt.data());
  if (ret == -1) return false;
  return ret;
}

template<bool gmt>
static Variant date_impl(int64_t argc,
                         const String& format, int64_t timestamp) {
  if (!gmt && format.empty()) return empty_string_variant();
  if (argc < 2) timestamp = TimeStamp::Current();
  String ret = req::make<DateTime>(timestamp, gmt)->toString(format, false);
  if (ret.isNull()) return false;
  return ret;
}

template<bool gmt>
static Variant strftime_impl(int64_t argc,
                             const String& format, int64_t timestamp) {
  if (argc < 2) timestamp = TimeStamp::Current();
  String ret = req::make<DateTime>(timestamp, gmt)->toString(format, true);
  if (ret.isNull()) return false;
  return ret;
}

static Variant HHVM_FUNCTION(strtotime, int64_t argc,
                             const String& input, int64_t timestamp) {
  if (input.empty()) {
    return false;
  }
  if (argc < 2) {
    timestamp = TimeStamp::Current();
  }
  auto dt = req::make<DateTime>(timestamp);
  if (!dt->fromString(input, req::ptr<TimeZone>(), nullptr, false)) {
    return false;
  }
  bool error;
  return dt->toTimeStamp(error);
}

static Array HHVM_FUNCTION(getdate, int64_t argc, int64_t timestamp) {
  if (argc < 1) {
    timestamp = TimeStamp::Current();
  }
  return req::make<DateTime>(timestamp, false)->
           toArray(DateTime::ArrayFormat::TimeMap);
}

static Array HHVM_FUNCTION(localtime, int64_t argc,
                           int64_t timestamp, bool is_assoc) {
  if (argc < 1) {
    timestamp = TimeStamp::Current();
  }
  auto format = is_assoc ? DateTime::ArrayFormat::TmMap
                         : DateTime::ArrayFormat::TmVector;
  return req::make<DateTime>(timestamp, false)->toArray(format);
}

Variant HHVM_FUNCTION(strptime,
                      const String& date,
                      const String& format) {
  Array ret = DateTime::ParseAsStrptime(format, date);
  if (ret.empty()) {
    return false;
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// timezone

String HHVM_FUNCTION(date_default_timezone_get) {
  return TimeZone::Current()->name();
}

bool HHVM_FUNCTION(date_default_timezone_set,
                   const String& name) {
  return TimeZone::SetCurrent(name.c_str());
}

Variant HHVM_FUNCTION(timezone_name_from_abbr,
                      const String& abbr,
                      int gmtoffset /* = -1 */,
                      int isdst /* = 1 */) {
  String ret = TimeZone::AbbreviationToName(abbr, gmtoffset, isdst);
  if (ret.isNull()) {
    return false;
  }
  return ret;
}

String HHVM_FUNCTION(timezone_version_get) {
  return TimeZone::getVersion();
}

///////////////////////////////////////////////////////////////////////////////
// datetime

bool HHVM_FUNCTION(checkdate,
                   int month,
                   int day,
                   int year) {
  return DateTime::IsValid(year, month, day);
}

Variant HHVM_FUNCTION(date_parse_from_format,
                      const String& format,
                      const String& date) {
  Array ret = DateTime::Parse(format, date);
  if (ret.empty()) {
    return false;
  }
  return ret;
}

Variant HHVM_FUNCTION(date_create,
                      const Variant& time /* = null_string */,
                      const Variant& timezone /* = null_variant */) {
  const String& str_time = time.isNull() ? null_string : time.toString();
  const Object& obj_timezone = timezone.isNull()
                             ? null_object
                             : timezone.toObject();
  Object ret{DateTimeData::getClass()};
  // Don't set the time here because it will throw if it is bad
  HHVM_MN(DateTime, __construct)(ret.get());
  if (str_time.empty()) {
    // zend does this, so so do we
    return ret;
  }
  auto dt = DateTimeData::unwrap(ret);
  auto tz = DateTimeZoneData::unwrap(obj_timezone);
  if (!dt->fromString(str_time, tz, nullptr, false)) {
    return false;
  }
  return ret;
}

String HHVM_FUNCTION(date_format,
                     const Object& datetime,
                     const String& format) {
  return DateTimeData::unwrap(datetime)->toString(format, false);
}

Variant HHVM_FUNCTION(date_parse,
                      const String& date) {
  Array ret = DateTime::Parse(date);
  if (ret.empty()) {
    return false;
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// sun

Array HHVM_FUNCTION(date_sun_info,
                    int64_t timestamp,
                    double latitude,
                    double longitude) {
  auto dt = req::make<DateTime>(timestamp, false);
  return dt->getSunInfo(latitude, longitude);
}

template<bool sunset>
Variant date_sunrise_sunset(int64_t numArgs,
                            int64_t timestamp, int64_t format,
                            double latitude, double longitude,
                            double zenith, double offset) {
  /* Fill in dynamic args (3..6) as needed */
  switch (numArgs) {
    case 0: case 1: /* fallthrough */
    case 2: latitude  = s_date_globals->default_latitude;
    case 3: longitude = s_date_globals->default_longitude;
    case 4: zenith = sunset ? s_date_globals->sunset_zenith
                            : s_date_globals->sunrise_zenith;
    case 5: offset = TimeZone::Current()->offset(0) / 3600;
  }
  return req::make<DateTime>(timestamp, false)->getSunInfo
    (static_cast<DateTime::SunInfoFormat>(format), latitude, longitude,
     zenith, offset, sunset);
}

///////////////////////////////////////////////////////////////////////////////

static const StaticString
  s_DateTimeZone("DateTimeZone"),
  s_DateTime("DateTime");

#define DATE_ATOM "Y-m-d\\TH:i:sP"
#define DATE_COOKIE "l, d-M-y H:i:s T"
#define DATE_ISO8601 "Y-m-d\\TH:i:sO"
#define DATE_RFC822 "D, d M y H:i:s O"
#define DATE_RFC850 "l, d-M-y H:i:s T"
#define DATE_RFC1036 "D, d M y H:i:s O"
#define DATE_RFC1123 "D, d M Y H:i:s O"
#define DATE_RFC2822 "D, d M Y H:i:s O"
#define DATE_RFC3339 "Y-m-d\\TH:i:sP"
#define DATE_RSS "D, d M Y H:i:s O"
#define DATE_W3C "Y-m-d\\TH:i:sP"

static class DateTimeExtension final : public Extension {
public:
  DateTimeExtension() : Extension("date", get_PHP_VERSION().c_str()) { }

  void moduleInit() override {
    HHVM_ME(DateTime, __construct);
    HHVM_ME(DateTime, add);
    HHVM_ME(DateTime, diff);
    HHVM_ME(DateTime, format);
    HHVM_ME(DateTime, getOffset);
    HHVM_ME(DateTime, getTimestamp);
    HHVM_ME(DateTime, getTimezone);
    HHVM_ME(DateTime, modify);
    HHVM_ME(DateTime, setDate);
    HHVM_ME(DateTime, setISODate);
    HHVM_ME(DateTime, setTime);
    HHVM_ME(DateTime, setTimestamp);
    HHVM_ME(DateTime, setTimezone);
    HHVM_ME(DateTime, sub);
    HHVM_ME(DateTime, __sleep);
    HHVM_ME(DateTime, __wakeup);
    HHVM_ME(DateTime, __debuginfo);
    HHVM_STATIC_ME(DateTime, createFromFormat);
    HHVM_STATIC_ME(DateTime, getLastErrors);

    Native::registerNativeDataInfo<DateTimeData>(
      DateTimeData::s_className.get(), Native::NDIFlags::NO_SWEEP);

    HHVM_RC_STR_SAME(DATE_ATOM);
    HHVM_RCC_STR(DateTime, ATOM, DATE_ATOM);
    HHVM_RC_STR_SAME(DATE_COOKIE);
    HHVM_RCC_STR(DateTime, COOKIE, DATE_COOKIE);
    HHVM_RC_STR_SAME(DATE_ISO8601);
    HHVM_RCC_STR(DateTime, ISO8601, DATE_ISO8601);
    HHVM_RC_STR_SAME(DATE_RFC822);
    HHVM_RCC_STR(DateTime, RFC822, DATE_RFC822);
    HHVM_RC_STR_SAME(DATE_RFC850);
    HHVM_RCC_STR(DateTime, RFC850, DATE_RFC850);
    HHVM_RC_STR_SAME(DATE_RFC1036);
    HHVM_RCC_STR(DateTime, RFC1036, DATE_RFC1036);
    HHVM_RC_STR_SAME(DATE_RFC1123);
    HHVM_RCC_STR(DateTime, RFC1123, DATE_RFC1123);
    HHVM_RC_STR_SAME(DATE_RFC2822);
    HHVM_RCC_STR(DateTime, RFC2822, DATE_RFC2822);
    HHVM_RC_STR_SAME(DATE_RFC3339);
    HHVM_RCC_STR(DateTime, RFC3339, DATE_RFC3339);
    HHVM_RC_STR_SAME(DATE_RSS);
    HHVM_RCC_STR(DateTime, RSS, DATE_RSS);
    HHVM_RC_STR_SAME(DATE_W3C);
    HHVM_RCC_STR(DateTime, W3C, DATE_W3C);

    HHVM_RCC_INT(DateTimeZone, AFRICA, DateTimeZoneData::AFRICA);
    HHVM_RCC_INT(DateTimeZone, AMERICA, DateTimeZoneData::AMERICA);
    HHVM_RCC_INT(DateTimeZone, ANTARCTICA, DateTimeZoneData::ANTARCTICA);
    HHVM_RCC_INT(DateTimeZone, ARCTIC, DateTimeZoneData::ARCTIC);
    HHVM_RCC_INT(DateTimeZone, ASIA, DateTimeZoneData::ASIA);
    HHVM_RCC_INT(DateTimeZone, ATLANTIC, DateTimeZoneData::ATLANTIC);
    HHVM_RCC_INT(DateTimeZone, AUSTRALIA, DateTimeZoneData::AUSTRALIA);
    HHVM_RCC_INT(DateTimeZone, EUROPE, DateTimeZoneData::EUROPE);
    HHVM_RCC_INT(DateTimeZone, INDIAN, DateTimeZoneData::INDIAN);
    HHVM_RCC_INT(DateTimeZone, PACIFIC, DateTimeZoneData::PACIFIC);
    HHVM_RCC_INT(DateTimeZone, UTC, DateTimeZoneData::UTC);
    HHVM_RCC_INT(DateTimeZone, ALL, DateTimeZoneData::ALL);
    HHVM_RCC_INT(DateTimeZone, ALL_WITH_BC, DateTimeZoneData::ALL_WITH_BC);
    HHVM_RCC_INT(DateTimeZone, PER_COUNTRY, DateTimeZoneData::PER_COUNTRY);

    HHVM_ME(DateTimeZone, __construct);
    HHVM_ME(DateTimeZone, getLocation);
    HHVM_ME(DateTimeZone, getName);
    HHVM_ME(DateTimeZone, getOffset);
    HHVM_ME(DateTimeZone, getTransitions);
    HHVM_STATIC_ME(DateTimeZone, listAbbreviations);
    HHVM_STATIC_ME(DateTimeZone, listIdentifiers);

    Native::registerNativeDataInfo<DateTimeZoneData>(
      DateTimeZoneData::s_className.get(), Native::NDIFlags::NO_SWEEP);

    HHVM_ME(DateInterval, __construct);
    HHVM_ME(DateInterval, __get);
    HHVM_ME(DateInterval, __set);
    HHVM_ME(DateInterval, format);
    HHVM_STATIC_ME(DateInterval, createFromDateString);

    Native::registerNativeDataInfo<DateIntervalData>(
      DateIntervalData::s_className.get(), Native::NDIFlags::NO_SWEEP);

    HHVM_FE(checkdate);
    HHVM_FE(date_parse_from_format);
    HHVM_FE(date_create);
    HHVM_FE(date_default_timezone_get);
    HHVM_FE(date_default_timezone_set);
    HHVM_FE(date_format);
    HHVM_FE(date_parse);
    HHVM_FE(date_sun_info);
    HHVM_NAMED_FE(date_sunrise, date_sunrise_sunset<false>);
    HHVM_NAMED_FE(date_sunset, date_sunrise_sunset<true>);
    HHVM_NAMED_FE(date, date_impl<false>);
    HHVM_NAMED_FE(gmdate, date_impl<true>);
    HHVM_FE(getdate);
    HHVM_FE(gettimeofday);
    HHVM_FE(gmmktime);
    HHVM_NAMED_FE(strftime, strftime_impl<false>);
    HHVM_NAMED_FE(gmstrftime, strftime_impl<true>);
    HHVM_FE(idate);
    HHVM_FE(localtime);
    HHVM_FE(microtime);
    HHVM_FE(mktime);
    HHVM_FE(strptime);
    HHVM_FE(strtotime);
    HHVM_FE(time);
    HHVM_FE(timezone_name_from_abbr);
    HHVM_FE(timezone_version_get);

    HHVM_RC_INT(SUNFUNCS_RET_DOUBLE, DateTime::SunInfoFormat::ReturnDouble);
    HHVM_RC_INT(SUNFUNCS_RET_STRING, DateTime::SunInfoFormat::ReturnString);
    HHVM_RC_INT(SUNFUNCS_RET_TIMESTAMP,
                DateTime::SunInfoFormat::ReturnTimeStamp);

    loadSystemlib("datetime");
  }

  void threadInit() override {
    IniSetting::Bind(
      this, IniSetting::PHP_INI_ALL,
      "date.timezone",
      "",
      IniSetting::SetAndGet<std::string>(
        dateTimezoneIniUpdate, dateTimezoneIniGet
      )
    );
    IniSetting::Bind(
      this, IniSetting::PHP_INI_ALL,
      "date.default_latitude", "31.7667",
      &s_date_globals->default_latitude
    );
    IniSetting::Bind(
      this, IniSetting::PHP_INI_ALL,
      "date.default_longitude", "35.2333",
      &s_date_globals->default_longitude
    );
    IniSetting::Bind(
      this, IniSetting::PHP_INI_ALL,
      "date.sunset_zenith", "90.583333",
      &s_date_globals->sunset_zenith
    );
    IniSetting::Bind(
      this, IniSetting::PHP_INI_ALL,
      "date.sunrise_zenith", "90.583333",
      &s_date_globals->sunrise_zenith
    );
  }

private:
  static bool dateTimezoneIniUpdate(const std::string& value) {
    if (value.empty()) {
      return false;
    }
    return TimeZone::SetCurrent(value.c_str());
  }

  static std::string dateTimezoneIniGet() {
    return RID().getTimeZone();
  }
} s_date_extension;

///////////////////////////////////////////////////////////////////////////////
}
