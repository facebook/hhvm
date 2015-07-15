/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include <folly/AtomicHashArray.h>

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/datetime.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/type-conversions.h"

#include "hphp/util/functional.h"
#include "hphp/util/logger.h"
#include "hphp/util/text-util.h"

namespace HPHP {

IMPLEMENT_RESOURCE_ALLOCATION(TimeZone)
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

    string_printf(m_warning, DATE_TZ_ERRMSG
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
};
static IMPLEMENT_THREAD_LOCAL(TimeZoneData, s_timezone_data);

struct ahm_eqstr {
  bool operator()(const char* a, const char* b) {
    return intptr_t(a) > 0 && (strcmp(a, b) == 0);
  }
};

using TimeZoneCache =
  folly::AtomicHashArray<const char*, timelib_tzinfo*, cstr_hash, ahm_eqstr>;
using TimeZoneCacheEntry = std::pair<const char*, timelib_tzinfo*>;

TimeZoneCache* s_tzCache;

using TimeZoneValidityCache =
  folly::AtomicHashArray<const char*, bool, cstr_hash, ahm_eqstr>;
using TimeZoneValidityCacheEntry = std::pair<const char*, bool>;

TimeZoneValidityCache* s_tzvCache;

void timezone_init() {
  // Allocate enough space to cache all possible timezones, if needed.
  constexpr size_t kMaxTimeZoneCache = 1000;
  s_tzCache = TimeZoneCache::create(kMaxTimeZoneCache).release();
  s_tzvCache = TimeZoneValidityCache::create(kMaxTimeZoneCache).release();
}

const timelib_tzdb *TimeZone::GetDatabase() {
  const timelib_tzdb *&Database = s_timezone_data->Database;
  if (Database == nullptr) {
    Database = timelib_builtin_db();
  }
  return Database;
}

timelib_tzinfo* TimeZone::GetTimeZoneInfoRaw(char* name,
                                             const timelib_tzdb* db) {
  auto const it = s_tzCache->find(name);
  if (it != s_tzCache->end()) {
    return it->second;
  }

  auto tzi = timelib_parse_tzfile(name, db);
  if (!tzi) {
    char* tzid = timelib_timezone_id_from_abbr(name, -1, 0);
    if (tzid) {
      tzi = timelib_parse_tzfile(tzid, db);
    }
  }

  if (tzi) {
    auto key = strdup(name);
    auto result = s_tzCache->insert(TimeZoneCacheEntry(key, tzi));
    if (!result.second) {
      // The cache should never fill up since tzinfos are finite.
      always_assert(result.first != s_tzCache->end());
      // A collision occurred, so we don't need our strdup'ed key.
      free(key);
      timelib_tzinfo_dtor(tzi);
      tzi = result.first->second;
    }
  }

  return tzi;
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

req::ptr<TimeZone> TimeZone::Current() {
  return req::make<TimeZone>(CurrentName());
}

bool TimeZone::SetCurrent(const String& zone) {
  bool valid;
  const char* name = zone.data();
  auto const it = s_tzvCache->find(name);
  if (it != s_tzvCache->end()) {
    valid = it->second;
  } else {
    valid = IsValid(zone);

    auto key = strdup(name);
    auto result = s_tzvCache->insert(TimeZoneValidityCacheEntry(key, valid));
    if (!result.second) {
      // The cache is full or a collision occurred, and we don't need our
      // strdup'ed key.
      free(key);
    }
  }

  if (!valid) {
    raise_notice("Timezone ID '%s' is invalid", zone.data());
    return false;
  }
  g_context->setTimeZone(zone);
  return true;
}

Array TimeZone::GetNamesToCountryCodes() {
  const timelib_tzdb *tzdb = timelib_builtin_db();
  int item_count = tzdb->index_size;
  const timelib_tzdb_index_entry *table = tzdb->index;

  Array ret;
  for (int i = 0; i < item_count; ++i) {
    // This string is what PHP considers as "data" or "info" which is basically
    // the string of "PHP1xx" where xx is country code that uses this timezone.
    // When country code is unknown or not in use anymore, ?? is used instead.
    // There is no known better way to extract this information out.
    const char* infoString = (const char*)&tzdb->data[table[i].pos];
    const char* countryCode = &infoString[5];

    ret.set(String(table[i].id, CopyString), String(countryCode, CopyString));
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
    ArrayInit element(3, ArrayInit::Map{});
    element.set(s_dst, (bool)entry->type);
    element.set(s_offset, entry->gmtoffset);
    if (entry->full_tz_name) {
      element.set(s_timezone_id, String(entry->full_tz_name, CopyString));
    } else {
      element.set(s_timezone_id, uninit_null());
    }
    auto& val = ret.lvalAt(String(entry->name));
    forceToArray(val).append(element.toArray());
  }
  return ret;
}

String TimeZone::AbbreviationToName(String abbr, int utcoffset /* = -1 */,
                                    int isdst /* = 1 */) {
  if (isdst != 0 && isdst != 1) {
    isdst = -1;
  }
  return String(timelib_timezone_id_from_abbr(abbr.data(), utcoffset,
                                              isdst),
                CopyString);
}

///////////////////////////////////////////////////////////////////////////////
// class TimeZone

TimeZone::TimeZone() {
  m_tzi = nullptr;
}

TimeZone::TimeZone(const String& name) {
  m_tzi = GetTimeZoneInfoRaw((char*)name.data(), GetDatabase());
}

TimeZone::TimeZone(timelib_tzinfo *tzi) {
  m_tzi = tzi;
}

req::ptr<TimeZone> TimeZone::cloneTimeZone() const {
  return req::make<TimeZone>(m_tzi);
}

String TimeZone::name() const {
  if (!m_tzi) return String();
  return String(m_tzi->name, CopyString);
}

String TimeZone::abbr(int type /* = 0 */) const {
  if (!m_tzi) return String();
  return String(&m_tzi->timezone_abbr[m_tzi->type[type].abbr_idx], CopyString);
}

int TimeZone::offset(int64_t timestamp) const {
  if (!m_tzi) return 0;

  timelib_time_offset *offset =
    timelib_get_time_zone_info(timestamp, m_tzi);
  int ret = offset->offset;
  timelib_time_offset_dtor(offset);
  return ret;
}

bool TimeZone::dst(int64_t timestamp) const {
  if (!m_tzi) return false;

  timelib_time_offset *offset =
    timelib_get_time_zone_info(timestamp, m_tzi);
  bool ret = offset->is_dst;
  timelib_time_offset_dtor(offset);
  return ret;
}

Array TimeZone::transitions(int64_t timestamp_begin, /* = k_PHP_INT_MIN */
                            int64_t timestamp_end /* = k_PHP_INT_MAX */) const {
  Array ret;
  if (m_tzi) {
    // If explicitly provided add the beginning timestamp to the ret array
    if (timestamp_begin > k_PHP_INT_MIN) {
      auto dt = req::make<DateTime>(timestamp_begin);
      ret.append(make_map_array(
            s_ts, timestamp_begin,
            s_time, dt->toString(DateTime::DateFormat::ISO8601),
            s_offset, m_tzi->type[0].offset,
            s_isdst, (bool)m_tzi->type[0].isdst,
            s_abbr, String(m_tzi->timezone_abbr + m_tzi->type[0].abbr_idx,
                           CopyString)
          ));
    }
    for (unsigned int i = 0; i < m_tzi->timecnt; ++i) {
      int timestamp = m_tzi->trans[i];
      if (timestamp > timestamp_begin && timestamp <= timestamp_end) {
        int index = m_tzi->trans_idx[i];
        auto dt = req::make<DateTime>(timestamp);
        ttinfo &offset = m_tzi->type[index];
        const char *abbr = m_tzi->timezone_abbr + offset.abbr_idx;

        ret.append(make_map_array(
          s_ts, timestamp,
          s_time, dt->toString(DateTime::DateFormat::ISO8601),
          s_offset, offset.offset,
          s_isdst, (bool)offset.isdst,
          s_abbr, String(abbr, CopyString)
        ));
      }
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
  throw_not_implemented("timelib version too old");
#endif

  return ret;
}

///////////////////////////////////////////////////////////////////////////////
}
