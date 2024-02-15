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

#include "hphp/runtime/ext/datetime/ext_datetime.h"

#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/runtime/vm/native-prop-handler.h"
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
RDS_LOCAL(DateGlobals, s_date_globals);

///////////////////////////////////////////////////////////////////////////////
// constants

static const StaticString
  s_data("data"),
  s_getTimestamp("getTimestamp"),
  s_DateTimeInterface("DateTimeInterface"),
  s_DateTimeZone("DateTimeZone"),
  s_DateInterval("DateInterval"),
  s_DateTime("DateTime");

///////////////////////////////////////////////////////////////////////////////

namespace {

void raise_argument_warning(const char* func,
                            int param,
                            const StaticString& expected,
                            const Object& given) {
  raise_warning("%s() expects parameter %d to be %s, %s given",
                func, param, expected.c_str(), given.get()->classname_cstr());
}

DateTimeData* getDateTimeData(ObjectData* this_) {
  auto const data = Native::data<DateTimeData>(this_);
  if (data->m_dt) return data;
  SystemLib::throwInvalidArgumentExceptionObject(
    "Use before DateTime::__construct() called"
  );
}

DateTimeData* getDateTimeData(const Object& this_) {
  return getDateTimeData(this_.get());
}

DateTimeZoneData* getDateTimeZoneData(ObjectData* this_) {
  auto const data = Native::data<DateTimeZoneData>(this_);
  if (data->m_tz) return data;
  SystemLib::throwInvalidArgumentExceptionObject(
    "Use before DateTimeZone::__construct() called"
  );
}

DateIntervalData* getDateIntervalData(ObjectData* this_) {
  auto const data = Native::data<DateIntervalData>(this_);
  if (data->m_di) return data;
  SystemLib::throwInvalidArgumentExceptionObject(
    "Use before DateInterval::__construct() called"
  );
}

}

///////////////////////////////////////////////////////////////////////////////
// DateTime

void HHVM_METHOD(DateTime, __construct,
                 const String& time /*= "now"*/,
                 const Variant& timezone /*= uninit_variant*/) {
  DateTimeData* data = Native::data<DateTimeData>(this_);
  auto tz = TimeZone::Current();
  if (!timezone.isNull()) {
    const Object& obj_timezone = timezone.toObject();
    tz = DateTimeZoneData::unwrap(obj_timezone);
    if (!tz) throw_invalid_object_type(obj_timezone);
  }
  data->m_dt = req::make<DateTime>(TimeStamp::Current(), tz);
  if (!time.empty()) {
    data->m_dt->fromString(time, tz);
  } else if (!timezone.isNull()) {
    // We still have to tell the underlying DateTime the timezone in case they
    // call setTimestamp or something else later
    data->m_dt->setTimezone(tz);
  }
}

Variant HHVM_STATIC_METHOD(DateTime, createFromFormat,
                           const String& format,
                           const String& time,
                           const Variant& timezone /*= uninit_variant */) {
  auto tz = TimeZone::Current();
  if (!timezone.isNull()) {
    const Object& obj_timezone = timezone.toObject();
    if (!obj_timezone.instanceof(s_DateTimeZone)) {
      raise_argument_warning("DateTime::createFromFormat", 3, s_DateTimeZone,
                             obj_timezone);
      return false;
    }
    tz = DateTimeZoneData::unwrap(obj_timezone);
  }
  Object obj{DateTimeData::classof()};
  DateTimeData* data = Native::data<DateTimeData>(obj);
  const auto curr = (format.find("!") != String::npos) ? 0 : ::time(0);
  data->m_dt = req::make<DateTime>(curr, false);
  if (!data->m_dt->fromString(time, tz, format.data(), false)) {
    return false;
  }

  return obj;
}

Variant HHVM_METHOD(DateTime, diff,
                    const Variant& datetime2,
                    const Variant& absolute) {
  auto const data = getDateTimeData(this_);
  const Object obj_datetime2 = datetime2.toObject();
  if (!obj_datetime2.instanceof(s_DateTimeInterface)) {
    raise_argument_warning("DateTime::diff", 1, s_DateTimeInterface,
                           obj_datetime2);
    return false;
  }
  auto dt = DateTimeData::unwrap(obj_datetime2);
  return DateIntervalData::wrap(data->m_dt->diff(dt, absolute.toBoolean()));
}

String HHVM_METHOD(DateTime, format,
                   const Variant& format) {
  auto const data = getDateTimeData(this_);
  return data->format(format.toString());
}

static const StaticString s_warning_count("warning_count");
static const StaticString s_warnings("warnings");
static const StaticString s_error_count("error_count");
static const StaticString s_errors("errors");

Array HHVM_STATIC_METHOD(DateTime, getLastErrors) {
  Array errors = DateTime::getLastErrors();
  Array warnings = DateTime::getLastWarnings();
  DictInit ret(4);

  ret.set(s_warning_count, warnings.size());
  ret.set(s_warnings, warnings);
  ret.set(s_error_count, errors.size());
  ret.set(s_errors, errors);

  return ret.toArray();
}

int64_t HHVM_METHOD(DateTime, getOffset) {
  auto const data = getDateTimeData(this_);
  return data->m_dt->offset();
}

int64_t HHVM_METHOD(DateTime, getTimestamp) {
  auto const data = getDateTimeData(this_);
  return data->getTimestamp();
}

Variant HHVM_METHOD(DateTime, getTimezone) {
  auto const data = getDateTimeData(this_);
  req::ptr<TimeZone> tz = data->m_dt->timezone();
  if (tz->isValid()) {
    return DateTimeZoneData::wrap(tz);
  }
  return false;
}

Variant HHVM_METHOD(DateTime, modify,
                   const String& modify) {
  auto const data = getDateTimeData(this_);
  if (!data->m_dt->modify(modify)) {
    return false;
  }
  return Object(this_);
}

Object HHVM_METHOD(DateTime, setDate,
                   int64_t year,
                   int64_t month,
                   int64_t day) {
  auto const data = getDateTimeData(this_);
  data->m_dt->setDate(year, month, day);
  return Object(this_);
}

Object HHVM_METHOD(DateTime, setISODate,
                   int64_t year,
                   int64_t week,
                   int64_t day /*= 1*/) {
  auto const data = getDateTimeData(this_);
  data->m_dt->setISODate(year, week, day);
  return Object(this_);
}

Object HHVM_METHOD(DateTime, setTime,
                   int64_t hour,
                   int64_t minute,
                   int64_t second /*= 0*/) {
  auto const data = getDateTimeData(this_);
  data->m_dt->setTime(hour, minute, second);
  return Object(this_);
}

Object HHVM_METHOD(DateTime, setTimestamp,
                   int64_t unixtimestamp) {
  auto const data = getDateTimeData(this_);
  data->m_dt->fromTimeStamp(unixtimestamp, false);
  return Object(this_);
}

Variant HHVM_METHOD(DateTime, setTimezone,
                    const Object& timezone) {
  auto const data = getDateTimeData(this_);
  if (!timezone.instanceof(s_DateTimeZone)) {
    raise_argument_warning("DateTime::setTimezone", 1, s_DateTimeZone,
                           timezone);
    return false;
  }
  data->m_dt->setTimezone(DateTimeZoneData::unwrap(timezone));
  return Object(this_);
}

Variant HHVM_METHOD(DateTime, add,
                    const Object& interval) {
  auto const data = getDateTimeData(this_);
  if (!interval.instanceof(s_DateInterval)) {
    raise_argument_warning("DateTime::add", 1, s_DateInterval, interval);
    return false;
  }
  auto di = DateIntervalData::unwrap(interval);
  data->m_dt->add(di);
  return Object(this_);
}

Variant HHVM_METHOD(DateTime, sub,
                    const Object& interval) {
  auto const data = getDateTimeData(this_);
  if (!interval.instanceof(s_DateInterval)) {
    raise_argument_warning("DateTime::sub", 1, s_DateInterval, interval);
    return false;
  }
  auto di = DateIntervalData::unwrap(interval);
  data->m_dt->sub(di);
  return Object(this_);
}

const StaticString
  s_date("date"),
  s_timezone_type("timezone_type"),
  s_timezone("timezone"),
  s_ISOformat("Y-m-d H:i:s.u");

Array HHVM_METHOD(DateTime, __sleep) {
  auto const data = getDateTimeData(this_);

  auto const formatted = data->format(s_ISOformat);
  this_->setProp(nullctx, s_date.get(), formatted.asTypedValue());
  int zoneType = data->m_dt->zoneType();
  this_->setProp(nullctx, s_timezone_type.get(),
                 make_tv<KindOfInt64>(zoneType));
  auto const timezone = zone_type_to_string(zoneType, data->m_dt);
  this_->setProp(nullctx, s_timezone.get(), timezone.asTypedValue());
  return make_vec_array(s_date, s_timezone_type, s_timezone);
}

void HHVM_METHOD(DateTime, __wakeup) {
  Object dtz_obj{DateTimeZoneData::classof()};
  HHVM_MN(DateTimeZone, __construct)(dtz_obj.get(),
                                     this_->o_get(s_timezone).toString());
  HHVM_MN(DateTime, __construct)(this_, this_->o_get(s_date).toString(),
                                 std::move(dtz_obj));

  // cleanup
  auto const klass = this_->getVMClass();
  auto const ctx = MemberLookupContext(klass, klass ? klass->moduleName() : nullptr);
  this_->unsetProp(ctx, s_date.get());
  this_->unsetProp(ctx, s_timezone_type.get());
  this_->unsetProp(ctx, s_timezone.get());
}

Array HHVM_METHOD(DateTime, __debugInfo) {
  auto const data = getDateTimeData(this_);
  return data->getDebugInfo();
}

Array DateTimeData::getDebugInfo() const {
  assertx(m_dt);
  return make_dict_array(
    s_date, format(s_ISOformat),
    s_timezone_type, m_dt->zoneType(),
    s_timezone, zone_type_to_string(m_dt->zoneType(), m_dt)
  );
}

///////////////////////////////////////////////////////////////////////////////
// DateTime helpers

int64_t DateTimeData::getTimestamp(const Object& obj) {
  if (LIKELY(obj.instanceof(classof()))) {
    return getDateTimeData(obj)->getTimestamp();
  }
  assertx(obj->instanceof(SystemLib::getDateTimeInterfaceClass()));
  Variant result = obj->o_invoke(s_getTimestamp, Array::CreateVec());
  return result.toInt64();
}

int64_t DateTimeData::getTimestamp(const ObjectData* od) {
  return getTimestamp(Object(const_cast<ObjectData*>(od)));
}

int DateTimeData::compare(const Object& left, const Object &right) {
  if (LIKELY(left.instanceof(classof()) && right.instanceof(classof()))) {
    auto const leftData = getDateTimeData(left);
    auto const rightData = getDateTimeData(right);
    return leftData->m_dt->compare(rightData->m_dt);
  } else {
    auto leftTime = getTimestamp(left);
    auto rightTime = getTimestamp(right);
    if (leftTime < rightTime) {
      return -1;
    } else if (leftTime > rightTime) {
      return 1;
    } else {
      return 0;
    }
  }
}

int DateTimeData::compare(const ObjectData* left, const ObjectData* right) {
  return compare(Object(const_cast<ObjectData*>(left)),
                 Object(const_cast<ObjectData*>(right)));
}

Object DateTimeData::wrap(req::ptr<DateTime> dt) {
  Object obj{classof()};
  DateTimeData* data = Native::data<DateTimeData>(obj);
  data->m_dt = dt;
  return obj;
}

req::ptr<DateTime> DateTimeData::unwrap(const Object& datetime) {
  if (LIKELY(datetime.instanceof(classof()))) {
    DateTimeData* data = Native::data<DateTimeData>(datetime);
    return data->m_dt;
  }
  if (datetime->instanceof(SystemLib::getDateTimeImmutableClass())) {
    auto rval = datetime->getProp(
      MemberLookupContext(
        SystemLib::getDateTimeImmutableClass(),
        SystemLib::getDateTimeImmutableClass()->moduleName()),
      s_data.get()
    );
    assertx(rval.is_set() && type(rval) == KindOfObject);
    Object impl(rval.val().pobj);
    return unwrap(impl);
  }
  return req::ptr<DateTime>();
}

///////////////////////////////////////////////////////////////////////////////
// DateTimeZone

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
  auto const data = getDateTimeZoneData(this_);
  return data->m_tz->getLocation();
}

String HHVM_METHOD(DateTimeZone, getName) {
  auto const data = getDateTimeZoneData(this_);
  return data->getName();
}

Variant HHVM_METHOD(DateTimeZone, getOffset,
                    const Object& datetime) {
  auto const data = getDateTimeZoneData(this_);
  bool error;
  if (!datetime.instanceof(s_DateTime)) {
    raise_argument_warning("DateTimeZone::getOffset", 1, s_DateTime, datetime);
    return false;
  }
  auto dt = DateTimeData::unwrap(datetime);
  int64_t ts = dt->toTimeStamp(error);
  return data->m_tz->offset(ts);
}

TypedValue HHVM_METHOD(DateTimeZone, getTransitions,
                  int64_t timestamp_begin, /*=k_PHP_INT_MIN*/
                  int64_t timestamp_end /*=k_PHP_INT_MAX*/) {
  auto const data = getDateTimeZoneData(this_);
  auto result = data->m_tz->transitions(timestamp_begin, timestamp_end);
  if (result.isNull()) {
    return make_tv<KindOfBoolean>(false);
  }
  return tvReturn(std::move(result));
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

  const timelib_tzdb *tzdb = timezone_get_tzdb();
  int item_count = tzdb->index_size;
  const timelib_tzdb_index_entry *table = tzdb->index;

  Array ret = Array::CreateVec();
  for (int i = 0; i < item_count; ++i) {
    // This string is what PHP considers as "data" or "info" which is basically
    // the string of "PHP1xx" where xx is country code that uses this timezone.
    // When country code is unknown or not in use anymore, ?? is used instead.
    // There is no known better way to extract this information out.
    const char* infoString = (const char*)&tzdb->data[table[i].pos];
    String countryCode = String(&infoString[5], 2, CopyString);
    if ((what == DateTimeZoneData::PER_COUNTRY && equal(country.get(), countryCode.get()))
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
  Object obj{classof()};
  DateTimeZoneData* data = Native::data<DateTimeZoneData>(obj);
  data->m_tz = tz;
  return obj;
}

req::ptr<TimeZone> DateTimeZoneData::unwrap(const Object& timezone) {
  if (timezone.instanceof(classof())) {
    DateTimeZoneData* data = Native::data<DateTimeZoneData>(timezone);
    return data->m_tz;
  }
  return req::ptr<TimeZone>();
}

Array HHVM_METHOD(DateTimeZone, __debugInfo) {
  auto const data = getDateTimeZoneData(this_);
  return data->getDebugInfo();
}

Array DateTimeZoneData::getDebugInfo() const {
  assertx(m_tz);
  return make_dict_array(
    s_timezone_type, m_tz->type(),
    s_timezone, m_tz->name()
  );
}

///////////////////////////////////////////////////////////////////////////////
// DateInterval

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

template <int64_t (DateInterval::*get)() const>
static Variant get_prop(const Object& this_) {
  return (getDateIntervalData(this_.get())->m_di.get()->*get)();
}
static Variant is_inverted(const Object& this_) {
  return (int)getDateIntervalData(this_.get())->m_di->isInverted();
}
static Variant get_total_days(const Object& this_) {
  auto di = getDateIntervalData(this_.get())->m_di;
  if (!di->haveTotalDays()) return false;
  return di->getTotalDays();
}

template <typename T> using setter_t = void (DateInterval::*)(T);
template <typename T> using cast_t = T (Variant::*)() const;
template <typename T> static void set_prop_impl(
  const Object& this_, const Variant& value, setter_t<T> set, cast_t<T> cast) {
  (getDateIntervalData(this_.get())->m_di.get()->*set)((value.*cast)());
}
template <setter_t<bool> set>
static void set_prop(const Object& this_, const Variant& value) {
  set_prop_impl(this_, value, set, &Variant::toBoolean);
}
template <setter_t<int64_t> set>
static void set_prop(const Object& this_, const Variant& value) {
  set_prop_impl(this_, value, set, &Variant::toInt64);
}

static Native::PropAccessor date_interval_properties[] = {
  { "y", get_prop<&DateInterval::getYears>, set_prop<&DateInterval::setYears> },
  { "m", get_prop<&DateInterval::getMonths>, set_prop<&DateInterval::setMonths> },
  { "d", get_prop<&DateInterval::getDays>, set_prop<&DateInterval::setDays> },
  { "h", get_prop<&DateInterval::getHours>, set_prop<&DateInterval::setHours> },
  { "i", get_prop<&DateInterval::getMinutes>, set_prop<&DateInterval::setMinutes> },
  { "s", get_prop<&DateInterval::getSeconds>, set_prop<&DateInterval::setSeconds> },
  { "invert", is_inverted, set_prop<&DateInterval::setInverted> },
  { "days", get_total_days, set_prop<&DateInterval::setTotalDays> },
  { nullptr }
};

Native::PropAccessorMap date_interval_properties_map{date_interval_properties};
struct DateIntervalPropHandler : Native::MapPropHandler<DateIntervalPropHandler> {
  static constexpr Native::PropAccessorMap& map = date_interval_properties_map;
};

Object HHVM_STATIC_METHOD(DateInterval, createFromDateString,
                          const String& time) {
  return DateIntervalData::wrap(req::make<DateInterval>(time, true));
}

String HHVM_METHOD(DateInterval, format,
                   const String& format) {
  auto const data = getDateIntervalData(this_);
  return data->m_di->format(format);
}

///////////////////////////////////////////////////////////////////////////////
// DateInterval helpers

Object DateIntervalData::wrap(req::ptr<DateInterval> di) {
  Object obj{classof()};
  DateIntervalData* data = Native::data<DateIntervalData>(obj);
  data->m_di = di;
  return obj;
}

req::ptr<DateInterval> DateIntervalData::unwrap(const Object& obj) {
  if (obj.instanceof(classof())) {
    DateIntervalData* data = Native::data<DateIntervalData>(obj);
    return data->m_di;
  }

  return req::ptr<DateInterval>();
}

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

static TypedValue HHVM_FUNCTION(idate,
                                const String& fmt, TypedValue timestamp) {
  if (fmt.size() != 1) {
    raise_invalid_argument_warning("format: %s", fmt.data());
    return make_tv<KindOfBoolean>(false);
  }
  int64_t ret = req::make<DateTime>(
    tvIsNull(timestamp) ? TimeStamp::Current() : tvAssertInt(timestamp),
    false
  )->toInteger(*fmt.data());
  if (ret == -1) return make_tv<KindOfBoolean>(false);
  return make_tv<KindOfInt64>(ret);
}

template<bool gmt>
static TypedValue date_impl(const String& format, TypedValue timestamp) {
  if (!gmt && format.empty()) {
    return tvReturn(empty_string());
  }

  String ret = req::make<DateTime>(
    tvIsNull(timestamp) ? TimeStamp::Current() : tvAssertInt(timestamp),
    gmt
  )->toString(format, false);
  if (ret.isNull()) return make_tv<KindOfBoolean>(false);
  return tvReturn(ret);
}

template<bool gmt>
static TypedValue strftime_impl(const String& format, TypedValue timestamp) {
  String ret = req::make<DateTime>(
    tvIsNull(timestamp) ? TimeStamp::Current() : tvAssertInt(timestamp),
    gmt
  )->toString(format, true);
  if (ret.isNull()) return make_tv<KindOfBoolean>(false);
  return tvReturn(ret);
}

TypedValue HHVM_FUNCTION(strtotime,
                         const String& input, TypedValue timestamp) {
  if (input.empty()) {
    return make_tv<KindOfBoolean>(false);
  }
  auto dt = req::make<DateTime>(
    tvIsNull(timestamp) ? TimeStamp::Current() : tvAssertInt(timestamp));
  if (!dt->fromString(input, req::ptr<TimeZone>(), nullptr, false)) {
    return make_tv<KindOfBoolean>(false);
  }
  bool error;
  return make_tv<KindOfInt64>(dt->toTimeStamp(error));
}

static Array HHVM_FUNCTION(getdate, TypedValue timestamp) {
  return req::make<DateTime>(
    tvIsNull(timestamp) ? TimeStamp::Current() : tvAssertInt(timestamp),
    false
  )->toArray(DateTime::ArrayFormat::TimeMap);
}

static Variant HHVM_FUNCTION(localtime,
                             TypedValue timestamp, bool is_assoc) {
  auto format = is_assoc ? DateTime::ArrayFormat::TmMap
                         : DateTime::ArrayFormat::TmVector;
  return req::make<DateTime>(
    tvIsNull(timestamp) ? TimeStamp::Current() : tvAssertInt(timestamp),
    false
  )->toArray(format);
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
                      int64_t gmtoffset /* = -1 */,
                      int64_t isdst /* = 1 */) {
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
                   int64_t month,
                   int64_t day,
                   int64_t year) {
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
                      const Variant& timezone /* = uninit_variant */) {
  const String& str_time = time.isNull() ? null_string : time.toString();
  auto tz = TimeZone::Current();
  if (!timezone.isNull()) {
    const Object& obj_timezone = timezone.toObject();
    if (!obj_timezone.instanceof(s_DateTimeZone)) {
      raise_argument_warning("date_create", 2, s_DateTimeZone, obj_timezone);
      return false;
    }
    tz = DateTimeZoneData::unwrap(obj_timezone);
  }
  Object ret{DateTimeData::classof()};
  // Don't set the time here because it will throw if it is bad
  HHVM_MN(DateTime, __construct)(ret.get());
  if (str_time.empty()) {
    // zend does this, so so do we
    return ret;
  }
  auto dt = DateTimeData::unwrap(ret);
  if (!dt->fromString(str_time, tz, nullptr, false)) {
    return false;
  }
  return ret;
}

Variant HHVM_FUNCTION(date_format,
                      const Object& datetime,
                      const String& format) {
  if (!datetime.instanceof(s_DateTimeInterface)) {
    raise_argument_warning("date_format", 1, s_DateTimeInterface, datetime);
    return false;
  }
  auto dt = DateTimeData::unwrap(datetime);
  return dt->toString(format, false);
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
TypedValue date_sunrise_sunset(int64_t timestamp, int64_t format,
                               TypedValue latitude, TypedValue longitude,
                               TypedValue zenith, TypedValue offset) {
  return tvReturn(req::make<DateTime>(timestamp, false)->getSunInfo(
    static_cast<DateTime::SunInfoFormat>(format),
    tvIsNull(latitude)
      ? s_date_globals->default_latitude
      : tvAssertDouble(latitude),
    tvIsNull(longitude)
      ? s_date_globals->default_longitude
      : tvAssertDouble(longitude),
    tvIsNull(zenith)
      ? (sunset
        ? s_date_globals->sunset_zenith
        : s_date_globals->sunrise_zenith)
      : tvAssertDouble(zenith),
    tvIsNull(offset)
      ? TimeZone::Current()->offset(0) / 3600
      : tvAssertDouble(offset),
    sunset));
}

///////////////////////////////////////////////////////////////////////////////

#define DATE_ATOM "Y-m-d\\TH:i:sP"
#define DATE_COOKIE "D, d-M-Y H:i:s T"
#define DATE_ISO8601 "Y-m-d\\TH:i:sO"
#define DATE_RFC822 "D, d M y H:i:s O"
#define DATE_RFC850 "l, d-M-y H:i:s T"
#define DATE_RFC1036 "D, d M y H:i:s O"
#define DATE_RFC1123 "D, d M Y H:i:s O"
#define DATE_RFC2822 "D, d M Y H:i:s O"
#define DATE_RFC3339 "Y-m-d\\TH:i:sP"
#define DATE_RSS "D, d M Y H:i:s O"
#define DATE_W3C "Y-m-d\\TH:i:sP"

static struct DateTimeExtension final : Extension {
  DateTimeExtension() : Extension("date", NO_EXTENSION_VERSION_YET, NO_ONCALL_YET) { }

  void moduleRegisterNative() override {
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
    HHVM_ME(DateTime, __debugInfo);
    HHVM_STATIC_ME(DateTime, createFromFormat);
    HHVM_STATIC_ME(DateTime, getLastErrors);

    Native::registerNativeDataInfo<DateTimeData>(Native::NDIFlags::NO_SWEEP);

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
    HHVM_ME(DateTimeZone, __debugInfo);
    HHVM_ME(DateTimeZone, getLocation);
    HHVM_ME(DateTimeZone, getName);
    HHVM_ME(DateTimeZone, getOffset);
    HHVM_ME(DateTimeZone, getTransitions);
    HHVM_STATIC_ME(DateTimeZone, listAbbreviations);
    HHVM_STATIC_ME(DateTimeZone, listIdentifiers);

    Native::registerNativeDataInfo<DateTimeZoneData>(
      Native::NDIFlags::NO_SWEEP);

    HHVM_ME(DateInterval, __construct);
    HHVM_ME(DateInterval, format);
    HHVM_STATIC_ME(DateInterval, createFromDateString);
    Native::registerNativePropHandler<DateIntervalPropHandler>(
      DateIntervalData::className());

    Native::registerNativeDataInfo<DateIntervalData>(
      Native::NDIFlags::NO_SWEEP);

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

    HHVM_RC_INT(SUNFUNCS_RET_DOUBLE,
                static_cast<int64_t>(DateTime::SunInfoFormat::ReturnDouble));
    HHVM_RC_INT(SUNFUNCS_RET_STRING,
                static_cast<int64_t>(DateTime::SunInfoFormat::ReturnString));
    HHVM_RC_INT(SUNFUNCS_RET_TIMESTAMP,
                static_cast<int64_t>(DateTime::SunInfoFormat::ReturnTimeStamp));
  }

  std::vector<std::string> hackFiles() const override {
    return {"datetime"};
  }

  void threadInit() override {
    IniSetting::Bind(
      this, IniSetting::Mode::Request,
      "date.timezone",
      "",
      IniSetting::SetAndGet<std::string>(
        dateTimezoneIniUpdate, dateTimezoneIniGet
      )
    );
    IniSetting::Bind(
      this, IniSetting::Mode::Request,
      "date.default_latitude", "31.7667",
      &s_date_globals->default_latitude
    );
    IniSetting::Bind(
      this, IniSetting::Mode::Request,
      "date.default_longitude", "35.2333",
      &s_date_globals->default_longitude
    );
    IniSetting::Bind(
      this, IniSetting::Mode::Request,
      "date.sunset_zenith", "90.583333",
      &s_date_globals->sunset_zenith
    );
    IniSetting::Bind(
      this, IniSetting::Mode::Request,
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
    return RID().getTimezone();
  }
} s_date_extension;

///////////////////////////////////////////////////////////////////////////////
}
