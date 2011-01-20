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

#include <runtime/base/time/timezone.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/time/datetime.h>
#include <runtime/base/execution_context.h>
#include <runtime/base/type_conversions.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/runtime_error.h>
#include <runtime/base/array/array_init.h>
#include <util/logger.h>

namespace HPHP {

IMPLEMENT_OBJECT_ALLOCATION(TimeZone)
///////////////////////////////////////////////////////////////////////////////

StaticString TimeZone::s_class_name("TimeZone");

///////////////////////////////////////////////////////////////////////////////

class GuessedTimeZone {
public:
  std::string m_tzid;
  std::string m_warning;

  GuessedTimeZone() {
    time_t the_time = time(0);
    struct tm tmbuf;
    struct tm *ta = localtime_r(&the_time, &tmbuf);
    const char *tzid = NULL;
    if (ta) {
      tzid = timelib_timezone_id_from_abbr(ta->tm_zone, ta->tm_gmtoff,
                                           ta->tm_isdst);
    }
    if (!tzid) {
      tzid = "UTC";
    }
    m_tzid = tzid;

#define DATE_TZ_ERRMSG \
  "It is not safe to rely on the system's timezone settings. Please use " \
  "the date.timezone setting, the TZ environment variable or the " \
  "date_default_timezone_set() function. In case you used any of those " \
  "methods and you are still getting this warning, you most likely " \
  "misspelled the timezone identifier. "

    Util::string_printf(m_warning, DATE_TZ_ERRMSG
                        "We selected '%s' for '%s/%.1f/%s' instead",
                        tzid, ta ? ta->tm_zone : "Unknown",
                        ta ? (float) (ta->tm_gmtoff / 3600) : 0,
                        ta ? (ta->tm_isdst ? "DST" : "no DST") : "Unknown");
  }
};
static GuessedTimeZone s_guessed_timezone;

///////////////////////////////////////////////////////////////////////////////
// statics

class TimeZoneData {
public:
  TimeZoneData() : Database(NULL) {}

  const timelib_tzdb *Database;
  MapStringToTimeZoneInfo Cache;
};
static IMPLEMENT_THREAD_LOCAL(TimeZoneData, s_timezone_data);

const timelib_tzdb *TimeZone::GetDatabase() {
  const timelib_tzdb *&Database = s_timezone_data->Database;
  if (Database == NULL) {
    Database = timelib_builtin_db();
  }
  return Database;
}

TimeZoneInfo TimeZone::GetTimeZoneInfo(CStrRef name) {
  MapStringToTimeZoneInfo &Cache = s_timezone_data->Cache;

  MapStringToTimeZoneInfo::const_iterator iter = Cache.find(name.data());
  if (iter != Cache.end()) {
    return iter->second;
  }

  TimeZoneInfo tzi(timelib_parse_tzfile((char *)name.data(), GetDatabase()),
                   tzinfo_deleter());
  if (tzi) {
    Cache[name.data()] = tzi;
  }
  return tzi;
}

bool TimeZone::IsValid(CStrRef name) {
  return timelib_timezone_id_is_valid((char*)name.data(), GetDatabase());
}

String TimeZone::CurrentName() {
  /* Checking configure timezone */
  String timezone = g_context->getTimeZone();
  if (!timezone.empty()) {
    return timezone;
  }

  /* Check environment variable */
  char *env = getenv("TZ");
  if (env && *env && IsValid(env)) {
    return String(env, CopyString);
  }

  /* Check config setting for default timezone */
  String default_timezone = g_context->getDefaultTimeZone();
  if (!default_timezone.empty() && IsValid(default_timezone.data())) {
    return default_timezone;
  }

  /* Try to guess timezone from system information */
  raise_strict_warning(s_guessed_timezone.m_warning);
  return String(s_guessed_timezone.m_tzid);
}

SmartObject<TimeZone> TimeZone::Current() {
  return NEWOBJ(TimeZone)(CurrentName());
}

bool TimeZone::SetCurrent(CStrRef zone) {
  if (!IsValid(zone)) {
    raise_notice("Timezone ID '%s' is invalid", zone.data());
    return false;
  }
  g_context->setTimeZone(zone);
  return true;
}

Array TimeZone::GetNames(int64 what, CStrRef country) {
  Array ret;
  if (what == PER_COUNTRY && country.size() != 2) {
    raise_notice("A two-letter ISO 3166-1 compatible country code is expected");
    return ret;
  }

  const timelib_tzdb *tzdb = timelib_builtin_db();
  int item_count = tzdb->index_size;
  const timelib_tzdb_index_entry *table = tzdb->index;

  for (int i = 0; i < item_count; ++i) {
    if (what == PER_COUNTRY) {
      if (tzdb->data[table[i].pos + 5] == country.charAt(0)
          && tzdb->data[table[i].pos + 6] == country.charAt(1)) {
        ret.append(String(table[i].id, AttachLiteral));
      }
    } else if (what == ALL_WITH_BC ||
               checkIdAllowed(table[i].id, what) &&
               tzdb->data[table[i].pos + 4] == '\1') {
      ret.append(String(table[i].id, AttachLiteral));
    }
  }
  return ret;
}

#define CHECK_ALLOWED(c, s) \
  if ((what & c) && strncasecmp(id, s "/", sizeof(s)) == 0) return true

bool TimeZone::checkIdAllowed(char *id, int64 what) {
  CHECK_ALLOWED(AFRICA,     "Africa"    );
  CHECK_ALLOWED(AMERICA,    "America"   );
  CHECK_ALLOWED(ANTARCTICA, "Antarctica");
  CHECK_ALLOWED(ARCTIC,     "Arctic"    );
  CHECK_ALLOWED(ASIA,       "Asia"      );
  CHECK_ALLOWED(ATLANTIC,   "Atlantic"  );
  CHECK_ALLOWED(AUSTRALIA,  "Australia" );
  CHECK_ALLOWED(EUROPE,     "Europe"    );
  CHECK_ALLOWED(INDIAN,     "Indian"    );
  CHECK_ALLOWED(PACIFIC,    "Pacific"   );
  CHECK_ALLOWED(UTC,        "UTC"       );
  return false;
}

Array TimeZone::GetAbbreviations() {
  Array ret;
  for (const timelib_tz_lookup_table *entry =
         timelib_timezone_abbreviations_list(); entry->name; entry++) {
    Array element;
    element.set("dst", (bool)entry->type);
    element.set("offset", entry->gmtoffset);
    if (entry->full_tz_name) {
      element.set("timezone_id", String(entry->full_tz_name, AttachLiteral));
    } else {
      element.set("timezone_id", null);
    }

    ret.lvalAt(entry->name).append(element);
  }
  return ret;
}

String TimeZone::AbbreviationToName(String abbr, int utcoffset /* = -1 */,
                                    bool isdst /* = true */) {
  return String(timelib_timezone_id_from_abbr(abbr.data(), utcoffset,
                                              isdst ? -1 : 0),
                CopyString);
}

///////////////////////////////////////////////////////////////////////////////
// class TimeZone

TimeZone::TimeZone() {
  m_tzi = TimeZoneInfo();
}

TimeZone::TimeZone(CStrRef name) {
  m_tzi = GetTimeZoneInfo(name);
}

TimeZone::TimeZone(timelib_tzinfo *tzi) {
  m_tzi = TimeZoneInfo(tzi, tzinfo_deleter());
}

SmartObject<TimeZone> TimeZone::cloneTimeZone() const {
  if (!m_tzi) return NEWOBJ(TimeZone)();
  return NEWOBJ(TimeZone)(timelib_tzinfo_clone(m_tzi.get()));
}

String TimeZone::name() const {
  if (!m_tzi) return String();
  return String(m_tzi->name, CopyString);
}

String TimeZone::abbr(int type /* = 0 */) const {
  if (!m_tzi) return String();
  return String(&m_tzi->timezone_abbr[m_tzi->type[type].abbr_idx], CopyString);
}

int TimeZone::offset(int timestamp) const {
  if (!m_tzi) return 0;

  timelib_time_offset *offset =
    timelib_get_time_zone_info(timestamp, m_tzi.get());
  int ret = offset->offset;
  timelib_time_offset_dtor(offset);
  return ret;
}

bool TimeZone::dst(int timestamp) const {
  if (!m_tzi) return false;

  timelib_time_offset *offset =
    timelib_get_time_zone_info(timestamp, m_tzi.get());
  bool ret = offset->is_dst;
  timelib_time_offset_dtor(offset);
  return ret;
}

void TimeZone::addTransition(Array &array, int i, int64 timestamp) const {
  int index = (i >= 0 ? m_tzi->trans_idx[i] : 0);
  DateTime dt(timestamp);
  ttinfo &offset = m_tzi->type[index];
  const char *abbr = m_tzi->timezone_abbr + offset.abbr_idx;

  Array element;
  element.set("ts", timestamp);
  element.set("time", dt.toString(DateTime::ISO8601));
  element.set("offset", offset.offset);
  element.set("isdst", (bool)offset.isdst);
  element.set("abbr", String(abbr, CopyString));

  array.append(element);
}

Array TimeZone::transitions(int64 timestamp_begin, int64 timestamp_end) const {
  Array ret;
  if (m_tzi) {
    int i = 0;
    bool found = false;
    if (timestamp_begin == LLONG_MIN) {
      addTransition(ret, -1, timestamp_begin);
      found = true;
    } else {
      if (m_tzi->timecnt > 0) {
        do {
          if (m_tzi->trans[i] > timestamp_begin) {
            if (i > 0) {
              addTransition(ret, i - 1, timestamp_begin);
            } else {
              addTransition(ret, -1, timestamp_begin);
            }
            found = true;
            break;
          }
        } while (++i < (int)m_tzi->timecnt);
      }
    }

    if (!found) {
      if (m_tzi->timecnt > 0) {
        addTransition(ret, m_tzi->timecnt - 1, timestamp_begin);
      } else {
        addTransition(ret, -1, timestamp_begin);
      }
    } else {
      for (; i < (int)m_tzi->timecnt && m_tzi->trans[i] < timestamp_end; ++i) {
        addTransition(ret, i, m_tzi->trans[i]);
      }
    }
  }
  return ret;
}

Array TimeZone::getLocation() const {
  Array ret;
  if (m_tzi) {
    ret.set("country_code", String(m_tzi->location.country_code, CopyString));
    ret.set("latitude",     m_tzi->location.latitude);
    ret.set("longitude",    m_tzi->location.longitude);
    ret.set("comments",     String(m_tzi->location.comments, CopyString));
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
}
