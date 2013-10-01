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

#ifndef incl_HPHP_TIMEZONE_H_
#define incl_HPHP_TIMEZONE_H_

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/complex-types.h"

extern "C" {
#include <timelib.h>
}

namespace HPHP {

typedef std::shared_ptr<timelib_tzinfo> TimeZoneInfo;
typedef std::map<std::string, TimeZoneInfo> MapStringToTimeZoneInfo;
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
  static SmartResource<TimeZone> Current(); // current timezone
  static bool SetCurrent(const String& name);   // returns false if invalid

  /**
   * TimeZone database queries.
   */
  static bool IsValid(const String& name);
  static Array GetNames();
  static Array GetAbbreviations();
  static String AbbreviationToName(String abbr, int utcoffset = -1,
                                   bool isdst = true);

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
  const String& o_getClassNameHook() const { return classnameof(); }

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
  int offset(int timestamp) const;

  /**
   * Test whether it was running under DST at specified timestamp.
   */
  bool dst(int timestamp) const;

  /**
   * Query transition times for DST.
   */
  Array transitions() const;

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
  SmartResource<TimeZone> cloneTimeZone() const;

protected:
  friend class DateTime;
  friend class TimeStamp;
  friend class DateInterval;

  /**
   * Returns raw pointer. For internal use only.
   */
  timelib_tzinfo *get() const { return m_tzi.get();}

private:
  struct tzinfo_deleter {
    void operator()(timelib_tzinfo *tzi) {
      if (tzi) {
        timelib_tzinfo_dtor(tzi);
      }
    }
  };

  static const timelib_tzdb *GetDatabase();

  /**
   * Look up cache and if found return it, otherwise, read it from database.
   */
  static TimeZoneInfo GetTimeZoneInfo(char* name, const timelib_tzdb* db);
  /**
   * only for timelib, don't use it unless you are passing to a timelib func
   */
  static timelib_tzinfo* GetTimeZoneInfoRaw(char* name, const timelib_tzdb* db);

  TimeZoneInfo m_tzi; // raw pointer
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_TIMEZONE_H_
