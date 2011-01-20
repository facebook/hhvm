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

#include <runtime/ext/ext_datetime.h>

namespace HPHP {
IMPLEMENT_DEFAULT_EXTENSION(date);
///////////////////////////////////////////////////////////////////////////////
// constants

const StaticString q_DateTime_ATOM(LITSTR_INIT("Y-m-d\\TH:i:sP"));
const StaticString q_DateTime_COOKIE(LITSTR_INIT("l, d-M-y H:i:s T"));
const StaticString q_DateTime_ISO8601(LITSTR_INIT("Y-m-d\\TH:i:sO"));
const StaticString q_DateTime_RFC822(LITSTR_INIT("D, d M y H:i:s O"));
const StaticString q_DateTime_RFC850(LITSTR_INIT("l, d-M-y H:i:s T"));
const StaticString q_DateTime_RFC1036(LITSTR_INIT("D, d M y H:i:s O"));
const StaticString q_DateTime_RFC1123(LITSTR_INIT("D, d M Y H:i:s O"));
const StaticString q_DateTime_RFC2822(LITSTR_INIT("D, d M Y H:i:s O"));
const StaticString q_DateTime_RFC3339(LITSTR_INIT("Y-m-d\\TH:i:sP"));
const StaticString q_DateTime_RSS(LITSTR_INIT("D, d M Y H:i:s O"));
const StaticString q_DateTime_W3C(LITSTR_INIT("Y-m-d\\TH:i:sP"));

const int64 q_DateTimeZone_AFRICA      = TimeZone::AFRICA;
const int64 q_DateTimeZone_AMERICA     = TimeZone::AMERICA;
const int64 q_DateTimeZone_ANTARCTICA  = TimeZone::ANTARCTICA;
const int64 q_DateTimeZone_ARCTIC      = TimeZone::ARCTIC;
const int64 q_DateTimeZone_ASIA        = TimeZone::ASIA;
const int64 q_DateTimeZone_ATLANTIC    = TimeZone::ATLANTIC;
const int64 q_DateTimeZone_AUSTRALIA   = TimeZone::AUSTRALIA;
const int64 q_DateTimeZone_EUROPE      = TimeZone::EUROPE;
const int64 q_DateTimeZone_INDIAN      = TimeZone::INDIAN;
const int64 q_DateTimeZone_PACIFIC     = TimeZone::PACIFIC;
const int64 q_DateTimeZone_UTC         = TimeZone::UTC;
const int64 q_DateTimeZone_ALL         = TimeZone::ALL;
const int64 q_DateTimeZone_ALL_WITH_BC = TimeZone::ALL_WITH_BC;
const int64 q_DateTimeZone_PER_COUNTRY = TimeZone::PER_COUNTRY;

///////////////////////////////////////////////////////////////////////////////
// methods

#define REL_TIME_INVALID_DAYS -99999

static void rel_time_to_interval(timelib_rel_time *relTime,
                                 c_DateInterval *interval) {
  interval->m_y = relTime->y;
  interval->m_m = relTime->m;
  interval->m_d = relTime->d;
  interval->m_h = relTime->h;
  interval->m_i = relTime->i;
  interval->m_s = relTime->s;
  interval->m_invert = relTime->invert;
  if (relTime->days != REL_TIME_INVALID_DAYS) {
    interval->m_days = relTime->days;
  } else {
    interval->m_days = null;
  }
}

static void interval_to_rel_time(c_DateInterval *interval,
                                  timelib_rel_time *relTime) {
  relTime->y = interval->m_y;
  relTime->m = interval->m_m;
  relTime->d = interval->m_d;
  relTime->h = interval->m_h;
  relTime->i = interval->m_i;
  relTime->s = interval->m_s;
  relTime->invert = interval->m_invert;
  if (interval->m_days.isInteger()) {
    relTime->days = interval->m_days.toInt64();
  } else {
    relTime->days = REL_TIME_INVALID_DAYS;
  }
}

c_DateTime::c_DateTime() {
}

c_DateTime::~c_DateTime() {
}

void c_DateTime::t___construct(CStrRef time /*= "now"*/,
                               CObjRef timezone /*= null_object*/) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DateTime, DateTime::__construct);
  m_dt = NEWOBJ(DateTime)(TimeStamp::Current());
  if (!time.empty()) {
    m_dt->fromString(time, c_DateTimeZone::unwrap(timezone));
  }
}

Object c_DateTime::ti_createfromformat(const char* cls , CStrRef format,
                                       CStrRef time,
                                       CObjRef timezone /*= null_object*/) {
  STATIC_METHOD_INJECTION_BUILTIN(DateTime, DateTime::createfromformat);
  c_DateTime *datetime = NEWOBJ(c_DateTime);
  datetime->m_dt = NEWOBJ(DateTime);
  datetime->m_dt->fromString(time, c_DateTimeZone::unwrap(timezone),
                             format.data());
  return datetime;
}

Object c_DateTime::t_diff(CObjRef datetime2, bool absolute /*= false*/) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DateTime, DateTime::diff);
  timelib_rel_time *relTime = m_dt->diff(*c_DateTime::unwrap(datetime2).get());
  c_DateInterval *interval = NEWOBJ(c_DateInterval);
  rel_time_to_interval(relTime, interval);
  timelib_rel_time_dtor(relTime);
  return interval;
}

Object c_DateTime::t_add(CObjRef interval) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DateTime, DateTime::add);
  c_DateInterval *tInterval = interval.getTyped<c_DateInterval>();
  timelib_rel_time *relTime = timelib_rel_time_ctor();
  interval_to_rel_time(tInterval, relTime);
  m_dt->add(*relTime);
  timelib_rel_time_dtor(relTime);
  return this;
}

Object c_DateTime::t_sub(CObjRef interval) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DateTime, DateTime::sub);
  c_DateInterval *tInterval = interval.getTyped<c_DateInterval>();
  timelib_rel_time *relTime = timelib_rel_time_ctor();
  interval_to_rel_time(tInterval, relTime);
  m_dt->sub(*relTime);
  timelib_rel_time_dtor(relTime);
  return this;
}

String c_DateTime::t_format(CStrRef format) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DateTime, DateTime::format);
  return m_dt->toString(format, false);
}

Variant c_DateTime::ti_getlasterrors(const char* cls) {
  STATIC_METHOD_INJECTION_BUILTIN(DateTime, DateTime::getlasterrors);
  return DateTime::s_last_errors->get();
}

int64 c_DateTime::t_getoffset() {
  INSTANCE_METHOD_INJECTION_BUILTIN(DateTime, DateTime::getoffset);
  return m_dt->offset();
}

Variant c_DateTime::t_gettimezone() {
  INSTANCE_METHOD_INJECTION_BUILTIN(DateTime, DateTime::gettimezone);
  SmartObject<TimeZone> tz = m_dt->timezone();
  if (tz->isValid()) {
    return c_DateTimeZone::wrap(tz);
  }
  return false;
}

Object c_DateTime::t_modify(CStrRef modify) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DateTime, DateTime::modify);
  m_dt->modify(modify);
  return this;
}

Object c_DateTime::t_setdate(int64 year, int64 month, int64 day) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DateTime, DateTime::setdate);
  m_dt->setDate(year, month, day);
  return this;
}

Object c_DateTime::t_setisodate(int64 year, int64 week, int64 day /*= 1*/) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DateTime, DateTime::setisodate);
  m_dt->setISODate(year, week, day);
  return this;
}

Object c_DateTime::t_settime(int64 hour, int64 minute, int64 second /*= 0*/) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DateTime, DateTime::settime);
  m_dt->setTime(hour, minute, second);
  return this;
}

Variant c_DateTime::t_gettimestamp() {
  INSTANCE_METHOD_INJECTION_BUILTIN(DateTime, DateTime::gettimestamp);
  bool err;
  int64 timestamp = m_dt->toTimeStamp(err);
  if (!err) {
    return timestamp;
  } else {
    return false;
  }
}

Object c_DateTime::t_settimestamp(int64 unixtimestamp) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DateTime, DateTime::settimestamp);
  m_dt->fromTimeStamp(unixtimestamp, false);
  return this;
}

Object c_DateTime::t_settimezone(CObjRef timezone) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DateTime, DateTime::settimezone);
  m_dt->setTimezone(c_DateTimeZone::unwrap(timezone));
  return this;
}

Variant c_DateTime::t___destruct() {
  INSTANCE_METHOD_INJECTION_BUILTIN(DateTime, DateTime::__destruct);
  return null;
}

ObjectData *c_DateTime::clone() {
  ObjectData *obj = cloneImpl();
  c_DateTime *dt = static_cast<c_DateTime*>(obj);
  dt->m_dt = m_dt->cloneDateTime();
  return obj;
}

c_DateTimeZone::c_DateTimeZone() {
}

c_DateTimeZone::~c_DateTimeZone() {
}

void c_DateTimeZone::t___construct(CStrRef timezone) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DateTimeZone, DateTimeZone::__construct);
  m_tz = NEWOBJ(TimeZone)(timezone);
  if (!m_tz->isValid()) {
    raise_error("DateTimeZone::__construct(): Unknown or bad timezone (%s)",
                timezone.data());
  }
}

String c_DateTimeZone::t_getname() {
  INSTANCE_METHOD_INJECTION_BUILTIN(DateTimeZone, DateTimeZone::getname);
  return m_tz->name();
}

int64 c_DateTimeZone::t_getoffset(CObjRef datetime) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DateTimeZone, DateTimeZone::getoffset);
  bool error;
  int64 ts = c_DateTime::unwrap(datetime)->toTimeStamp(error);
  return m_tz->offset(ts);
}

Array c_DateTimeZone::t_getlocation() {
  INSTANCE_METHOD_INJECTION_BUILTIN(DateTimeZone, DateTimeZone::getlocation);
  return m_tz->getLocation();
}

Array c_DateTimeZone::t_gettransitions(int64 timestamp_begin /*= LLONG_MIN*/,
                                       int64 timestamp_end /*= LLONG_MAX*/) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DateTimeZone, DateTimeZone::gettransitions);
  return m_tz->transitions(timestamp_begin, timestamp_end);
}

Array c_DateTimeZone::ti_listabbreviations(const char* cls) {
  STATIC_METHOD_INJECTION_BUILTIN(DateTimeZone, DateTimeZone::listabbreviations);
  return TimeZone::GetAbbreviations();
}

Array c_DateTimeZone::ti_listidentifiers(const char* cls, int64 what /*= 2047*/,
                                         CStrRef country /*= null_string*/) {
  STATIC_METHOD_INJECTION_BUILTIN(DateTimeZone, DateTimeZone::listidentifiers);
  return TimeZone::GetNames(what, country);
}

Variant c_DateTimeZone::t___destruct() {
  INSTANCE_METHOD_INJECTION_BUILTIN(DateTimeZone, DateTimeZone::__destruct);
  return null;
}

ObjectData *c_DateTimeZone::clone() {
  ObjectData *obj = cloneImpl();
  c_DateTimeZone *dtz = static_cast<c_DateTimeZone*>(obj);
  dtz->m_tz = m_tz->cloneTimeZone();
  return obj;
}

c_DateInterval::c_DateInterval() {
}

c_DateInterval::~c_DateInterval() {
}

void c_DateInterval::t___construct(CStrRef interval_spec) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DateInterval, DateInterval::__construct);
  timelib_time     *b = NULL, *e = NULL;
  timelib_rel_time *relTime = NULL;
  int               r = 0;
  timelib_error_container *errors;

  timelib_strtointerval((char*)interval_spec.data(), interval_spec.size(),
                        &b, &e, &relTime, &r, &errors);

  int errors1 = errors->error_count;
  timelib_error_container_dtor(errors);
  if (errors1 > 0) {
    timelib_rel_time_dtor(relTime);
    raise_error("Unknown or bad format (%s)", interval_spec.c_str());
  } else {
    rel_time_to_interval(relTime, this);
    timelib_rel_time_dtor(relTime);
  }
}

Variant c_DateInterval::t___destruct() {
  INSTANCE_METHOD_INJECTION_BUILTIN(DateInterval, DateInterval::__destruct);
  return null;
}

Object c_DateInterval::ti_createfromdatestring(const char* cls , CStrRef time) {
  STATIC_METHOD_INJECTION_BUILTIN(DateInterval, DateInterval::createfromdatestring);
  DateTime datetime;
  datetime.fromString(time, SmartObject<TimeZone>());
  c_DateInterval *interval = NEWOBJ(c_DateInterval);
  rel_time_to_interval(datetime.getRelTime(), interval);
  return interval;
}

String c_DateInterval::t_format(CStrRef format) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DateInterval, DateInterval::format);
  StringBuffer s;
  const int LENGTH = 33;
  char buf[LENGTH];
  int l;
  bool hasFormatSpec = false;
  for (int i = 0; i < format.length(); ++i) {
    char c = format.charAt(i);
    if (!hasFormatSpec) {
      if (c == '%') {
        hasFormatSpec = true;
      } else {
        s.append(c);
      }
    } else {
      switch (c) {
      case 'Y': l = snprintf(buf, LENGTH, "%02lld", m_y); break;
      case 'y': l = snprintf(buf, LENGTH, "%lld", m_y); break;

      case 'M': l = snprintf(buf, LENGTH, "%02lld", m_m); break;
      case 'm': l = snprintf(buf, LENGTH, "%lld", m_m); break;

      case 'D': l = snprintf(buf, LENGTH, "%02lld", m_d); break;
      case 'd': l = snprintf(buf, LENGTH, "%lld", m_d); break;

      case 'H': l = snprintf(buf, LENGTH, "%02lld", m_h); break;
      case 'h': l = snprintf(buf, LENGTH, "%lld", m_h); break;

      case 'I': l = snprintf(buf, LENGTH, "%02lld", m_i); break;
      case 'i': l = snprintf(buf, LENGTH, "%lld", m_i); break;

      case 'S': l = snprintf(buf, LENGTH, "%02lld", m_s); break;
      case 's': l = snprintf(buf, LENGTH, "%lld", m_s); break;

      case 'a': {
        if (m_days.isInteger()) {
          l = snprintf(buf, LENGTH, "%lld", m_days.toInt64());
        } else {
          l = snprintf(buf, LENGTH, "(unknown)");
        }
        break;
      }

      case 'r': l = snprintf(buf, LENGTH, "%s", m_invert ? "-" : ""); break;
      case 'R': l = snprintf(buf, LENGTH, "%c", m_invert ? '-' : '+'); break;

      case '%': l = snprintf(buf, 32, "%%"); break;
      default: buf[0] = '%'; buf[1] = c; buf[2] = '\0'; l = 2; break;
      }

      s.append(buf, std::min(l, LENGTH - 1));
      hasFormatSpec = false;
    }
  }
  return s.detach();
}

///////////////////////////////////////////////////////////////////////////////
}
