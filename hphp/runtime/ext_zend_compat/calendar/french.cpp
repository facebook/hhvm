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
 *     void
 *     SdnToFrench(
 *         long int  sdn,
 *         int      *pYear,
 *         int      *pMonth,
 *         int      *pDay);
 *
 * Convert a SDN to a French republican calendar date.  If the input SDN is
 * before the first day of year 1 or after the last day of year 14, the
 * three output values will all be set to zero, otherwise *pYear will be in
 * the range 1 to 14 inclusive; *pMonth will be in the range 1 to 13
 * inclusive; *pDay will be in the range 1 to 30 inclusive.  If *pMonth is
 * 13, the SDN represents one of the holidays at the end of the year and
 * *pDay will be in the range 1 to 6 inclusive.
 *
 *     long int
 *     FrenchToSdn(
 *         int year,
 *         int month,
 *         int day);
 *
 * Convert a French republican calendar date to a SDN.  Zero is returned
 * when the input date is detected as invalid or out of the supported
 * range.  The return value will be > 0 for all valid, supported dates, but
 * there are some invalid dates that will return a positive value.  To
 * verify that a date is valid, convert it to SDN and then back and compare
 * with the original.
 *
 *     char *FrenchMonthName[14];
 *
 * Convert a French republican month number (1 to 13) to the name of the
 * French republican month (null terminated).  An index of 13 (for the
 * "extra" days at the end of the year) will return the string "Extra".  An
 * index of zero will return a zero length string.
 *
 * VALID RANGE
 *
 *     These routines only convert dates in years 1 through 14 (Gregorian
 *     dates 22 September 1792 through 22 September 1806).  This more than
 *     covers the period when the calendar was in use.
 *
 *     I would support a wider range of dates, but I have not been able to
 *     find an authoritative definition of when leap years were to have
 *     occurred.  There are suggestions that it was to skip a leap year ever
 *     100 years like the Gregorian calendar.
 *
 * CALENDAR OVERVIEW
 *
 *     The French republican calendar was adopted in October 1793 during
 *     the French Revolution and was abandoned in January 1806.  The intent
 *     was to create a new calendar system that was based on scientific
 *     principals, not religious traditions.
 *
 *     The year is divided into 12 months of 30 days each.  The remaining 5
 *     to 6 days in the year are grouped at the end and are holidays.  Each
 *     month is divided into three decades (instead of weeks) of 10 days
 *     each.
 *
 *     The epoch (first day of the first year) is 22 September 1792 in the
 *     Gregorian calendar.  Leap years are every fourth year (year 3, 7,
 *     11, etc.)
 *
 * TESTING
 *
 *     This algorithm has been tested from the year 1 to 14.  The source
 *     code of the verification program is included in this package.
 *
 * REFERENCES
 *
 *     I have found no detailed, authoritative reference on this calendar.
 *     The algorithms are based on a preponderance of less authoritative
 *     sources.
 *
 **************************************************************************/

#include "sdncal.h"

#define FRENCH_SDN_OFFSET         2375474
#define DAYS_PER_4_YEARS   1461
#define DAYS_PER_MONTH     30
#define FIRST_VALID        2375840
#define LAST_VALID         2380952

void SdnToFrench(
          long int sdn,
          int *pYear,
          int *pMonth,
          int *pDay)
{
  long int temp;
  int dayOfYear;

  if (sdn < FIRST_VALID || sdn > LAST_VALID) {
    *pYear = 0;
    *pMonth = 0;
    *pDay = 0;
    return;
  }
  temp = (sdn - FRENCH_SDN_OFFSET) * 4 - 1;
  *pYear = temp / DAYS_PER_4_YEARS;
  dayOfYear = (temp % DAYS_PER_4_YEARS) / 4;
  *pMonth = dayOfYear / DAYS_PER_MONTH + 1;
  *pDay = dayOfYear % DAYS_PER_MONTH + 1;
}

long int FrenchToSdn(
            int year,
            int month,
            int day)
{
  /* check for invalid dates */
  if (year < 1 || year > 14 ||
    month < 1 || month > 13 ||
    day < 1 || day > 30) {
    return (0);
  }
  return ((year * DAYS_PER_4_YEARS) / 4
      + (month - 1) * DAYS_PER_MONTH
      + day
      + FRENCH_SDN_OFFSET);
}

char *FrenchMonthName[14] =
{
  "",
  "Vendemiaire",
  "Brumaire",
  "Frimaire",
  "Nivose",
  "Pluviose",
  "Ventose",
  "Germinal",
  "Floreal",
  "Prairial",
  "Messidor",
  "Thermidor",
  "Fructidor",
  "Extra"
};


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */
