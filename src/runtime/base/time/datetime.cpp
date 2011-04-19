/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/base/time/datetime.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/util/string_buffer.h>
#include <runtime/base/runtime_error.h>
#include <runtime/base/type_conversions.h>
#include <runtime/base/builtin_functions.h>

namespace HPHP {

IMPLEMENT_OBJECT_ALLOCATION_NO_DEFAULT_SWEEP(DateTime);
///////////////////////////////////////////////////////////////////////////////
// statics

StaticString DateTime::s_class_name("DateTime");

const char *DateTime::DateFormatRFC822     = "D, d M y H:i:s O";
const char *DateTime::DateFormatRFC850     = "l, d-M-y H:i:s T";
const char *DateTime::DateFormatRFC1036    = "D, d M y H:i:s O";
const char *DateTime::DateFormatRFC1123    = "D, d M Y H:i:s O";
const char *DateTime::DateFormatRFC2822    = "D, d M Y H:i:s O";
const char *DateTime::DateFormatRFC3339    = "Y-m-d\\TH:i:sP";
const char *DateTime::DateFormatISO8601    = "Y-m-d\\TH:i:sO";
const char *DateTime::DateFormatCookie     = "D, d-M-Y H:i:s T";
const char *DateTime::DateFormatHttpHeader = "D, d M Y H:i:s T";

const char *DateTime::MonthNames[] = {
  "January", "February", "March", "April", "May", "June",
  "July", "August", "September", "October", "November", "December"
};

const char *DateTime::ShortMonthNames[] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

const char *DateTime::WeekdayNames[] = {
  "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
};

const char *DateTime::ShortWeekdayNames[] = {
  "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

const char *DateTime::GetWeekdayName(int y, int m, int d) {
  int day_of_week = timelib_day_of_week(y, m, d);
  if (day_of_week < 0) {
    return "Unknown";
  }
  return WeekdayNames[day_of_week];
}

const char *DateTime::GetShortWeekdayName(int y, int m, int d) {
  int day_of_week = timelib_day_of_week(y, m, d);
  if (day_of_week < 0) {
    return "Unknown";
  }
  return ShortWeekdayNames[day_of_week];
}

const char *DateTime::OrdinalSuffix(int number) {
  if (number >= 10 && number <= 19) {
    return "th";
  }
  switch (number % 10) {
  case 1: return "st";
  case 2: return "nd";
  case 3: return "rd";
  }
  return "th";
}

bool DateTime::IsLeap(int year) {
  return timelib_is_leap(year);
}

int DateTime::DaysInMonth(int y, int m) {
  return timelib_days_in_month(y, m);
}

bool DateTime::IsValid(int y, int m, int d) {
  return y >= 1 && y <= 32767 && m >= 1 && m <= 12 && d >= 1 &&
    d <= timelib_days_in_month(y, m);
}

SmartObject<DateTime> DateTime::Current(bool utc /* = false */) {
  return NEWOBJ(DateTime)(time(0), utc);
}

#define PHP_DATE_PARSE_DATE_SET_TIME_ELEMENT(name, elem) \
  if ((int)parsed_time->elem == -99999) {                \
    ret.set(#name, false);                               \
  } else {                                               \
    ret.set(#name, (int)parsed_time->elem);              \
  }

Array DateTime::Parse(CStrRef datetime) {
  struct timelib_error_container *error;
  timelib_time *parsed_time =
    timelib_strtotime((char*)datetime.data(), datetime.size(), &error,
                      TimeZone::GetDatabase());

  Array ret;
  PHP_DATE_PARSE_DATE_SET_TIME_ELEMENT(year,      y);
  PHP_DATE_PARSE_DATE_SET_TIME_ELEMENT(month,     m);
  PHP_DATE_PARSE_DATE_SET_TIME_ELEMENT(day,       d);
  PHP_DATE_PARSE_DATE_SET_TIME_ELEMENT(hour,      h);
  PHP_DATE_PARSE_DATE_SET_TIME_ELEMENT(minute,    i);
  PHP_DATE_PARSE_DATE_SET_TIME_ELEMENT(second,    s);

  if (parsed_time->f == -99999) {
    ret.set("fraction", false);
  } else {
    ret.set("fraction", parsed_time->f);
  }

  {
    ret.set("warning_count", error->warning_count);
    Array element;
    for (int i = 0; i < error->warning_count; i++) {
      element.set(error->warning_messages[i].position,
                  String(error->warning_messages[i].message, CopyString));
    }
    ret.set("warnings", element);
  }
  {
    ret.set("error_count", error->error_count);
    Array element;
    for (int i = 0; i < error->error_count; i++) {
      element.set(error->error_messages[i].position,
                  String(error->error_messages[i].message, CopyString));
    }
    ret.set("errors", element);
  }
  timelib_error_container_dtor(error);

  ret.set("is_localtime", (bool)parsed_time->is_localtime);
  if (parsed_time->is_localtime) {
    PHP_DATE_PARSE_DATE_SET_TIME_ELEMENT(zone_type, zone_type);
    switch (parsed_time->zone_type) {
    case TIMELIB_ZONETYPE_OFFSET:
      PHP_DATE_PARSE_DATE_SET_TIME_ELEMENT(zone, z);
      ret.set("is_dst", (bool)parsed_time->dst);
      break;
    case TIMELIB_ZONETYPE_ID:
      if (parsed_time->tz_abbr) {
        ret.set("tz_abbr", String(parsed_time->tz_abbr, CopyString));
      }
      if (parsed_time->tz_info) {
        ret.set("tz_id", String(parsed_time->tz_info->name, CopyString));
      }
      break;
    case TIMELIB_ZONETYPE_ABBR:
      PHP_DATE_PARSE_DATE_SET_TIME_ELEMENT(zone, z);
      ret.set("is_dst", (bool)parsed_time->dst);
      ret.set("tz_abbr", String(parsed_time->tz_abbr, CopyString));
      break;
    }
  }

  {
    Array element;
    if (parsed_time->have_relative) {
      element.set("year",   parsed_time->relative.y);
      element.set("month",  parsed_time->relative.m);
      element.set("day",    parsed_time->relative.d);
      element.set("hour",   parsed_time->relative.h);
      element.set("minute", parsed_time->relative.i);
      element.set("second", parsed_time->relative.s);
#if defined(TIMELIB_VERSION)
      if (parsed_time->relative.have_weekday_relative) {
#else
      if (parsed_time->have_weekday_relative) {
#endif
        element.set("weekday", parsed_time->relative.weekday);
      }
      ret.set("relative", element);
    }
  }

  timelib_time_dtor(parsed_time);
  return ret;
}

Array DateTime::Parse(CStrRef ts, CStrRef format) {
  struct tm parsed_time;
  memset(&parsed_time, 0, sizeof(parsed_time));
  char *unparsed_part = strptime(ts.data(), format.data(), &parsed_time);
  if (unparsed_part == NULL) {
    return Array();
  }

  Array ret;
  ret.set("tm_sec",  parsed_time.tm_sec);
  ret.set("tm_min",  parsed_time.tm_min);
  ret.set("tm_hour", parsed_time.tm_hour);
  ret.set("tm_mday", parsed_time.tm_mday);
  ret.set("tm_mon",  parsed_time.tm_mon);
  ret.set("tm_year", parsed_time.tm_year);
  ret.set("tm_wday", parsed_time.tm_wday);
  ret.set("tm_yday", parsed_time.tm_yday);
  ret.set("unparsed", String(unparsed_part, CopyString));
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// constructors

DateTime::DateTime() : m_timestamp(-1), m_timestampSet(false) {
  m_time = TimePtr(timelib_time_ctor(), time_deleter());
  setTimezone(TimeZone::Current());
}

DateTime::DateTime(int64 timestamp, bool utc /* = false */) {
  fromTimeStamp(timestamp, utc);
}

void DateTime::fromTimeStamp(int64 timestamp, bool utc /* = false */) {
  m_timestamp = timestamp;
  m_timestampSet = true;

  timelib_time *t = timelib_time_ctor();
  if (utc) {
    timelib_unixtime2gmt(t, (timelib_sll)m_timestamp);
  } else {
    m_tz = TimeZone::Current();
    t->tz_info = m_tz->get();
    t->zone_type = TIMELIB_ZONETYPE_ID;
    timelib_unixtime2local(t, (timelib_sll)m_timestamp);
  }
  m_time = TimePtr(t, time_deleter());
}

void DateTime::sweep() {
  m_time.reset();
}

///////////////////////////////////////////////////////////////////////////////
// informational

int DateTime::beat() const {
  int retval = (((((long)m_time->sse)-(((long)m_time->sse) -
                                       ((((long)m_time->sse) % 86400) +
                                        3600))) * 10) / 864);
  while (retval < 0) {
    retval += 1000;
  }
  retval = retval % 1000;
  return retval;
}

int DateTime::dow() const {
  return timelib_day_of_week(year(), month(), day());
}

int DateTime::doy() const {
  return timelib_day_of_year(year(), month(), day());
}

int DateTime::isoWeek() const {
  timelib_sll iw, iy;
  timelib_isoweek_from_date(year(), month(), day(), &iw, &iy);
  return iw;
}

int DateTime::isoYear() const {
  timelib_sll iw, iy;
  timelib_isoweek_from_date(year(), month(), day(), &iw, &iy);
  return iy;
}

int DateTime::isoDow() const {
  return timelib_iso_day_of_week(year(), month(), day());
}

int DateTime::offset() const {
  if (local()) {
    switch (m_time->zone_type) {
    case TIMELIB_ZONETYPE_ABBR:
    case TIMELIB_ZONETYPE_OFFSET:
      return (m_time->z - (m_time->dst * 60)) * -60;
    default:
      {
        bool error;
        timelib_time_offset *offset =
          timelib_get_time_zone_info(toTimeStamp(error), m_tz->get());
        int ret = offset->offset;
        timelib_time_offset_dtor(offset);
        return ret;
      }
    }
  }
  return 0;
}

const char *DateTime::weekdayName() const {
  return GetWeekdayName(year(), month(), day());
}

const char *DateTime::shortWeekdayName() const {
  return GetShortWeekdayName(year(), month(), day());
}

const char *DateTime::monthName() const {
  return MonthNames[month() - 1];
}

const char *DateTime::shortMonthName() const {
  return ShortMonthNames[month() - 1];
}

///////////////////////////////////////////////////////////////////////////////
// modifications

void DateTime::update() {
  if (utc()) {
    timelib_update_ts(m_time.get(), NULL);
  } else {
    timelib_update_ts(m_time.get(), m_tz->get());
  }
  m_timestamp = 0;
  m_timestampSet = false;
}

void DateTime::set(int hou, int min, int sec, int mon, int day, int yea) {
  /* Fill in the new data */
  if (yea != INT_MAX) {
    if (yea < 70) {
      yea += 2000;
    } else if (yea >= 70 && yea <= 110) {
      yea += 1900;
    }
    m_time->y = yea;
  }
  if (day != INT_MAX) m_time->d = day;
  if (mon != INT_MAX) m_time->m = mon;
  if (sec != INT_MAX) m_time->s = sec;
  if (min != INT_MAX) m_time->i = min;
  if (hou != INT_MAX) m_time->h = hou;
  update();
}

void DateTime::setDate(int y, int m, int d) {
  m_time->y = y;
  m_time->m = m;
  m_time->d = d;
  update();
}

void DateTime::setISODate(int y, int w, int d /* = 1 */) {
  m_time->y = y;
  m_time->m = 1;
  m_time->d = 1;
  m_time->relative.d = timelib_daynr_from_weeknr(y, w, d);
  m_time->have_relative = 1;
  update();
}

void DateTime::setTime(int hour, int minute, int second) {
  m_time->h = hour;
  m_time->i = minute;
  m_time->s = second;
  update();
}

void DateTime::setTimezone(SmartObject<TimeZone> timezone) {
  if (!timezone.isNull()) {
    m_tz = timezone->cloneTimeZone();
    if (m_tz.get() && m_tz->get()) {
      timelib_set_timezone(m_time.get(), m_tz->get());
      timelib_unixtime2local(m_time.get(), m_time->sse);
    }
  }
}

void DateTime::modify(CStrRef diff) {
  timelib_time *tmp_time = timelib_strtotime((char*)diff.data(), diff.size(),
                                             NULL, TimeZone::GetDatabase());
  m_time->relative.y = tmp_time->relative.y;
  m_time->relative.m = tmp_time->relative.m;
  m_time->relative.d = tmp_time->relative.d;
  m_time->relative.h = tmp_time->relative.h;
  m_time->relative.i = tmp_time->relative.i;
  m_time->relative.s = tmp_time->relative.s;
  m_time->relative.weekday = tmp_time->relative.weekday;
  m_time->have_relative = tmp_time->have_relative;
#if defined(TIMELIB_VERSION)
  m_time->relative.have_weekday_relative =
    tmp_time->relative.have_weekday_relative;
#else
  m_time->have_weekday_relative = tmp_time->have_weekday_relative;
#endif
  m_time->sse_uptodate = 0;
  timelib_time_dtor(tmp_time);
  update();
  timelib_update_from_sse(m_time.get());
}

///////////////////////////////////////////////////////////////////////////////
// conversions

void DateTime::toTm(struct tm &ta) const {
  ta.tm_sec  = second();
  ta.tm_min  = minute();
  ta.tm_hour = hour();
  ta.tm_mday = day();
  ta.tm_mon  = month() - 1;
  ta.tm_year = year() - 1900;
  ta.tm_wday = dow();
  ta.tm_yday = doy();
  if (utc()) {
    ta.tm_isdst = 0;
    ta.tm_gmtoff = 0;
    ta.tm_zone = "GMT";
  } else {
    timelib_time_offset *offset =
      timelib_get_time_zone_info(m_time->sse, m_time->tz_info);
    ta.tm_isdst = offset->is_dst;
    ta.tm_gmtoff = offset->offset;
    ta.tm_zone = offset->abbr;
    timelib_time_offset_dtor(offset);
  }
}

int64 DateTime::toTimeStamp(bool &err) const {
  err = false;
  if (!m_timestampSet) {
    int error;
    m_timestamp = timelib_date_to_int(m_time.get(), &error);
    if (error) {
      err = true;
    } else {
      m_timestampSet = true;
    }
  }
  return m_timestamp;
}

int64 DateTime::toInteger(char format) const {
  bool error;
  switch (format) {
  case 'd':
  case 'j': return day();
  case 'w': return dow();
  case 'z': return doy();
  case 'W': return isoWeek();
  case 'm':
  case 'n': return month();
  case 't': return DaysInMonth(year(), month());
  case 'L': return DateTime::IsLeap(year());
  case 'y': return (year() % 100);
  case 'Y': return year();
  case 'B': return beat();
  case 'g':
  case 'h': return hour12();
  case 'H':
  case 'G': return hour();
  case 'i': return minute();
  case 's': return second();
  case 'I': return (!utc() && m_tz->dst(toTimeStamp(error))) ? 1 : 0;
  case 'Z': return utc() ? 0 : m_tz->offset(toTimeStamp(error));
  case 'U': return toTimeStamp(error);
  }
  throw_invalid_argument("unknown format char: %d", (int)format);
  return -1;
}

String DateTime::toString(CStrRef format, bool stdc /* = false */) const {
  if (format.empty()) return String();
  return stdc ? stdcFormat(format) : rfcFormat(format);
}

String DateTime::toString(DateFormat format) const {
  switch (format) {
  case RFC822:     return rfcFormat(DateFormatRFC822);
  case RFC850:     return rfcFormat(DateFormatRFC850);
  case RFC1036:    return rfcFormat(DateFormatRFC1036);
  case RFC1123:    return rfcFormat(DateFormatRFC1123);
  case RFC2822:    return rfcFormat(DateFormatRFC2822);
  case RFC3339:    return rfcFormat(DateFormatRFC3339);
  case ISO8601:    return rfcFormat(DateFormatISO8601);
  case Cookie:     return rfcFormat(DateFormatCookie);
  case HttpHeader: return rfcFormat(DateFormatHttpHeader);
  default:
    ASSERT(false);
  }
  throw_invalid_argument("format: %d", format);
  return String();
}

String DateTime::rfcFormat(CStrRef format) const {
  StringBuffer s;
  bool rfc_colon = false;
  bool error;
  for (int i = 0; i < format.size(); i++) {
    switch (format.charAt(i)) {
    case 'd': s.printf("%02d", day()); break;
    case 'D': s.append(shortWeekdayName()); break;
    case 'j': s.append(day()); break;
    case 'l': s.append(weekdayName()); break;
    case 'S': s.append(OrdinalSuffix(day())); break;
    case 'w': s.append(dow()); break;
    case 'N': s.append(isoDow()); break;
    case 'z': s.append(doy()); break;
    case 'W': s.printf("%02d", isoWeek()); break;
    case 'o': s.append(isoYear()); break;
    case 'F': s.append(monthName()); break;
    case 'm': s.printf("%02d", month()); break;
    case 'M': s.append(shortMonthName()); break;
    case 'n': s.append(month()); break;
    case 't': s.append(DaysInMonth(year(), month())); break;
    case 'L': s.append(IsLeap(year())); break;
    case 'y': s.printf("%02d", year() % 100); break;
    case 'Y': s.printf("%s%04d", year() < 0 ? "-" : "", abs(year()));
      break;
    case 'a': s.append(hour() >= 12 ? "pm" : "am"); break;
    case 'A': s.append(hour() >= 12 ? "PM" : "AM"); break;
    case 'B': s.printf("%03d", beat()); break;
    case 'g': s.append((hour() % 12) ? (int)hour() % 12 : 12); break;
    case 'G': s.append(hour()); break;
    case 'h': s.printf("%02d", (hour() % 12) ? (int)hour() % 12 : 12); break;
    case 'H': s.printf("%02d", (int)hour()); break;
    case 'i': s.printf("%02d", (int)minute()); break;
    case 's': s.printf("%02d", (int)second()); break;
    case 'u': s.printf("%06d", (int)floor(fraction() * 1000000)); break;
    case 'I': s.append(!utc() && m_tz->dst(toTimeStamp(error)) ? 1 : 0);
      break;
    case 'P': rfc_colon = true; /* break intentionally missing */
    case 'O':
      if (utc()) {
        s.printf("+0%s0", rfc_colon ? ":" : "");
      } else {
        int offset = m_tz->offset(toTimeStamp(error));
        s.printf("%c%02d%s%02d",
                 (offset < 0 ? '-' : '+'), abs(offset / 3600),
                 rfc_colon ? ":" : "", abs((offset % 3600) / 60));
      }
      break;
    case 'T': s.append(utc() ? "GMT" : m_time->tz_abbr); break;
    case 'e': s.append(utc() ? "UTC" : m_tz->name()); break;
    case 'Z': s.append(utc() ? 0 : m_tz->offset(toTimeStamp(error)));
      break;
    case 'c':
      if (utc()) {
        s.printf("%04d-%02d-%02dT%02d:%02d:%02d+0:0",
                 year(), month(), day(), hour(), minute(), second());
      } else {
        int offset = m_tz->offset(toTimeStamp(error));
        s.printf("%04d-%02d-%02dT%02d:%02d:%02d%c%02d:%02d",
                 year(), month(), day(), hour(), minute(), second(),
                 (offset < 0 ? '-' : '+'),
                 abs(offset / 3600), abs((offset % 3600) / 60));
      }
      break;
    case 'r':
      if (utc()) {
        s.printf("%3s, %02d %3s %04d %02d:%02d:%02d +00",
                 shortWeekdayName(), day(), shortMonthName(), year(),
                 hour(), minute(), second());
      } else {
        int offset = m_tz->offset(toTimeStamp(error));
        s.printf("%3s, %02d %3s %04d %02d:%02d:%02d %c%02d%02d",
                 shortWeekdayName(), day(), shortMonthName(), year(),
                 hour(), minute(), second(),
                 (offset < 0 ? '-' : '+'),
                 abs(offset / 3600), abs((offset % 3600) / 60));
      }
      break;
    case 'U': s.printf("%lld", toTimeStamp(error)); break;
    case '\\':
      if (i < format.size()) i++; /* break intentionally missing */
    default:
      s.append(format[i]);
      break;
    }
  }
  return s.detach();
}

String DateTime::stdcFormat(CStrRef format) const {
  struct tm ta;
  timelib_time_offset *offset = NULL;
  ta.tm_sec  = second();
  ta.tm_min  = minute();
  ta.tm_hour = hour();
  ta.tm_mday = day();
  ta.tm_mon  = month() - 1;
  ta.tm_year = year() - 1900;
  ta.tm_wday = dow();
  ta.tm_yday = doy();
  if (utc()) {
    ta.tm_isdst = 0;
    ta.tm_gmtoff = 0;
    ta.tm_zone = "GMT";
  } else {
    offset = timelib_get_time_zone_info(m_time->sse, m_time->tz_info);
    ta.tm_isdst = offset->is_dst;
    ta.tm_gmtoff = offset->offset;
    ta.tm_zone = offset->abbr;
  }

  int max_reallocs = 5;
  size_t buf_len = 256, real_len;
  char *buf = (char *)malloc(buf_len);
  while ((real_len = strftime(buf, buf_len, format.data(), &ta)) == buf_len ||
         real_len == 0) {
    buf_len *= 2;
    free(buf);
    buf = (char *)malloc(buf_len);
    if (!--max_reallocs) {
      break;
    }
  }
  if (!utc()) {
    timelib_time_offset_dtor(offset);
  }
  if (real_len && real_len != buf_len) {
    return String(buf, real_len, AttachString);
  }
  free(buf);
  throw_invalid_argument("format: (over internal buffer)");
  return String();
}

Array DateTime::toArray(ArrayFormat format) const {
  Array ret;
  bool error;
  switch (format) {
  case TimeMap:
    ret.set("seconds", second());
    ret.set("minutes", minute());
    ret.set("hours",   hour());
    ret.set("mday",    day());
    ret.set("wday",    dow());
    ret.set("mon",     month());
    ret.set("year",    year());
    ret.set("yday",    doy());
    ret.set("weekday", weekdayName());
    ret.set("month",   monthName());
    ret.set(0,         toTimeStamp(error));
    break;
  case TmMap:
    {
      struct tm tm;
      toTm(tm);
      ret.set("tm_sec",   tm.tm_sec);
      ret.set("tm_min",   tm.tm_min);
      ret.set("tm_hour",  tm.tm_hour);
      ret.set("tm_mday",  tm.tm_mday);
      ret.set("tm_mon",   tm.tm_mon);
      ret.set("tm_year",  tm.tm_year);
      ret.set("tm_wday",  tm.tm_wday);
      ret.set("tm_yday",  tm.tm_yday);
      ret.set("tm_isdst", tm.tm_isdst);
    }
    break;
  case TmVector:
    {
      struct tm tm;
      toTm(tm);
      ret.append(tm.tm_sec);
      ret.append(tm.tm_min);
      ret.append(tm.tm_hour);
      ret.append(tm.tm_mday);
      ret.append(tm.tm_mon);
      ret.append(tm.tm_year);
      ret.append(tm.tm_wday);
      ret.append(tm.tm_yday);
      ret.append(tm.tm_isdst);
    }
    break;
  }
  return ret;
}

bool DateTime::fromString(CStrRef input, SmartObject<TimeZone> tz) {
  struct timelib_error_container *error;
  timelib_time *t = timelib_strtotime((char*)input.data(), input.size(),
                                      &error, TimeZone::GetDatabase());
  int error1 = error->error_count;
  timelib_error_container_dtor(error);

  if (m_timestamp == -1) {
    fromTimeStamp(0);
  }
  if (tz.get()) {
    setTimezone(tz);
  } else {
    setTimezone(TimeZone::Current());
  }

  // needed if any date part is missing
  timelib_fill_holes(t, m_time.get(), 0);
  timelib_update_ts(t, m_tz->get());

  int error2;
  m_timestamp = timelib_date_to_int(t, &error2);
  if (error1 || error2) {
    timelib_tzinfo_dtor(t->tz_info);
    timelib_time_dtor(t);
    return false;
  }

  m_time = TimePtr(t, time_deleter());
  m_tz = NEWOBJ(TimeZone)(t->tz_info);
  return true;
}

SmartObject<DateTime> DateTime::cloneDateTime() const {
  bool err;
  SmartObject<DateTime> ret(NEWOBJ(DateTime)(toTimeStamp(err), true));
  ret->setTimezone(m_tz);
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// sun

Array DateTime::getSunInfo(double latitude, double longitude) const {
  Array ret;
  timelib_sll sunrise, sunset, transit;
  double ddummy;

  /* Get sun up/down and transit */
  int rs = timelib_astro_rise_set_altitude(m_time.get(), longitude, latitude,
                                           -35.0/60, 1, &ddummy, &ddummy,
                                           &sunrise, &sunset, &transit);
  switch (rs) {
  case -1: /* always below */
    ret.set("sunrise", false);
    ret.set("sunset", false);
    break;
  case 1: /* always above */
    ret.set("sunrise", true);
    ret.set("sunset", true);
    break;
  default:
    ret.set("sunrise", sunrise);
    ret.set("sunset", sunset);
  }
  ret.set("transit", transit);

  /* Get civil twilight */
  rs = timelib_astro_rise_set_altitude(m_time.get(), longitude, latitude,
                                       -6.0, 0,
                                       &ddummy, &ddummy, &sunrise, &sunset,
                                       &transit);
  switch (rs) {
  case -1: /* always below */
    ret.set("civil_twilight_begin", false);
    ret.set("civil_twilight_end", false);
    break;
  case 1: /* always above */
    ret.set("civil_twilight_begin", true);
    ret.set("civil_twilight_end", true);
    break;
  default:
    ret.set("civil_twilight_begin", sunrise);
    ret.set("civil_twilight_end", sunset);
  }

  /* Get nautical twilight */
  rs = timelib_astro_rise_set_altitude(m_time.get(), longitude, latitude,
                                       -12.0, 0,
                                       &ddummy, &ddummy, &sunrise, &sunset,
                                       &transit);
  switch (rs) {
  case -1: /* always below */
    ret.set("nautical_twilight_begin", false);
    ret.set("nautical_twilight_end", false);
    break;
  case 1: /* always above */
    ret.set("nautical_twilight_begin", true);
    ret.set("nautical_twilight_end", true);
    break;
  default:
    ret.set("nautical_twilight_begin", sunrise);
    ret.set("nautical_twilight_end", sunset);
  }

  /* Get astronomical twilight */
  rs = timelib_astro_rise_set_altitude(m_time.get(), longitude, latitude,
                                       -18.0, 0,
                                       &ddummy, &ddummy, &sunrise, &sunset,
                                       &transit);
  switch (rs) {
  case -1: /* always below */
    ret.set("astronomical_twilight_begin", false);
    ret.set("astronomical_twilight_end", false);
    break;
  case 1: /* always above */
    ret.set("astronomical_twilight_begin", true);
    ret.set("astronomical_twilight_end", true);
    break;
  default:
    ret.set("astronomical_twilight_begin", sunrise);
    ret.set("astronomical_twilight_end", sunset);
  }
  return ret;
}

Variant DateTime::getSunInfo(SunInfoFormat retformat,
                             double latitude, double longitude,
                             double zenith, double utc_offset,
                             bool calc_sunset) const {
  if (retformat != ReturnTimeStamp &&
      retformat != ReturnString &&
      retformat != ReturnDouble) {
    raise_warning("Wrong return format given, pick one of "
                    "SUNFUNCS_RET_TIMESTAMP, SUNFUNCS_RET_STRING or "
                    "SUNFUNCS_RET_DOUBLE");
    return false;
  }
  bool error;
  double altitude = 90 - zenith;
  if (utc_offset == -99999.0) {
    if (utc()) {
      utc_offset = 0;
    } else {
      utc_offset = m_tz->offset(toTimeStamp(error)) / 3600;
    }
  }

  double h_rise, h_set; timelib_sll sunrise, sunset, transit;
  int rs = timelib_astro_rise_set_altitude(m_time.get(), longitude, latitude,
                                           altitude, altitude > -1 ? 1 : 0,
                                           &h_rise, &h_set, &sunrise, &sunset,
                                           &transit);
  if (rs != 0) {
    return false;
  }

  if (retformat == ReturnTimeStamp) {
    return calc_sunset ? sunset : sunrise;
  }

  double N = (calc_sunset ? h_set : h_rise) + utc_offset;
  if (N > 24 || N < 0) {
    N -= floor(N / 24) * 24;
  }

  if (retformat == ReturnString) {
    char retstr[6];
    snprintf(retstr, sizeof(retstr),
             "%02d:%02d", (int) N, (int) (60 * (N - (int) N)));
    return String(retstr, CopyString);
  }

  ASSERT(retformat == ReturnDouble);
  return N;
}

///////////////////////////////////////////////////////////////////////////////
}
