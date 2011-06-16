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

#ifndef __HPHP_TIMEZONE_H__
#define __HPHP_TIMEZONE_H__

#include <runtime/base/types.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/resource_data.h>
#include <runtime/base/util/smart_object.h>

extern "C" {
#include <timelib.h>
}

namespace HPHP {

typedef boost::shared_ptr<timelib_tzinfo> TimeZoneInfo;
typedef std::map<std::string, TimeZoneInfo> MapStringToTimeZoneInfo;
///////////////////////////////////////////////////////////////////////////////

/**
 * Handles all timezone related functions.
 */
class TimeZone : public SweepableResourceData {
public:
  DECLARE_OBJECT_ALLOCATION(TimeZone);

  static const int64 AFRICA      = 0x0001;
  static const int64 AMERICA     = 0x0002;
  static const int64 ANTARCTICA  = 0x0004;
  static const int64 ARCTIC      = 0x0008;
  static const int64 ASIA        = 0x0010;
  static const int64 ATLANTIC    = 0x0020;
  static const int64 AUSTRALIA   = 0x0040;
  static const int64 EUROPE      = 0x0080;
  static const int64 INDIAN      = 0x0100;
  static const int64 PACIFIC     = 0x0200;
  static const int64 UTC         = 0x0400;
  static const int64 ALL         = 0x07FF;
  static const int64 ALL_WITH_BC = 0x0FFF;
  static const int64 PER_COUNTRY = 0x1000;

  /**
   * Get/set current timezone that controls how local time is interpreted.
   */
  static String CurrentName();            // current timezone's name
  static SmartObject<TimeZone> Current(); // current timezone
  static bool SetCurrent(CStrRef name);   // returns false if invalid

  /**
   * TimeZone database queries.
   */
  static bool IsValid(CStrRef name);
  static Array GetNames(int64 what, CStrRef country);
  static Array GetAbbreviations();
  static String AbbreviationToName(String abbr, int utcoffset = -1,
                                   bool isdst = true);

public:
  /**
   * Constructing a timezone object by name or a raw pointer (internal).
   */
  TimeZone();
  TimeZone(CStrRef name);
  TimeZone(timelib_tzinfo *tzi);

  static StaticString s_class_name;
  // overriding ResourceData
  CStrRef o_getClassName() const { return s_class_name; }

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
  Array transitions(int64 timestamp_begin, int64 timestamp_end) const;

  /**
   * Make a copy of this timezone object, so it can be changed independently.
   */
  SmartObject<TimeZone> cloneTimeZone() const;

  /**
   * Returns location information for a timezone.
   */
  Array getLocation() const;

protected:
  friend class DateTime;
  friend class TimeStamp;

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
  static TimeZoneInfo GetTimeZoneInfo(CStrRef name);

  static bool checkIdAllowed(char *id, int64 what);

  void addTransition(Array &array, int i, int64 timestamp) const;

  TimeZoneInfo m_tzi; // raw pointer
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_TIMEZONE_H__
