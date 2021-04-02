/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/util/functional.h"
#include "hphp/util/logger.h"
#include "hphp/util/lock.h"
#include "hphp/util/text-util.h"


// The opaque "struct _ttinfo" moved into a private header file.
#if TIMELIB_VERSION >= TIMELIB_MODERN
extern "C" {
#include <timelib_private.h>
}
#endif

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

struct GuessedTimeZone {
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

struct TimeZoneData {
  TimeZoneData() : Database(nullptr) {}

  const timelib_tzdb *Database;
};
static RDS_LOCAL(TimeZoneData, s_timezone_data);

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

const timelib_tzdb* (*timezone_raw_get_tzdb)() = timelib_builtin_db;

const timelib_tzdb* timezone_get_tzdb() {
  if (s_tzdb_cache.load() == nullptr) {
    Lock tzdbLock(s_tzdb_mutex);
    if (s_tzdb_cache.load() == nullptr) {
      s_tzdb_cache = (*timezone_raw_get_tzdb)();
    }
  }
  if (s_tzdb_cache == nullptr) {
    raise_error("Couldn't load tzdata");
  }
  return s_tzdb_cache;
}

const timelib_tzdb *TimeZone::GetDatabase() {
  const timelib_tzdb *&Database = s_timezone_data->Database;
  if (Database == nullptr) {
    Database = timezone_get_tzdb();
  }
  return Database;
}

#if TIMELIB_VERSION >= TIMELIB_MODERN
timelib_tzinfo* TimeZone::GetTimeZoneInfoRaw(const char* name,
                                             const timelib_tzdb* db,
                                             int* error_code)
#else
timelib_tzinfo* TimeZone::GetTimeZoneInfoRaw(char* name,
                                             const timelib_tzdb* db)
#endif
{
  auto const it = s_tzCache->find(name);
  if (it != s_tzCache->end()) {
    return it->second;
  }
#if TIMELIB_VERSION >= TIMELIB_MODERN
  auto tzi = timelib_parse_tzfile(name, db, error_code);
#else
  auto tzi = timelib_parse_tzfile(name, db);
#endif
  if (!tzi) {
    char* tzid = timelib_timezone_id_from_abbr(name, -1, 0);
    if (tzid) {
#if TIMELIB_VERSION >= TIMELIB_MODERN
      tzi = timelib_parse_tzfile(tzid, db, error_code);
#else
      tzi = timelib_parse_tzfile(tzid, db);
#endif
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
  auto& tz = RID().getTimezone();
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
  RID().setTimezone(name);
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
    DArrayInit element(3);
    element.set(s_dst, (bool)entry->type);
    element.set(s_offset, entry->gmtoffset);
    if (entry->full_tz_name) {
      element.set(s_timezone_id, String(entry->full_tz_name, CopyString));
    } else {
      element.set(s_timezone_id, uninit_null());
    }
    if (ret.isNull()) {
      ret = Array::CreateDict();
    }
    String key{entry->name};
    if (!ret.exists(key)) {
      ret.set(key, Array::CreateVec());
    }
    auto const lval = ret.lval(key);
    forceToArray(lval).append(element.toArray());
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
  /* We can't just use GetTimeZoneInfoRaw, as that requires a real timezone
   * name; we need to handle other cases like `GMT+2`. Quirks:
   *
   * - `GMT+2` is a ZONETYPE_OFFSET, but `Etc/GMT+2` is a ZONETYPE_ID
   * - `GMT+0` is usually (tzdb-dependent) a ZONETYPE_ID
   * - `CET` quirk below
   */
#if TIMELIB_VERSION >= TIMELIB_MODERN
  const char* tzname = name.data();
#else
  char* tzname = (char*) name.data();
#endif
  // Try an ID lookup first, so that `CET` is interpreted as an ID, not an
  // abbreviation. It's valid as either. PHP 5.5+ considers it an abbreviation,
  // HHVM currently intentionally considers it an ID for backwards
  // compatibility (which matches PHP <= 5.4)
#if TIMELIB_VERSION >= TIMELIB_MODERN
  int error_code = TIMELIB_ERROR_NO_ERROR;
  m_tzi = GetTimeZoneInfoRaw(tzname, GetDatabase(), &error_code);
#else
  m_tzi = GetTimeZoneInfoRaw(tzname, GetDatabase());
#endif
  if (m_tzi) {
    m_tztype = TIMELIB_ZONETYPE_ID;
    return;
  }

  // Not a timezone ID, try to parse it more generally.

  timelib_time *dummy = timelib_time_ctor();
  SCOPE_EXIT { timelib_time_dtor(dummy); };
  int dst, not_found;
  dummy->z = timelib_parse_zone(&tzname, &dst, dummy, &not_found, GetDatabase(),
                               GetTimeZoneInfoRaw);
  if (not_found) {
    return;
  }

  m_tztype = dummy->zone_type;
  switch(dummy->zone_type) {
    case TIMELIB_ZONETYPE_ID: {
      always_assert(false && TIMELIB_ZONETYPE_ID);
      break;
    }
    case TIMELIB_ZONETYPE_OFFSET:
      m_offset = dummy->z;
      break;
    case TIMELIB_ZONETYPE_ABBR:
      m_offset = dummy->z;
      m_dst = dummy->dst;
      m_abbr = String(dummy->tz_abbr, CopyString);
      break;
  }
}

TimeZone::TimeZone(timelib_tzinfo *tzi) {
  m_tzi = tzi;
  m_tztype = TIMELIB_ZONETYPE_ID;
}

req::ptr<TimeZone> TimeZone::cloneTimeZone() const {
  auto tz = req::make<TimeZone>();
  tz->m_tztype = m_tztype;
  tz->m_tzi = m_tzi;
  tz->m_offset = m_offset;
  tz->m_dst = m_dst;
  tz->m_abbr = m_abbr;
  assertx(tz->isValid() == isValid() && "incomplete TimeZone copy");
  return tz;
}

String TimeZone::name() const {
  switch (m_tztype) {
    case TIMELIB_ZONETYPE_ID:
      assertx(m_tzi);
      return String(m_tzi->name, CopyString);
    case TIMELIB_ZONETYPE_ABBR:
      return String(m_abbr, CopyString);
    case TIMELIB_ZONETYPE_OFFSET: {
      char buf[sizeof("+00:00")];

#if TIMELIB_VERSION >= TIMELIB_MODERN
      snprintf(
        buf,
        sizeof("+00:00"),
        "%c%02d:%02d",
        (m_offset < 0 ? '-' : '+'),
        abs(m_offset / 3600) % 100, // % 100 to convince compiler we have 2 digits
        abs((m_offset % 3600) / 60)
      );
#else
      snprintf(
        buf,
        sizeof("+00:00"),
        "%c%02d:%02d",
        (m_offset > 0 ? '-' : '+'),
        abs(m_offset / 60) % 100, // % 100 to convince compiler we have 2 digits
        abs(m_offset % 60)
      );
#endif
      return String(buf, sizeof("+00:00") - 1, CopyString);
    }
  }
  always_assert(false && "invalid tz type");
}

String TimeZone::abbr(int type /* = 0 */) const {
  if (m_tztype == TIMELIB_ZONETYPE_ABBR) {
    return String(m_abbr, CopyString);
  }

  if (!m_tzi) return String();
  return String(&m_tzi->timezone_abbr[m_tzi->type[type].abbr_idx], CopyString);
}

int TimeZone::type() const {
  return m_tztype;
}

int TimeZone::offset(int64_t timestamp) const {
  if (m_tztype == TIMELIB_ZONETYPE_OFFSET
      || m_tztype == TIMELIB_ZONETYPE_ABBR) {
    return m_offset;
  }

  assertx(m_tzi && m_tztype == TIMELIB_ZONETYPE_ID);

  timelib_time_offset *offset =
    timelib_get_time_zone_info(timestamp, m_tzi);
  int ret = offset->offset;
  timelib_time_offset_dtor(offset);
  return ret;
}

bool TimeZone::dst(int64_t timestamp) const {
  if (m_tztype == TIMELIB_ZONETYPE_ABBR) {
    return m_dst;
  }
  if (!m_tzi) return false;

  timelib_time_offset *offset =
    timelib_get_time_zone_info(timestamp, m_tzi);
  bool ret = offset->is_dst;
  timelib_time_offset_dtor(offset);
  return ret;
}

Array TimeZone::transitions(int64_t timestamp_begin, /* = k_PHP_INT_MIN */
                            int64_t timestamp_end /* = k_PHP_INT_MAX */) const {
  Array ret = Array::CreateVec();

  if (m_tztype == TIMELIB_ZONETYPE_ABBR) {
    // PHP 5.5+ just returns `false` here, but we're probably depending on
    // on this working for some of the timezones that can be parsed either
    // as an ID (old behavior) or abbreviation (new behavior), e.g. 'CDT'
    // ... BC is nice anyway :)
    auto name = timelib_timezone_id_from_abbr(
      m_abbr.data(),
      m_offset,
      m_dst
    );
    if (name) {
      auto tz = req::make<TimeZone>(name);
      return tz->transitions(timestamp_begin, timestamp_end);
    }
  }

  if (!m_tzi) {
    return null_array;
  }

  uint64_t timecnt;
  timecnt = m_tzi->bit64.timecnt;
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
    ret.append(make_darray(
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
    int64_t timestamp = m_tzi->trans[i];
    if (timestamp_begin <= timestamp) {
      int index = m_tzi->trans_idx[i];
      auto dt = req::make<DateTime>(timestamp, req::make<TimeZone>("UTC"));
      ttinfo &offset = m_tzi->type[index];
      const char *abbr = m_tzi->timezone_abbr + offset.abbr_idx;

      ret.append(make_darray(
        s_ts, timestamp,
        s_time, dt->toString(DateTime::DateFormat::ISO8601),
        s_offset, offset.offset,
        s_isdst, (bool)offset.isdst,
        s_abbr, String(abbr, CopyString)
      ));
    }
  }
  if (ret.empty()) {
    return null_array;
  }
  return ret;
}

Array TimeZone::getLocation() const {
  if (!m_tzi) return Array{};
  DArrayInit ret(4);

  ret.set(s_country_code, String(m_tzi->location.country_code, CopyString));
  ret.set(s_latitude,     m_tzi->location.latitude);
  ret.set(s_longitude,    m_tzi->location.longitude);
  ret.set(s_comments,     String(m_tzi->location.comments, CopyString));

  return ret.toArray();
}

void TimeZone::sweep() {
  m_abbr.reset();
}

///////////////////////////////////////////////////////////////////////////////
}
