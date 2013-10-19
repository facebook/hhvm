/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/timezone.h"
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/datetime.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/util/logger.h"

namespace HPHP {

IMPLEMENT_OBJECT_ALLOCATION(TimeZone)
///////////////////////////////////////////////////////////////////////////////

class GuessedTimeZone {
public:
  std::string m_tzid;
  std::string m_warning;

  GuessedTimeZone() {
    time_t the_time = time(0);
    struct tm tmbuf;
    struct tm *ta = localtime_r(&the_time, &tmbuf);
    const char *tzid = nullptr;
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
  TimeZoneData() : Database(nullptr) {}

  const timelib_tzdb *Database;
  MapStringToTimeZoneInfo Cache;
};
static IMPLEMENT_THREAD_LOCAL(TimeZoneData, s_timezone_data);

const timelib_tzdb *TimeZone::GetDatabase() {
  const timelib_tzdb *&Database = s_timezone_data->Database;
  if (Database == nullptr) {
    Database = timelib_builtin_db();
  }
  return Database;
}

TimeZoneInfo TimeZone::GetTimeZoneInfo(char* name, const timelib_tzdb* db) {
  MapStringToTimeZoneInfo &Cache = s_timezone_data->Cache;

  MapStringToTimeZoneInfo::const_iterator iter = Cache.find(name);
  if (iter != Cache.end()) {
    return iter->second;
  }

  TimeZoneInfo tzi(timelib_parse_tzfile(name, db), tzinfo_deleter());
  if (tzi) {
    Cache[name] = tzi;
  }
  return tzi;
}

timelib_tzinfo* TimeZone::GetTimeZoneInfoRaw(char* name,
                                             const timelib_tzdb* db) {
  return GetTimeZoneInfo(name, db).get();
}

bool TimeZone::IsValid(const String& name) {
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

SmartResource<TimeZone> TimeZone::Current() {
  return NEWOBJ(TimeZone)(CurrentName());
}

bool TimeZone::SetCurrent(const String& zone) {
  if (!IsValid(zone)) {
    raise_notice("Timezone ID '%s' is invalid", zone.data());
    return false;
  }
  g_context->setTimeZone(zone);
  return true;
}

Array TimeZone::GetNames() {
  const timelib_tzdb *tzdb = timelib_builtin_db();
  int item_count = tzdb->index_size;
  const timelib_tzdb_index_entry *table = tzdb->index;

  Array ret;
  for (int i = 0; i < item_count; ++i) {
    ret.append(String(table[i].id, CopyString));
  }
  return ret;
}

const StaticString
  s_dst("dst"),
  s_offset("offset"),
  s_timezone_id("timezone_id"),
  s_ts("ts"),
  s_time("time"),
  s_isdst("isdst"),
  s_abbr("abbr"),
  s_country_code("country_code"),
  s_latitude("latitude"),
  s_longitude("longitude"),
  s_comments("comments");

Array TimeZone::GetAbbreviations() {
  Array ret;
  for (const timelib_tz_lookup_table *entry =
         timelib_timezone_abbreviations_list(); entry->name; entry++) {
    ArrayInit element(3);
    element.set(s_dst, (bool)entry->type);
    element.set(s_offset, entry->gmtoffset);
    if (entry->full_tz_name) {
      element.set(s_timezone_id, String(entry->full_tz_name, CopyString));
    } else {
      element.set(s_timezone_id, uninit_null());
    }
    ret.lvalAt(String(entry->name)).append(element.create());
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

TimeZone::TimeZone(const String& name) {
  m_tzi = GetTimeZoneInfo((char*)name.data(), GetDatabase());
}

TimeZone::TimeZone(timelib_tzinfo *tzi) {
  m_tzi = TimeZoneInfo(tzi, tzinfo_deleter());
}

SmartResource<TimeZone> TimeZone::cloneTimeZone() const {
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

Array TimeZone::transitions() const {
  Array ret;
  if (m_tzi) {
    for (unsigned int i = 0; i < m_tzi->timecnt; ++i) {
      int index = m_tzi->trans_idx[i];
      int timestamp = m_tzi->trans[i];
      DateTime dt(timestamp);
      ttinfo &offset = m_tzi->type[index];
      const char *abbr = m_tzi->timezone_abbr + offset.abbr_idx;

      ArrayInit element(5);
      element.set(s_ts, timestamp);
      element.set(s_time, dt.toString(DateTime::DateFormat::ISO8601));
      element.set(s_offset, offset.offset);
      element.set(s_isdst, (bool)offset.isdst);
      element.set(s_abbr, String(abbr, CopyString));
      ret.append(element.create());
    }
  }
  return ret;
}

Array TimeZone::getLocation() const {
  Array ret;
  if (!m_tzi) return ret;

#ifdef TIMELIB_HAVE_TZLOCATION
  ret.set(s_country_code, String(m_tzi->location.country_code, CopyString));
  ret.set(s_latitude,     m_tzi->location.latitude);
  ret.set(s_longitude,    m_tzi->location.longitude);
  ret.set(s_comments,     String(m_tzi->location.comments, CopyString));
#else
  throw NotImplementedException("timelib version too old");
#endif

  return ret;
}

///////////////////////////////////////////////////////////////////////////////
}
