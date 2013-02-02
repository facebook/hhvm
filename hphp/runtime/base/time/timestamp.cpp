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

#include <runtime/base/time/timestamp.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/time/datetime.h>
#include <timelib.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// creation

int64 TimeStamp::Current() {
  return time(0);
}

double TimeStamp::CurrentSecond() {
  struct timeval tp;
  gettimeofday(&tp, NULL);
  return (double)tp.tv_sec + (double)tp.tv_usec / 1000000;
}

Array TimeStamp::CurrentTime() {
  struct timeval tp;
  gettimeofday(&tp, NULL);

  timelib_time_offset *offset =
    timelib_get_time_zone_info(tp.tv_sec, TimeZone::Current()->get());

  Array ret;
  ret.set("sec", (int)tp.tv_sec);
  ret.set("usec", (int)tp.tv_usec);
  ret.set("minuteswest", (int)(-offset->offset / 60));
  ret.set("dsttime", (int)offset->is_dst);

  timelib_time_offset_dtor(offset);
  return ret;
}

String TimeStamp::CurrentMicroTime() {
  struct timeval tp;
  gettimeofday(&tp, NULL);
  char ret[100];
  snprintf(ret, 100, "%.8F %ld", (double)tp.tv_usec / 1000000, tp.tv_sec);
  return String(ret, CopyString);
}

int64 TimeStamp::Get(bool &error, int hou, int min, int sec, int mon, int day,
                   int yea, bool gmt) {
  DateTime dt(Current());
  if (gmt) {
    dt.setTimezone(SmartObject<TimeZone>(NEWOBJ(TimeZone)("UTC")));
  }
  dt.set(hou, min, sec, mon, day, yea);
  return dt.toTimeStamp(error);
}

///////////////////////////////////////////////////////////////////////////////
}
