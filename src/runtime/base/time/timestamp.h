/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#ifndef __HPHP_TIMESTAMP_H__
#define __HPHP_TIMESTAMP_H__

#include <runtime/base/types.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * Using an integer to represent time: the value of time in seconds since 0
 * hours, 0 minutes, 0 seconds, January 1, 1970, Coordinated Universal Time,
 * without including leap seconds. Therefore, this integer is by definition
 * non timezone specific and thus not shifted by DST.
 */
class TimeStamp {
public:
  static int Current();
  static double CurrentSecond();
  static Array CurrentTime();
  static String CurrentMicroTime();

  static int Get(bool &error, int hour = INT_MAX, int minute = INT_MAX,
                 int second = INT_MAX, int month = INT_MAX, int day = INT_MAX,
                 int year = INT_MAX, bool gmt = false);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_TIMESTAMP_H__
