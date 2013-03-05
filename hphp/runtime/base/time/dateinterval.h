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

#ifndef __HPHP_DATEINTERVAL_H__
#define __HPHP_DATEINTERVAL_H__

#include <runtime/base/types.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/resource_data.h>
#include <runtime/base/util/smart_object.h>
#include <util/alloc.h>

extern "C" {
#include <timelib.h>
}

/**
 * Older versions of timelib don't support certain
 * relative interval functions.  Mock them as needed here.
 */
#if defined(TIMELIB_VERSION) && (TIMELIB_VERSION >= 201101)
# define TIMELIB_HAVE_INTERVAL
# define TIMELIB_HAVE_TZLOCATION

# define TIMELIB_REL_INVERT(rel)          (rel)->invert
# define TIMELIB_REL_DAYS(rel)            (rel)->days
# define TIMELIB_REL_INVERT_SET(rel, val) (rel)->invert = (val)
# define TIMELIB_REL_DAYS_SET(rel, val)   (rel)->days = (val)
#else
# define TIMELIB_REL_INVERT(rel)          0
# define TIMELIB_REL_DAYS(rel)            -99999
# define TIMELIB_REL_INVERT_SET(rel, val)
# define TIMELIB_REL_DAYS_SET(rel, val)
inline timelib_rel_time* timelib_rel_time_clone(timelib_rel_time* t) {
  timelib_rel_time *ret = (timelib_rel_time*)
          HPHP::Util::safe_malloc(sizeof(timelib_rel_time));
  memcpy(ret, t, sizeof(timelib_rel_time));
  return ret;
}
inline void timelib_rel_time_dtor(timelib_rel_time *t) {
  HPHP::Util::safe_free(t);
}
#endif /* TIMELIB_HAVE_INTERVAL */

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
typedef boost::shared_ptr<timelib_rel_time> DateIntervalPtr;

/**
 * Handles all date interal related functions.
 */
class DateInterval : public SweepableResourceData {
public:
  DECLARE_OBJECT_ALLOCATION(DateInterval);
  static StaticString s_class_name;
  CStrRef o_getClassNameHook() const { return s_class_name; }

  DateInterval();
  DateInterval(CStrRef date_interval, bool date_string = false);
  DateInterval(timelib_rel_time *di);

  int64_t getYears()      const    { return m_di->y;                      }
  int64_t getMonths()     const    { return m_di->m;                      }
  int64_t getDays()       const    { return m_di->d;                      }
  int64_t getHours()      const    { return m_di->h;                      }
  int64_t getMinutes()    const    { return m_di->i;                      }
  int64_t getSeconds()    const    { return m_di->s;                      }
  bool  isInverted()    const    { return TIMELIB_REL_INVERT(m_di);     }
  bool  haveTotalDays() const    { return TIMELIB_REL_DAYS(m_di) != -99999; }
  int64_t getTotalDays()  const    { return TIMELIB_REL_DAYS(m_di);       }

  void setYears(int64_t value)     { if (isValid()) m_di->y      = value; }
  void setMonths(int64_t value)    { if (isValid()) m_di->m      = value; }
  void setDays(int64_t value)      { if (isValid()) m_di->d      = value; }
  void setHours(int64_t value)     { if (isValid()) m_di->h      = value; }
  void setMinutes(int64_t value)   { if (isValid()) m_di->i      = value; }
  void setSeconds(int64_t value)   { if (isValid()) m_di->s      = value; }
  void setInverted(bool value)   {
    if (isValid()) TIMELIB_REL_INVERT_SET(m_di, value);
  }
  void setTotalDays(int64_t value) {
    if (isValid()) TIMELIB_REL_DAYS_SET(m_di, value);
  }

  bool setDateString(CStrRef date_string);
  bool setInterval(CStrRef date_interval);
  String format(CStrRef format_spec);

  bool isValid() const { return get(); }
  SmartObject<DateInterval> cloneDateInterval() const;

protected:
  friend class DateTime;

  timelib_rel_time *get() const { return m_di.get(); }

private:
  struct dateinterval_deleter {
    void operator()(timelib_rel_time *di) {
      if (di) {
        timelib_rel_time_dtor(di);
      }
    }
  };

  DateIntervalPtr m_di;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_DATEINTERVAL_H__
