/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/util/lock.h"
#include "hphp/util/text-util.h"

namespace HPHP {

IMPLEMENT_RESOURCE_ALLOCATION(TimeZone)
///////////////////////////////////////////////////////////////////////////////

class GuessedTimeZone {
public:
  std::string m_tzid;

  GuessedTimeZone() {
    time_t the_time = time(0);
    struct tm tmbuf;
    struct tm *ta = localtime_r(&the_time, &tmbuf);
    const char *tzid = nullptr;
#ifndef _MSC_VER
    // TODO: Fixme under MSVC!
    if (ta) {
      tzid = timelib_timezone_id_from_abbr(ta->tm_zone, ta->tm_gmtoff,
                                           ta->tm_isdst);
    }
#endif
    if (!tzid) {
      tzid = "UTC";
    }
    m_tzid = tzid;
  }
};
static GuessedTimeZone s_guessed_timezone;
static Mutex s_tzdb_mutex;
static std::atomic<const timelib_tzdb*> s_tzdb_cache { nullptr };

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

const timelib_tzdb* timezone_get_builtin_tzdb() {
  if (s_tzdb_cache != nullptr) return s_tzdb_cache;

  Lock tzdbLock(s_tzdb_mutex);
  if (s_tzdb_cache == nullptr) s_tzdb_cache = timelib_builtin_db();
  return s_tzdb_cache;
}

const timelib_tzdb *TimeZone::GetDatabase() {
  const timelib_tzdb *&Database = s_timezone_data->Database;
  if (Database == nullptr) {
    Database = timezone_get_builtin_tzdb();
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

bool TimeZone::IsValid(const char* name) {
  return timelib_timezone_id_is_valid((char*)name, GetDatabase());
}

String TimeZone::CurrentName() {
  /* Checking configure timezone */
  auto& tz = RID().getTimeZone();
  if (!tz.empty()) {
    return String(tz);
  }

  /* Check environment variable */
  char *env = getenv("TZ");
  if (env && *env && IsValid(env)) {
    return String(env, CopyString);
  }

  return String(s_guessed_timezone.m_tzid);
}

req::ptr<TimeZone> TimeZone::Current() {
  return req::make<TimeZone>(CurrentName());
}

bool TimeZone::SetCurrent(const char* name) {
  bool valid;
  auto const it = s_tzvCache->find(name);
  if (it != s_tzvCache->end()) {
    valid = it->second;
  } else {
    valid = IsValid(name);

    auto key = strdup(name);
    auto result = s_tzvCache->insert(TimeZoneValidityCacheEntry(key, valid));
    if (!result.second) {
      // The cache is full or a collision occurred, and we don't need our
      // strdup'ed key.
      free(key);
    }
  }

  if (!valid) {
    raise_notice("Timezone ID '%s' is invalid", name);
    return false;
  }
  RID().setTimeZone(name);
  return true;
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
    uint32_t timecnt;
#ifdef FACEBOOK
    // Internal builds embed tzdata into HHVM by keeping timelib updated
    timecnt = m_tzi->bit32.timecnt;
#else
    // OSS builds read tzdata from the system location (eg /usr/share/zoneinfo)
    // as this format must be stable, we're keeping timelib stable too
    timecnt = m_tzi->timecnt;
#endif
    uint32_t lastBefore = 0;
    for (uint32_t i = 0;
         i < timecnt && m_tzi->trans && m_tzi->trans[i] <= timestamp_begin;
         ++i) {
      lastBefore = i;
    }
    // If explicitly provided a timestamp to the ret array
    // and always make sure there is at least one returned value
    if (!m_tzi->trans ||
        timestamp_begin >= timestamp_end || (
          (timestamp_begin != k_PHP_INT_MIN || timestamp_end != k_PHP_INT_MAX) &&
          timestamp_begin != m_tzi->trans[lastBefore])) {
      auto dt = req::make<DateTime>(
        timestamp_begin, req::make<TimeZone>("UTC"));
      int index = m_tzi->trans ? m_tzi->trans_idx[lastBefore] : 0;
      ttinfo &offset = m_tzi->type[index];
      const char *abbr = m_tzi->timezone_abbr + offset.abbr_idx;
      ret.append(make_map_array(
        s_ts, timestamp_begin,
        s_time, dt->toString(DateTime::DateFormat::ISO8601),
        s_offset, offset.offset,
        s_isdst, (bool)offset.isdst,
        s_abbr, String(abbr, CopyString)
      ));
    }
    for (uint32_t i = lastBefore;
         i < timecnt && m_tzi->trans && m_tzi->trans[i] < timestamp_end;
         ++i) {
      int timestamp = m_tzi->trans[i];
      if (timestamp_begin <= timestamp) {
        int index = m_tzi->trans_idx[i];
        auto dt = req::make<DateTime>(timestamp, req::make<TimeZone>("UTC"));
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

  ret.set(s_country_code, String(m_tzi->location.country_code, CopyString));
  ret.set(s_latitude,     m_tzi->location.latitude);
  ret.set(s_longitude,    m_tzi->location.longitude);
  ret.set(s_comments,     String(m_tzi->location.comments, CopyString));

  return ret;
}

///////////////////////////////////////////////////////////////////////////////
}
