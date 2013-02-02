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

#include <system/lib/systemlib.h>

namespace HPHP {
IMPLEMENT_DEFAULT_EXTENSION(date);
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

const int64 q_DateTimeZone$$AFRICA = 1;
const int64 q_DateTimeZone$$AMERICA = 2;
const int64 q_DateTimeZone$$ANTARCTICA = 4;
const int64 q_DateTimeZone$$ARCTIC = 8;
const int64 q_DateTimeZone$$ASIA = 16;
const int64 q_DateTimeZone$$ATLANTIC = 32;
const int64 q_DateTimeZone$$AUSTRALIA = 64;
const int64 q_DateTimeZone$$EUROPE = 128;
const int64 q_DateTimeZone$$INDIAN = 256;
const int64 q_DateTimeZone$$PACIFIC = 512;
const int64 q_DateTimeZone$$UTC = 1024;
const int64 q_DateTimeZone$$ALL = 2047;
const int64 q_DateTimeZone$$ALL_WITH_BC = 4095;
const int64 q_DateTimeZone$$PER_COUNTRY = 4096;

///////////////////////////////////////////////////////////////////////////////
// methods

c_DateTime::c_DateTime(const ObjectStaticCallbacks *cb) : ExtObjectData(cb) {
}

c_DateTime::~c_DateTime() {
}

Object c_DateTime::t_add(CObjRef interval) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DateTime, DateTime::add);
  m_dt->add(c_DateInterval::unwrap(interval));
  return this;
}

void c_DateTime::t___construct(CStrRef time /*= "now"*/,
                               CObjRef timezone /*= null_object*/) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DateTime, DateTime::__construct);
  m_dt = NEWOBJ(DateTime)(TimeStamp::Current());
  if (!time.empty()) {
    m_dt->fromString(time, c_DateTimeZone::unwrap(timezone));
  }
}

Object c_DateTime::ti_createfromformat(const char* cls , CStrRef format, CStrRef time,
                                       CObjRef timezone /*= null_object */) {
  STATIC_METHOD_INJECTION_BUILTIN(DateTime, DateTime::createfromformat);
  c_DateTime *datetime = NEWOBJ(c_DateTime);
  datetime->m_dt = NEWOBJ(DateTime);
  datetime->m_dt->fromString(time, c_DateTimeZone::unwrap(timezone), format.data());
  return datetime;
}

Object c_DateTime::t_diff(CObjRef datetime2, bool absolute) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DateTime, DateTime::diff);
  return c_DateInterval::wrap(m_dt->diff(c_DateTime::unwrap(datetime2), absolute));
}

String c_DateTime::t_format(CStrRef format) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DateTime, DateTime::format);
  return m_dt->toString(format, false);
}

Array c_DateTime::ti_getlasterrors(const char* cls ) {
  STATIC_METHOD_INJECTION_BUILTIN(DateTime, DateTime::getlasterrors);
  Array errors = DateTime::getLastErrors();
  Array warnings = DateTime::getLastWarnings();
  Array ret = Array::Create();

  ret.add("warning_count", warnings.size());
  ret.add("warnings", warnings);
  ret.add("error_count", errors.size());
  ret.add("errors", errors);

  return ret;
}

int64 c_DateTime::t_getoffset() {
  INSTANCE_METHOD_INJECTION_BUILTIN(DateTime, DateTime::getoffset);
  return m_dt->offset();
}

int64 c_DateTime::t_gettimestamp() {
  INSTANCE_METHOD_INJECTION_BUILTIN(DateTime, DateTime::gettimestamp);
  bool err = false;
  return m_dt->toTimeStamp(err);
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

Object c_DateTime::t_sub(CObjRef interval) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DateTime, DateTime::sub);
  m_dt->sub(c_DateInterval::unwrap(interval));
  return this;
}

ObjectData *c_DateTime::clone() {
  ObjectData *obj = ObjectData::clone();
  c_DateTime *dt = static_cast<c_DateTime*>(obj);
  dt->m_dt = m_dt->cloneDateTime();
  return obj;
}

c_DateTimeZone::c_DateTimeZone(const ObjectStaticCallbacks *cb) :
    ExtObjectData(cb) {
}

c_DateTimeZone::~c_DateTimeZone() {
}

void c_DateTimeZone::t___construct(CStrRef timezone) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DateTimeZone, DateTimeZone::__construct);
  m_tz = NEWOBJ(TimeZone)(timezone);
  if (!m_tz->isValid()) {
    std::string msg = "DateTimeZone::__construct(): Unknown or bad timezone (";
    msg += timezone.data();
    msg += ")";
    throw Object(SystemLib::AllocExceptionObject(msg));
  }
}

Array c_DateTimeZone::t_getlocation() {
  INSTANCE_METHOD_INJECTION_BUILTIN(DateTimeZone, DateTimeZone::getlocation);
  return m_tz->getLocation();
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

Array c_DateTimeZone::t_gettransitions() {
  INSTANCE_METHOD_INJECTION_BUILTIN(DateTimeZone, DateTimeZone::gettransitions);
  return m_tz->transitions();
}

Array c_DateTimeZone::ti_listabbreviations(const char* cls) {
  STATIC_METHOD_INJECTION_BUILTIN(DateTimeZone, DateTimeZone::listabbreviations);
  return TimeZone::GetAbbreviations();
}

Array c_DateTimeZone::ti_listidentifiers(const char* cls) {
  STATIC_METHOD_INJECTION_BUILTIN(DateTimeZone, DateTimeZone::listidentifiers);
  return TimeZone::GetNames();
}

ObjectData *c_DateTimeZone::clone() {
  ObjectData *obj = ObjectData::clone();
  c_DateTimeZone *dtz = static_cast<c_DateTimeZone*>(obj);
  dtz->m_tz = m_tz->cloneTimeZone();
  return obj;
}

c_DateInterval::c_DateInterval(const ObjectStaticCallbacks *cb) :
    ExtObjectDataFlags<ObjectData::UseGet|ObjectData::UseSet>(cb) {
}

c_DateInterval::~c_DateInterval() {
}

void c_DateInterval::t___construct(CStrRef interval_spec) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DateInterval, DateInterval::__construct);
  m_di = NEWOBJ(DateInterval)(interval_spec);
  if (!m_di->isValid()) {
    std::string msg = "DateInterval::__construct: Invalid interval (";
    msg += interval_spec.data();
    msg += ")";
    throw Object(SystemLib::AllocExceptionObject(msg));
  }
}

Variant c_DateInterval::t___get(Variant member) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DateInterval, DateInterval::__get);
  if (member.isString()) {
    if (member.same("y"))      return m_di->getYears();
    if (member.same("m"))      return m_di->getMonths();
    if (member.same("d"))      return m_di->getDays();
    if (member.same("h"))      return m_di->getHours();
    if (member.same("i"))      return m_di->getMinutes();
    if (member.same("s"))      return m_di->getSeconds();
    if (member.same("invert")) return m_di->isInverted();
    if (member.same("days")) {
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
  INSTANCE_METHOD_INJECTION_BUILTIN(DateInterval, DateInterval::__set);
  if (member.isString()) {
    if (member.same("y")) { m_di->setYears(value.toInt64());   return null; }
    if (member.same("m")) { m_di->setMonths(value.toInt64());  return null; }
    if (member.same("d")) { m_di->setDays(value.toInt64());    return null; }
    if (member.same("h")) { m_di->setHours(value.toInt64());   return null; }
    if (member.same("i")) { m_di->setMinutes(value.toInt64()); return null; }
    if (member.same("s")) { m_di->setSeconds(value.toInt64()); return null; }
    if (member.same("invert")) {
      m_di->setInverted(value.toBoolean());
      return null;
    }
    if (member.same("days")) {
      m_di->setTotalDays(value.toInt64());
      return null;
    }
  }

  std::string msg = "Undefined property '";
  msg += member.toString().data();
  msg += ") on DateInterval object";
  throw Object(SystemLib::AllocExceptionObject(msg));
}

Object c_DateInterval::ti_createfromdatestring(const char* cls , CStrRef time) {
  STATIC_METHOD_INJECTION_BUILTIN(DateInterval, DateInterval::createfromdatestring);
  SmartObject<DateInterval> di(NEWOBJ(DateInterval)(time, true));
  return c_DateInterval::wrap(di);
}

String c_DateInterval::t_format(CStrRef format) {
  INSTANCE_METHOD_INJECTION_BUILTIN(DateInterval, DateInterval::format);
  return m_di->format(format);
}

ObjectData *c_DateInterval::clone() {
  ObjectData *obj = ObjectData::clone();
  c_DateInterval *di = static_cast<c_DateInterval*>(obj);
  di->m_di = m_di->cloneDateInterval();
  return obj;
}

///////////////////////////////////////////////////////////////////////////////
}
