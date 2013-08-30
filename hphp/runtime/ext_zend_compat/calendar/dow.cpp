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

/**************************************************************************
 *
 * These are the externally visible components of this file:
 *
 *     int
 *     DayOfWeek(
 *         long int sdn);
 *
 * Convert a SDN to a day-of-week number (0 to 6).  Where 0 stands for
 * Sunday, 1 for Monday, etc. and 6 stands for Saturday.
 *
 *     char *DayNameShort[7];
 *
 * Convert a day-of-week number (0 to 6), as returned from DayOfWeek(), to
 * the abbreviated (three character) name of the day.
 *
 *     char *DayNameLong[7];
 *
 * Convert a day-of-week number (0 to 6), as returned from DayOfWeek(), to
 * the name of the day.
 *
 **************************************************************************/

#include "sdncal.h"

int DayOfWeek(
         long int sdn)
{
  int dow;

  dow = (sdn + 1) % 7;
  if (dow >= 0) {
    return (dow);
  } else {
    return (dow + 7);
  }
}

char *DayNameShort[7] =
{
  "Sun",
  "Mon",
  "Tue",
  "Wed",
  "Thu",
  "Fri",
  "Sat"
};

char *DayNameLong[7] =
{
  "Sunday",
  "Monday",
  "Tuesday",
  "Wednesday",
  "Thursday",
  "Friday",
  "Saturday"
};

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */
