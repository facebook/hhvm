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

#include "hphp/runtime/base/type-resource.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/util/alloc.h"

#include <memory>

extern "C" {
#include <timelib.h>
}

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
typedef std::shared_ptr<timelib_rel_time> DateIntervalPtr;

/**
 * Handles all date interval related functions.
 */
struct DateInterval : SweepableResourceData {
  DECLARE_RESOURCE_ALLOCATION(DateInterval);
  static const StaticString& classnameof() {
    static const StaticString result("DateInterval");
    return result;
  }
  const String& o_getClassNameHook() const override { return classnameof(); }

  DateInterval();
  explicit DateInterval(const String& date_interval, bool date_string = false);
  explicit DateInterval(timelib_rel_time *di);

  int64_t getYears()      const    { return m_di->y;                      }
  int64_t getMonths()     const    { return m_di->m;                      }
  int64_t getDays()       const    { return m_di->d;                      }
  int64_t getHours()      const    { return m_di->h;                      }
  int64_t getMinutes()    const    { return m_di->i;                      }
  int64_t getSeconds()    const    { return m_di->s;                      }
  bool  isInverted()      const    { return m_di->invert;                 }
  bool  haveTotalDays()   const    { return m_di->days != -99999;         }
  int64_t getTotalDays()  const    { return m_di->days;                   }

  void setYears(int64_t value)     { if (isValid()) m_di->y      = value; }
  void setMonths(int64_t value)    { if (isValid()) m_di->m      = value; }
  void setDays(int64_t value)      { if (isValid()) m_di->d      = value; }
  void setHours(int64_t value)     { if (isValid()) m_di->h      = value; }
  void setMinutes(int64_t value)   { if (isValid()) m_di->i      = value; }
  void setSeconds(int64_t value)   { if (isValid()) m_di->s      = value; }
  void setInverted(bool value)   {
    if (isValid()) m_di->invert = value;
  }
  void setTotalDays(int64_t value) {
    if (isValid()) m_di->days = value;
  }

  String format(const String& format_spec);

  bool isValid() const { return get(); }
  req::ptr<DateInterval> cloneDateInterval() const;

protected:
  friend struct DateTime;

  timelib_rel_time *get() const { return m_di.get(); }

private:
  void setDateString(const String& date_string);
  void setInterval(const String& date_interval);
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

