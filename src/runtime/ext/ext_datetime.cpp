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

const StaticString q_datetime_ATOM = "Y-m-d\\TH:i:sP";
const StaticString q_datetime_COOKIE = "l, d-M-y H:i:s T";
const StaticString q_datetime_ISO8601 = "Y-m-d\\TH:i:sO";
const StaticString q_datetime_RFC822 = "D, d M y H:i:s O";
const StaticString q_datetime_RFC850 = "l, d-M-y H:i:s T";
const StaticString q_datetime_RFC1036 = "D, d M y H:i:s O";
const StaticString q_datetime_RFC1123 = "D, d M Y H:i:s O";
const StaticString q_datetime_RFC2822 = "D, d M Y H:i:s O";
const StaticString q_datetime_RFC3339 = "Y-m-d\\TH:i:sP";
const StaticString q_datetime_RSS = "D, d M Y H:i:s O";
const StaticString q_datetime_W3C = "Y-m-d\\TH:i:sP";

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
  m_dt = NEW(DateTime)(TimeStamp::Current());
  if (!time.empty()) {
    m_dt->fromString(time, c_datetimezone::unwrap(timezone));
  }
}

String c_datetime::t_format(CStrRef format) {
  return m_dt->toString(format, false);
}

int64 c_datetime::t_getoffset() {
  return m_dt->offset();
}

Variant c_datetime::t_gettimezone() {
  SmartObject<TimeZone> tz = m_dt->timezone();
  if (tz->isValid()) {
    return c_datetimezone::wrap(tz);
  }
  return false;
}

Object c_datetime::t_modify(CStrRef modify) {
  m_dt->modify(modify);
  return this;
}

Object c_datetime::t_setdate(int64 year, int64 month, int64 day) {
  m_dt->setDate(year, month, day);
  return this;
}

Object c_datetime::t_setisodate(int64 year, int64 week, int64 day /*= 1*/) {
  m_dt->setISODate(year, week, day);
  return this;
}

Object c_datetime::t_settime(int64 hour, int64 minute, int64 second /*= 0*/) {
  m_dt->setTime(hour, minute, second);
  return this;
}

Object c_datetime::t_settimezone(CObjRef timezone) {
  m_dt->setTimezone(c_datetimezone::unwrap(timezone));
  return this;
}

Variant c_datetime::t___destruct() {
  return null;
}

c_datetimezone::c_datetimezone() {
}

c_datetimezone::~c_datetimezone() {
}

void c_datetimezone::t___construct(CStrRef timezone) {
  m_tz = NEW(TimeZone)(timezone);
}

String c_datetimezone::t_getname() {
  return m_tz->name();
}

int64 c_datetimezone::t_getoffset(CObjRef datetime) {
  bool error;
  int64 ts = c_datetime::unwrap(datetime)->toTimeStamp(error);
  return m_tz->offset(ts);
}

Array c_datetimezone::t_gettransitions() {
  return m_tz->transitions();
}

Array c_datetimezone::ti_listabbreviations(const char* cls) {
  return TimeZone::GetAbbreviations();
}

Array c_datetimezone::ti_listidentifiers(const char* cls) {
  return TimeZone::GetNames();
}

Variant c_datetimezone::t___destruct() {
  return null;
}

///////////////////////////////////////////////////////////////////////////////
}
