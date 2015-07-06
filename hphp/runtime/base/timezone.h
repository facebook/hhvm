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

#ifndef incl_HPHP_TIMEZONE_H_
#define incl_HPHP_TIMEZONE_H_

#include "hphp/runtime/base/resource-data.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/system/constants.h"

extern "C" {
#include <timelib.h>
#include <map>
#include <memory>
}

namespace HPHP {

class Array;

///////////////////////////////////////////////////////////////////////////////

/**
 * Handles all timezone related functions.
 */
class TimeZone : public SweepableResourceData {
public:
  DECLARE_RESOURCE_ALLOCATION(TimeZone);

  /**
   * Get/set current timezone that controls how local time is interpreted.
   */
  static String CurrentName();            // current timezone's name
  static req::ptr<TimeZone> Current(); // current timezone
  static bool SetCurrent(const String& name);   // returns false if invalid

  /**
   * TimeZone database queries.
   */
  static bool IsValid(const String& name);
  static Array GetAbbreviations();
  static Array GetNamesToCountryCodes();
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
  bool isValid() const { return get();}

  /**
   * Get timezone's name or abbreviation.
   */
  String name() const;
  String abbr(int type = 0) const;

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
  friend class DateTime;
  friend class TimeStamp;
  friend class DateInterval;

  /**
   * Returns raw pointer. For internal use only.
   */
  timelib_tzinfo *get() const { return m_tzi; }

private:
  static const timelib_tzdb *GetDatabase();

  /**
   * Look up cache and if found return it, otherwise, read it from database.
   */
  static timelib_tzinfo* GetTimeZoneInfoRaw(char* name, const timelib_tzdb* db);

  timelib_tzinfo* m_tzi;
};

///////////////////////////////////////////////////////////////////////////////

void timezone_init();

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_TIMEZONE_H_
