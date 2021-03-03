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

#pragma once

#include "hphp/runtime/base/resource-data.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/ext/std/ext_std_misc.h"

#include <map>
#include <memory>

extern "C" {
#include <timelib.h>
}

namespace HPHP {

struct Array;

///////////////////////////////////////////////////////////////////////////////

/**
 * Handles all timezone related functions.
 */
struct TimeZone : SweepableResourceData {
  DECLARE_RESOURCE_ALLOCATION(TimeZone);

  /**
   * Get/set current timezone that controls how local time is interpreted.
   */
  static String CurrentName();              // current timezone's name
  static req::ptr<TimeZone> Current();      // current timezone
  static bool SetCurrent(const char* name); // returns false if invalid

  /**
   * TimeZone database queries.
   */
  static bool IsValid(const char* name);
  static Array GetAbbreviations();
  static String AbbreviationToName(String abbr, int utcoffset = -1,
                                   int isdst = 1);

public:
  /**
   * Constructing a timezone object by name or a raw pointer (internal).
   */
  TimeZone();
  explicit TimeZone(const String& name);
  explicit TimeZone(timelib_tzinfo *tzi);

  static StaticString& classnameof() {
    static StaticString result("TimeZone");
    return result;
  }
  // overriding ResourceData
  const String& o_getClassNameHook() const override { return classnameof(); }

  /**
   * Whether this represents a valid timezone.
   */
  bool isValid() const {
    switch (m_tztype) {
      case 0:
        return false;
      case TIMELIB_ZONETYPE_ID:
        return getTZInfo();
      case TIMELIB_ZONETYPE_OFFSET:
      case TIMELIB_ZONETYPE_ABBR:
        return true;
    }
    always_assert(false && "invalid tztype");
  }

  /**
   * Get timezone's name or abbreviation.
   */
  String name() const;
  String abbr(int type = 0) const;
  int type() const;

  /**
   * Get offset from UTC at the specified timestamp under this timezone.
   */
  int offset(int64_t timestamp) const;

  /**
   * Test whether it was running under DST at specified timestamp.
   */
  bool dst(int64_t timestamp) const;

  /**
   * Query transition times for DST.
   */
  Array transitions(int64_t timestamp_begin = k_PHP_INT_MIN,
                    int64_t timestamp_end = k_PHP_INT_MAX) const;

  /**
   * Get information about a timezone
   */
  Array getLocation() const;

  /**
   * Timezone Database version
   */
  static String getVersion() {
    const timelib_tzdb* db = GetDatabase();
    return String(db->version, CopyString);
  }

  /**
   * Make a copy of this timezone object, so it can be changed independently.
   */
  req::ptr<TimeZone> cloneTimeZone() const;

protected:
  friend struct DateTime;
  friend struct TimeStamp;
  friend struct DateInterval;

  /**
   * Returns raw pointer. For internal use only.
   *
   * If type() !== TIMELIB_ZONETYPE_ID, this will definitely return nullptr,
   * even if isValid().
   */
  timelib_tzinfo *getTZInfo() const { return m_tzi; }

private:
  static const timelib_tzdb *GetDatabase();

  /**
   * Look up cache and if found return it, otherwise, read it from database.
   */
  static timelib_tzinfo* GetTimeZoneInfoRaw(const char* name, const timelib_tzdb* db, int* error_code);

  unsigned int m_tztype = 0;
  timelib_tzinfo* m_tzi = nullptr;
  int m_offset;
  int m_dst;
  String m_abbr;
};

///////////////////////////////////////////////////////////////////////////////

void timezone_init();
const timelib_tzdb* timezone_get_tzdb();
extern const timelib_tzdb* (*timezone_raw_get_tzdb)();

///////////////////////////////////////////////////////////////////////////////
}
