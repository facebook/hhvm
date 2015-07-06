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

#include "hphp/runtime/ext/datetime/ext_datetime.h"

#include "hphp/runtime/base/actrec-args.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/system/constants.h"
#include "hphp/system/systemlib.h"

namespace HPHP {

static int check_id_allowed(const String& id, long bf) {
  if (bf & q_DateTimeZone$$AFRICA && id.find("Africa/") == 0) return 1;
  if (bf & q_DateTimeZone$$AMERICA && id.find("America/") == 0) return 1;
  if (bf & q_DateTimeZone$$ANTARCTICA && id.find("Antarctica/") == 0) return 1;
  if (bf & q_DateTimeZone$$ARCTIC && id.find("Arctic/") == 0) return 1;
  if (bf & q_DateTimeZone$$ASIA && id.find("Asia/") == 0) return 1;
  if (bf & q_DateTimeZone$$ATLANTIC && id.find("Atlantic/") == 0) return 1;
  if (bf & q_DateTimeZone$$AUSTRALIA && id.find("Australia/") == 0) return 1;
  if (bf & q_DateTimeZone$$EUROPE && id.find("Europe/") == 0) return 1;
  if (bf & q_DateTimeZone$$INDIAN && id.find("Indian/") == 0) return 1;
  if (bf & q_DateTimeZone$$PACIFIC && id.find("Pacific/") == 0) return 1;
  if (bf & q_DateTimeZone$$UTC && id.find("UTC") == 0) return 1;
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

#define DEFINE_TIME_ZONE_CONSTANT(name, value)                                 \
  const int64_t q_DateTimeZone$$##name(value);                                 \
  const StaticString s_DateTimeZone$$##name(#name)                             \

DEFINE_TIME_ZONE_CONSTANT(AFRICA, 1);
DEFINE_TIME_ZONE_CONSTANT(AMERICA, 2);
DEFINE_TIME_ZONE_CONSTANT(ANTARCTICA, 4);
DEFINE_TIME_ZONE_CONSTANT(ARCTIC, 8);
DEFINE_TIME_ZONE_CONSTANT(ASIA, 16);
DEFINE_TIME_ZONE_CONSTANT(ATLANTIC, 32);
DEFINE_TIME_ZONE_CONSTANT(AUSTRALIA, 64);
DEFINE_TIME_ZONE_CONSTANT(EUROPE, 128);
DEFINE_TIME_ZONE_CONSTANT(INDIAN, 256);
DEFINE_TIME_ZONE_CONSTANT(PACIFIC, 512);
DEFINE_TIME_ZONE_CONSTANT(UTC, 1024);
DEFINE_TIME_ZONE_CONSTANT(ALL, 2047);
DEFINE_TIME_ZONE_CONSTANT(ALL_WITH_BC, 4095);
DEFINE_TIME_ZONE_CONSTANT(PER_COUNTRY, 4096);

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

Object HHVM_METHOD(DateTime, modify,
                   const String& modify) {
  DateTimeData* data = Native::data<DateTimeData>(this_);
  data->m_dt->modify(modify);
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
  Array all = TimeZone::GetNamesToCountryCodes();

  // This is the same check that PHP5 performs, no validation needed.
  // See ext/date/php_date.c lines 4496-4499
  if (what == q_DateTimeZone$$PER_COUNTRY && country.length() != 2) {
    raise_notice("A two-letter ISO 3166-1 compatible country code is expected");
    return false;
  }

  Array result = Array::Create();
  for (ArrayIter iter(all); iter; ++iter) {
    const Variant& tzid = iter.first();
    const Variant& tzcountry = iter.second();

    if (what == q_DateTimeZone$$PER_COUNTRY && equal(country, tzcountry)) {
      result.append(tzid);
    } else if (what == q_DateTimeZone$$ALL_WITH_BC ||
               check_id_allowed(tzid.toStrNR(), what)) {
      result.append(tzid);
    }
  }

  return result;
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
      return data->m_di->isInverted();
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

static Variant idateImpl(const String& format, int64_t timestamp) {
  if (format.size() != 1) {
    throw_invalid_argument("format: %s", format.data());
    return false;
  }
  auto dt = req::make<DateTime>(timestamp, false);
  int64_t ret = dt->toInteger(*format.data());
  if (ret == -1) return false;
  return ret;
}

#define GET_ARGS_AND_CALL(ar, func)                                            \
  try {                                                                        \
    return arReturn(ar, func(                                                  \
      getArgStrict<KindOfString>(ar, 0),                                       \
      getArgStrict<KindOfInt64>(ar, 1, TimeStamp::Current())));                \
  } catch (const IncoercibleArgumentException& e) {                            \
    return arReturn(ar, false);                                                \
  }                                                                            \

TypedValue* HHVM_FN(idate)(ActRec* ar) {
  GET_ARGS_AND_CALL(ar, idateImpl)
}

static Variant dateImpl(const String& format, int64_t timestamp) {
  if (format.empty()) return empty_string_variant();
  auto dt = req::make<DateTime>(timestamp, false);
  String ret = dt->toString(format, false);
  if (ret.isNull()) return false;
  return ret;
}

TypedValue* HHVM_FN(date)(ActRec* ar) {
  GET_ARGS_AND_CALL(ar, dateImpl)
}

static Variant gmdateImpl(const String& format, int64_t timestamp) {
  auto dt = req::make<DateTime>(timestamp, true);
  String ret = dt->toString(format, false);
  if (ret.isNull()) return false;
  return ret;
}

TypedValue* HHVM_FN(gmdate)(ActRec* ar) {
  GET_ARGS_AND_CALL(ar, gmdateImpl)
}

static Variant strftimeImpl(const String& format, int64_t timestamp) {
  auto dt = req::make<DateTime>(timestamp, false);
  String ret = dt->toString(format, true);
  if (ret.isNull()) return false;
  return ret;
}

TypedValue* HHVM_FN(strftime)(ActRec* ar) {
  GET_ARGS_AND_CALL(ar, strftimeImpl)
}

static String gmstrftimeImpl(const String& format, int64_t timestamp) {
  auto dt = req::make<DateTime>(timestamp, true);
  String ret = dt->toString(format, true);
  if (ret.isNull()) return false;
  return ret;
}

TypedValue* HHVM_FN(gmstrftime)(ActRec* ar) {
  GET_ARGS_AND_CALL(ar, gmstrftimeImpl)
}

static Variant strtotimeImpl(const String& input, int64_t timestamp) {
  if (input.empty()) {
    return false;
  }
  auto dt = req::make<DateTime>(timestamp);
  if (!dt->fromString(input, req::ptr<TimeZone>(), nullptr, false)) {
    return false;
  }
  bool error;
  return dt->toTimeStamp(error);
}

TypedValue* HHVM_FN(strtotime)(ActRec* ar) {
  GET_ARGS_AND_CALL(ar, strtotimeImpl);
}

#undef GET_ARGS_AND_CALL

static Array getdateImpl(int64_t timestamp) {
  auto dt = req::make<DateTime>(timestamp, false);
  return dt->toArray(DateTime::ArrayFormat::TimeMap);
}

TypedValue* HHVM_FN(getdate)(ActRec* ar) {
  try {
    int64_t timestamp = getArgStrict<KindOfInt64>(ar, 0, TimeStamp::Current());
    return arReturn(ar, getdateImpl(timestamp));
  } catch (const IncoercibleArgumentException& e) {
    return arReturn(ar, false);
  }
}

static Array localtimeImpl(int64_t timestamp, bool is_associative) {
  DateTime::ArrayFormat format =
    is_associative ? DateTime::ArrayFormat::TmMap :
                     DateTime::ArrayFormat::TmVector;

  return req::make<DateTime>(timestamp, false)->toArray(format);
}

TypedValue* HHVM_FN(localtime)(ActRec* ar) {
  try {
    int64_t timestamp = getArgStrict<KindOfInt64>(ar, 0, TimeStamp::Current());
    bool associative = getArgStrict<KindOfBoolean>(ar, 1, false);
    return arReturn(ar, localtimeImpl(timestamp, associative));
  } catch (const IncoercibleArgumentException& e) {
    return arReturn(ar, false);
  }
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
  return TimeZone::SetCurrent(name);
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

double get_date_default_latitude() {
  return s_date_globals->default_latitude;
}

double get_date_default_longitude() {
  return s_date_globals->default_longitude;
}

double get_date_default_sunset_zenith() {
  return s_date_globals->sunset_zenith;
}

double get_date_default_sunrise_zenith() {
  return s_date_globals->sunrise_zenith;
}

double get_date_default_gmt_offset() {
  req::ptr<TimeZone> tzi = TimeZone::Current();
  // just get the offset form utc time
  // set the timestamp 0 is ok
  return tzi->offset(0) / 3600;
}

Array HHVM_FUNCTION(date_sun_info,
                    int64_t timestamp,
                    double latitude,
                    double longitude) {
  auto dt = req::make<DateTime>(timestamp, false);
  return dt->getSunInfo(latitude, longitude);
}

#define GET_ARGS_AND_CALL(ar, func)                                            \
  try {                                                                        \
    return arReturn(ar, func(                                                  \
      getArgStrict<KindOfInt64>(ar, 0),                                        \
      getArgStrict<KindOfInt64>(ar, 1, 1),                                     \
      getArgStrict<KindOfDouble>(ar, 2, get_date_default_latitude()),          \
      getArgStrict<KindOfDouble>(ar, 3, get_date_default_longitude()),         \
      getArgStrict<KindOfDouble>(ar, 4, get_date_default_sunset_zenith()),     \
      getArgStrict<KindOfDouble>(ar, 5, get_date_default_gmt_offset())));      \
  } catch (const IncoercibleArgumentException& e) {                            \
    return arReturn(ar, false);                                                \
  }                                                                            \

Variant date_sunriseImpl(int64_t timestamp, int format, double latitude,
                         double longitude, double zenith, double gmt_offset) {
  return req::make<DateTime>(timestamp, false)->getSunInfo
    (static_cast<DateTime::SunInfoFormat>(format), latitude, longitude,
     zenith, gmt_offset, false);
}

TypedValue* HHVM_FN(date_sunrise)(ActRec* ar) {
  GET_ARGS_AND_CALL(ar, date_sunriseImpl)
}

Variant date_sunsetImpl(int64_t timestamp, int format, double latitude,
                        double longitude, double zenith, double gmt_offset) {
  return req::make<DateTime>(timestamp, false)->getSunInfo
    (static_cast<DateTime::SunInfoFormat>(format), latitude, longitude,
     zenith, gmt_offset, true);
}

TypedValue* HHVM_FN(date_sunset)(ActRec* ar) {
  GET_ARGS_AND_CALL(ar, date_sunsetImpl)
}

#undef GET_ARGS_AND_CALL

///////////////////////////////////////////////////////////////////////////////

#define REGISTER_TIME_ZONE_CONSTANT(name)                                      \
  Native::registerClassConstant<KindOfInt64>(                                  \
    DateTimeZoneData::s_className.get(), s_DateTimeZone$$##name.get(),         \
    q_DateTimeZone$$##name)                                                    \

static class DateTimeExtension final : public Extension {
public:
  DateTimeExtension() : Extension("date", k_PHP_VERSION.c_str()) { }

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

    REGISTER_TIME_ZONE_CONSTANT(AFRICA);
    REGISTER_TIME_ZONE_CONSTANT(AMERICA);
    REGISTER_TIME_ZONE_CONSTANT(ANTARCTICA);
    REGISTER_TIME_ZONE_CONSTANT(ARCTIC);
    REGISTER_TIME_ZONE_CONSTANT(ASIA);
    REGISTER_TIME_ZONE_CONSTANT(ATLANTIC);
    REGISTER_TIME_ZONE_CONSTANT(AUSTRALIA);
    REGISTER_TIME_ZONE_CONSTANT(EUROPE);
    REGISTER_TIME_ZONE_CONSTANT(INDIAN);
    REGISTER_TIME_ZONE_CONSTANT(PACIFIC);
    REGISTER_TIME_ZONE_CONSTANT(UTC);
    REGISTER_TIME_ZONE_CONSTANT(ALL);
    REGISTER_TIME_ZONE_CONSTANT(ALL_WITH_BC);
    REGISTER_TIME_ZONE_CONSTANT(PER_COUNTRY);

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
    HHVM_FE(date_sunrise);
    HHVM_FE(date_sunset);
    HHVM_FE(date);
    HHVM_FE(getdate);
    HHVM_FE(gettimeofday);
    HHVM_FE(gmdate);
    HHVM_FE(gmmktime);
    HHVM_FE(gmstrftime);
    HHVM_FE(idate);
    HHVM_FE(localtime);
    HHVM_FE(microtime);
    HHVM_FE(mktime);
    HHVM_FE(strftime);
    HHVM_FE(strptime);
    HHVM_FE(strtotime);
    HHVM_FE(time);
    HHVM_FE(timezone_name_from_abbr);
    HHVM_FE(timezone_version_get);

    loadSystemlib("datetime");
  }

  void threadInit() override {
    IniSetting::Bind(
      this, IniSetting::PHP_INI_ALL,
      "date.timezone",
      g_context->getDefaultTimeZone().c_str(),
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
    return f_date_default_timezone_set(value);
  }

  static std::string dateTimezoneIniGet() {
    auto ret = g_context->getTimeZone();
    if (ret.isNull()) {
      return "";
    }
    return ret.toCppString();
  }
} s_date_extension;

///////////////////////////////////////////////////////////////////////////////
}
