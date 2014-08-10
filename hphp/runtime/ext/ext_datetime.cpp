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

#include "hphp/runtime/ext/ext_datetime.h"
#include "hphp/runtime/base/ini-setting.h"

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

static String zone_type_to_string(int zoneType, SmartResource<DateTime> dt) {
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

static class DateExtension : public Extension {
 public:
  DateExtension() : Extension("date", k_PHP_VERSION.c_str()) { }
  void threadInit() {
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

  double get_date_default_gmt_offset() {
    SmartResource<TimeZone> tzi = TimeZone::Current();
    // just get the offset form utc time
    // set the timestamp 0 is ok
    return tzi->offset(0) / 3600;
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

Object c_DateTime::t_add(const Object& interval) {
  m_dt->add(c_DateInterval::unwrap(interval));
  return this;
}

void c_DateTime::t___construct(const String& time /*= "now"*/,
                               const Object& timezone /*= null_object*/) {
  m_dt = NEWOBJ(DateTime)(TimeStamp::Current());
  if (!time.empty()) {
    m_dt->fromString(time, c_DateTimeZone::unwrap(timezone));
  } else if (!timezone.isNull()) {
    // We still have to tell the underlying DateTime the timezone incase they
    // call setTimestamp or something else later
    m_dt->setTimezone(c_DateTimeZone::unwrap(timezone));
  }
}

Variant c_DateTime::ti_createfromformat(const String& format,
                                        const String& time,
                                        const Object& timezone /*= null_object */) {
  c_DateTime *datetime = NEWOBJ(c_DateTime);
  const auto curr = (format.find("!") != String::npos) ? 0 : f_time() ;
  datetime->m_dt = NEWOBJ(DateTime(curr, false));
  if (!datetime->m_dt->fromString(time, c_DateTimeZone::unwrap(timezone),
                                 format.data(), false)) {
    return false;
  }

  return datetime;
}

Object c_DateTime::t_diff(const Object& datetime2, bool absolute) {
  return c_DateInterval::wrap(m_dt->diff(c_DateTime::unwrap(datetime2),
                                         absolute));
}

String c_DateTime::t_format(const String& format) {
  return m_dt->toString(format, false);
}

const StaticString s_data("data");
const StaticString s_getTimestamp("getTimestamp");

int64_t c_DateTime::GetTimestamp(const Object& obj) {
  if (LIKELY(obj.is<c_DateTime>())) {
    return obj.getTyped<c_DateTime>(true)->t_gettimestamp();
  }
  assert(obj->instanceof(SystemLib::s_DateTimeInterfaceClass));
  Variant result = obj->o_invoke(s_getTimestamp, Array::Create());
  return result.toInt64();
}

int64_t c_DateTime::GetTimestamp(const ObjectData* od) {
  return GetTimestamp(Object(const_cast<ObjectData*>(od)));
}

SmartResource<DateTime> c_DateTime::unwrap(const Object& datetime) {
  if (LIKELY(datetime.is<c_DateTime>())) {
    SmartObject<c_DateTime> cdt = datetime.getTyped<c_DateTime>(true);
    if (cdt.get() == nullptr)
      return SmartResource<DateTime>();
    return cdt->m_dt;
  }
  if (datetime->instanceof(SystemLib::s_DateTimeImmutableClass)) {
    bool visible, accessible, unset;
    TypedValue* tv = datetime->getProp(SystemLib::s_DateTimeImmutableClass,
                                       s_data.get(),
                                       visible,
                                       accessible,
                                       unset);
    assert(tv->m_type == KindOfObject);
    Object impl(tv->m_data.pobj);
    return unwrap(impl);
  }
  return SmartResource<DateTime>();
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

Object c_DateTime::t_settimezone(const Object& timezone) {
  m_dt->setTimezone(c_DateTimeZone::unwrap(timezone));
  return this;
}

Object c_DateTime::t_sub(const Object& interval) {
  m_dt->sub(c_DateInterval::unwrap(interval));
  return this;
}

const StaticString
  s_date("date"),
  s_timezone_type("timezone_type"),
  s_timezone("timezone"),
  s_ISOformat("Y-m-d H:i:s.u");

Array c_DateTime::t___sleep() {
  int zoneType = m_dt->zoneType();

  o_set(s_date, t_format(s_ISOformat));
  o_set(s_timezone_type, zoneType);
  o_set(s_timezone, zone_type_to_string(zoneType, m_dt));
  return make_packed_array(s_date, s_timezone_type, s_timezone);
}

void c_DateTime::t___wakeup() {
  c_DateTimeZone *ctz = NEWOBJ(c_DateTimeZone)();
  Object timezone(ctz);
  ctz->t___construct(o_get(s_timezone).toString());

  t___construct(o_get(s_date).toString(), timezone);

  // cleanup
  unsetProp(getVMClass(), s_date.get());
  unsetProp(getVMClass(), s_timezone_type.get());
  unsetProp(getVMClass(), s_timezone.get());
}

Array c_DateTime::t___debuginfo() {
  return make_map_array(
    s_date, t_format(s_ISOformat),
    s_timezone_type, m_dt->zoneType(),
    s_timezone, zone_type_to_string(m_dt->zoneType(), m_dt)
  );
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

int64_t c_DateTimeZone::t_getoffset(const Object& datetime) {
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

Variant c_DateTimeZone::ti_listidentifiers(int64_t what,
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
      return init_null();
    }
    if (same(member, s_m)) {
      m_di->setMonths(value.toInt64());
      return init_null();
    }
    if (same(member, s_d)) {
      m_di->setDays(value.toInt64());
      return init_null();
    }
    if (same(member, s_h)) {
      m_di->setHours(value.toInt64());
      return init_null();
    }
    if (same(member, s_i)) {
      m_di->setMinutes(value.toInt64());
      return init_null();
    }
    if (same(member, s_s)) {
      m_di->setSeconds(value.toInt64());
      return init_null();
    }
    if (same(member, s_invert)) {
      m_di->setInverted(value.toBoolean());
      return init_null();
    }
    if (same(member, s_days)) {
      m_di->setTotalDays(value.toInt64());
      return init_null();
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
  if (format.empty()) return empty_string_variant();
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

Variant f_timezone_identifiers_list(int64_t what, const String& country) {
  return c_DateTimeZone::ti_listidentifiers(what, country);
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

Array f_timezone_location_get(const Object& timezone) {
  return timezone.getTyped<c_DateTimeZone>()->t_getlocation();
}

String f_timezone_name_get(const Object& object) {
  return object.getTyped<c_DateTimeZone>()->t_getname();
}

int64_t f_timezone_offset_get(const Object& object, const Object& dt) {
  return object.getTyped<c_DateTimeZone>()->t_getoffset(dt);
}

Array f_timezone_transitions_get(const Object& object) {
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

Object f_date_add(const Object& datetime, const Object& interval) {
  return datetime.getTyped<c_DateTime>()->
    t_add(interval.getTyped<c_DateInterval>());
}

Variant f_date_create_from_format(const String& format,
                                 const String& time,
                                 const Object& timezone /* = null_object */) {
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
                      const Object& timezone /* = null_object */) {
  c_DateTime *cdt = NEWOBJ(c_DateTime)();
  Object ret(cdt);
  // Don't set the time here because it will throw if it is bad
  cdt->t___construct();
  if (time.empty()) {
    // zend does this, so so do we
    return ret;
  }
  auto dt = c_DateTime::unwrap(ret);
  if (!dt->fromString(time, c_DateTimeZone::unwrap(timezone), nullptr, false)) {
    return false;
  }
  return ret;
}

void f_date_date_set(const Object& object, int year, int month, int day) {
  object.getTyped<c_DateTime>()->t_setdate(year, month, day);
}

Object f_date_diff(const Object& datetime,
                   const Object& datetime2,
                   bool absolute /* = false */) {
  return datetime.getTyped<c_DateTime>()->
    t_diff(datetime2.getTyped<c_DateTime>(), absolute);
}

void f_date_isodate_set(const Object& object, int year, int week,
                        int day /* = 1 */) {
  object.getTyped<c_DateTime>()->t_setisodate(year, week, day);
}

String f_date_format(const Object& object, const String& format) {
  return c_DateTime::unwrap(object.getTyped<c_DateTime>())->
                            toString(format, false);
}

Array f_date_get_last_errors() {
  return c_DateTime::ti_getlasterrors();
}

Object f_date_interval_create_from_date_string(const String& time) {
  return c_DateInterval::ti_createfromdatestring(time);
}

String f_date_interval_format(const Object& interval, const String& format_spec) {
  return interval.getTyped<c_DateInterval>()->t_format(format_spec);
}

void f_date_modify(const Object& object, const String& modify) {
  object.getTyped<c_DateTime>()->t_modify(modify);
}

Variant f_date_parse(const String& date) {
  Array ret = DateTime::Parse(date);
  if (ret.empty()) {
    return false;
  }
  return ret;
}

void f_date_time_set(const Object& object, int hour, int minute,
                     int second /* = 0 */) {
  object.getTyped<c_DateTime>()->t_settime(hour, minute, second);
}

int64_t f_date_timestamp_get(const Object& datetime) {
  return datetime.getTyped<c_DateTime>()->t_gettimestamp();
}

Object f_date_timestamp_set(const Object& datetime, int64_t timestamp) {
  return datetime.getTyped<c_DateTime>()->
    t_settimestamp(timestamp);
}

Variant f_date_timezone_get(const Object& object) {
  return object.getTyped<c_DateTime>()->t_gettimezone();
}

void f_date_timezone_set(const Object& object, const Object& timezone) {
  object.getTyped<c_DateTime>()->t_settimezone(timezone);
}

Object f_date_sub(const Object& datetime, const Object& interval) {
  return datetime.getTyped<c_DateTime>()->
    t_sub(interval.getTyped<c_DateInterval>());
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
  return s_date_extension.get_date_default_gmt_offset();
}

Array f_date_sun_info(int64_t ts, double latitude, double longitude) {
  return DateTime(ts, false).getSunInfo(latitude, longitude);
}

Variant f_date_sunrise(int64_t timestamp,
                       int format,
                       double latitude,
                       double longitude,
                       double zenith,
                       double gmt_offset) {
  return DateTime(timestamp, false).getSunInfo
    (static_cast<DateTime::SunInfoFormat>(format), latitude, longitude,
     zenith, gmt_offset, false);
}

Variant f_date_sunset(int64_t timestamp,
                      int format,
                      double latitude,
                      double longitude,
                      double zenith,
                      double gmt_offset) {
  return DateTime(timestamp, false).getSunInfo
    (static_cast<DateTime::SunInfoFormat>(format), latitude, longitude,
     zenith, gmt_offset, true);
}

///////////////////////////////////////////////////////////////////////////////
}
