/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

const StaticString q_datetime_ATOM(LITSTR_INIT("Y-m-d\\TH:i:sP"));
const StaticString q_datetime_COOKIE(LITSTR_INIT("l, d-M-y H:i:s T"));
const StaticString q_datetime_ISO8601(LITSTR_INIT("Y-m-d\\TH:i:sO"));
const StaticString q_datetime_RFC822(LITSTR_INIT("D, d M y H:i:s O"));
const StaticString q_datetime_RFC850(LITSTR_INIT("l, d-M-y H:i:s T"));
const StaticString q_datetime_RFC1036(LITSTR_INIT("D, d M y H:i:s O"));
const StaticString q_datetime_RFC1123(LITSTR_INIT("D, d M Y H:i:s O"));
const StaticString q_datetime_RFC2822(LITSTR_INIT("D, d M Y H:i:s O"));
const StaticString q_datetime_RFC3339(LITSTR_INIT("Y-m-d\\TH:i:sP"));
const StaticString q_datetime_RSS(LITSTR_INIT("D, d M Y H:i:s O"));
const StaticString q_datetime_W3C(LITSTR_INIT("Y-m-d\\TH:i:sP"));

const int64 q_datetimezone_AFRICA = 1;
const int64 q_datetimezone_AMERICA = 2;
const int64 q_datetimezone_ANTARCTICA = 4;
const int64 q_datetimezone_ARCTIC = 8;
const int64 q_datetimezone_ASIA = 16;
const int64 q_datetimezone_ATLANTIC = 32;
const int64 q_datetimezone_AUSTRALIA = 64;
const int64 q_datetimezone_EUROPE = 128;
const int64 q_datetimezone_INDIAN = 256;
const int64 q_datetimezone_PACIFIC = 512;
const int64 q_datetimezone_UTC = 1024;
const int64 q_datetimezone_ALL = 2047;
const int64 q_datetimezone_ALL_WITH_BC = 4095;
const int64 q_datetimezone_PER_COUNTRY = 4096;

///////////////////////////////////////////////////////////////////////////////
// methods

c_datetime::c_datetime() {
}

c_datetime::~c_datetime() {
}

void c_datetime::t___construct(CStrRef time /*= "now"*/,
                               CObjRef timezone /*= null_object*/) {
  INSTANCE_METHOD_INJECTION_BUILTIN(datetime, datetime::__construct);
  m_dt = NEW(DateTime)(TimeStamp::Current());
  if (!time.empty()) {
    m_dt->fromString(time, c_datetimezone::unwrap(timezone));
  }
}

String c_datetime::t_format(CStrRef format) {
  INSTANCE_METHOD_INJECTION_BUILTIN(datetime, datetime::format);
  return m_dt->toString(format, false);
}

int64 c_datetime::t_getoffset() {
  INSTANCE_METHOD_INJECTION_BUILTIN(datetime, datetime::getoffset);
  return m_dt->offset();
}

Variant c_datetime::t_gettimezone() {
  INSTANCE_METHOD_INJECTION_BUILTIN(datetime, datetime::gettimezone);
  SmartObject<TimeZone> tz = m_dt->timezone();
  if (tz->isValid()) {
    return c_datetimezone::wrap(tz);
  }
  return false;
}

Object c_datetime::t_modify(CStrRef modify) {
  INSTANCE_METHOD_INJECTION_BUILTIN(datetime, datetime::modify);
  m_dt->modify(modify);
  return this;
}

Object c_datetime::t_setdate(int64 year, int64 month, int64 day) {
  INSTANCE_METHOD_INJECTION_BUILTIN(datetime, datetime::setdate);
  m_dt->setDate(year, month, day);
  return this;
}

Object c_datetime::t_setisodate(int64 year, int64 week, int64 day /*= 1*/) {
  INSTANCE_METHOD_INJECTION_BUILTIN(datetime, datetime::setisodate);
  m_dt->setISODate(year, week, day);
  return this;
}

Object c_datetime::t_settime(int64 hour, int64 minute, int64 second /*= 0*/) {
  INSTANCE_METHOD_INJECTION_BUILTIN(datetime, datetime::settime);
  m_dt->setTime(hour, minute, second);
  return this;
}

Object c_datetime::t_settimezone(CObjRef timezone) {
  INSTANCE_METHOD_INJECTION_BUILTIN(datetime, datetime::settimezone);
  m_dt->setTimezone(c_datetimezone::unwrap(timezone));
  return this;
}

Variant c_datetime::t___destruct() {
  INSTANCE_METHOD_INJECTION_BUILTIN(datetime, datetime::__destruct);
  return null;
}

c_datetimezone::c_datetimezone() {
}

c_datetimezone::~c_datetimezone() {
}

void c_datetimezone::t___construct(CStrRef timezone) {
  INSTANCE_METHOD_INJECTION_BUILTIN(datetimezone, datetimezone::__construct);
  m_tz = NEW(TimeZone)(timezone);
}

String c_datetimezone::t_getname() {
  INSTANCE_METHOD_INJECTION_BUILTIN(datetimezone, datetimezone::getname);
  return m_tz->name();
}

int64 c_datetimezone::t_getoffset(CObjRef datetime) {
  INSTANCE_METHOD_INJECTION_BUILTIN(datetimezone, datetimezone::getoffset);
  bool error;
  int64 ts = c_datetime::unwrap(datetime)->toTimeStamp(error);
  return m_tz->offset(ts);
}

Array c_datetimezone::t_gettransitions() {
  INSTANCE_METHOD_INJECTION_BUILTIN(datetimezone, datetimezone::gettransitions);
  return m_tz->transitions();
}

Array c_datetimezone::ti_listabbreviations(const char* cls) {
  STATIC_METHOD_INJECTION_BUILTIN(datetimezone, datetimezone::listabbreviations);
  return TimeZone::GetAbbreviations();
}

Array c_datetimezone::ti_listidentifiers(const char* cls) {
  STATIC_METHOD_INJECTION_BUILTIN(datetimezone, datetimezone::listidentifiers);
  return TimeZone::GetNames();
}

Variant c_datetimezone::t___destruct() {
  INSTANCE_METHOD_INJECTION_BUILTIN(datetimezone, datetimezone::__destruct);
  return null;
}

///////////////////////////////////////////////////////////////////////////////
}
