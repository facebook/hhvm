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

#include <test/test_ext_datetime.h>
#include <runtime/ext/ext_datetime.h>
#include <runtime/ext/ext_string.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtDatetime::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_checkdate);
  RUN_TEST(test_date_create);
  RUN_TEST(test_date_date_set);
  RUN_TEST(test_date_default_timezone_get);
  RUN_TEST(test_date_default_timezone_set);
  RUN_TEST(test_date_format);
  RUN_TEST(test_date_isodate_set);
  RUN_TEST(test_date_modify);
  RUN_TEST(test_date_offset_get);
  RUN_TEST(test_date_parse);
  RUN_TEST(test_date_sun_info);
  RUN_TEST(test_date_sunrise);
  RUN_TEST(test_date_sunset);
  RUN_TEST(test_date_time_set);
  RUN_TEST(test_date_timezone_get);
  RUN_TEST(test_date_timezone_set);
  RUN_TEST(test_date);
  RUN_TEST(test_getdate);
  RUN_TEST(test_gettimeofday);
  RUN_TEST(test_gmdate);
  RUN_TEST(test_gmmktime);
  RUN_TEST(test_gmstrftime);
  RUN_TEST(test_idate);
  RUN_TEST(test_localtime);
  RUN_TEST(test_microtime);
  RUN_TEST(test_mktime);
  RUN_TEST(test_strftime);
  RUN_TEST(test_strptime);
  RUN_TEST(test_strtotime);
  RUN_TEST(test_time);
  RUN_TEST(test_timezone_abbreviations_list);
  RUN_TEST(test_timezone_identifiers_list);
  RUN_TEST(test_timezone_name_from_abbr);
  RUN_TEST(test_timezone_name_get);
  RUN_TEST(test_timezone_offset_get);
  RUN_TEST(test_timezone_open);
  RUN_TEST(test_timezone_transitions_get);

  return ret;
}

#define VDT(dt, s) VS(f_date_format(dt, "Y-m-d H:i:s"), s)

///////////////////////////////////////////////////////////////////////////////

bool TestExtDatetime::test_checkdate() {
  VERIFY(f_checkdate(12, 31, 2000));
  VERIFY(!f_checkdate(2, 29, 2001));
  return Count(true);
}

bool TestExtDatetime::test_date_create() {
  VDT(f_date_create("2006-12-12"),  "2006-12-12 00:00:00");
  VDT(f_date_create("@1170288001"), "2007-02-01 00:00:01");
  return Count(true);
}

bool TestExtDatetime::test_date_date_set() {
  Object dt = f_date_create("2006-12-12 12:34:56");
  f_date_date_set(dt, 2007, 11, 23);
  VDT(dt, "2007-11-23 12:34:56");
  return Count(true);
}

bool TestExtDatetime::test_date_default_timezone_get() {
  VS(f_date_default_timezone_get(), "America/Los_Angeles");
  return Count(true);
}

bool TestExtDatetime::test_date_default_timezone_set() {
  VERIFY(f_date_default_timezone_set("Asia/Shanghai"));
  VS(f_date_default_timezone_get(), "Asia/Shanghai");
  VERIFY(f_date_default_timezone_set("America/Los_Angeles"));
  VS(f_date_default_timezone_get(), "America/Los_Angeles");
  return Count(true);
}

bool TestExtDatetime::test_date_format() {
  Object dt = f_date_create("@1170288001");
  VS(f_date_format(dt, "Y-m-d\\TH:i:s\\Z"), "2007-02-01T00:00:01Z");
  VS(f_date_format(dt, "Y-m-dTH:i:sZ"), "2007-02-01PDT00:00:01-28800");
  VS(DateTime(1255494072, true).toString(DateTime::Cookie),
     "Wed, 14-Oct-2009 04:21:12 GMT");
  return Count(true);
}

bool TestExtDatetime::test_date_isodate_set() {
  Object dt = f_date_create("2008-08-08 00:00:00");
  f_date_isodate_set(dt, 2007, 35, 3);
  VDT(dt, "2007-08-29 00:00:00");
  return Count(true);
}

bool TestExtDatetime::test_date_modify() {
  Object dt = f_date_create("2006-12-12 00:00:00");
  f_date_modify(dt, "+1 day");
  VDT(dt, "2006-12-13 00:00:00");
  return Count(true);
}

bool TestExtDatetime::test_date_offset_get() {
  VS(f_date_offset_get(f_date_create("2006-12-12")), -28800);
  VS(f_date_offset_get(f_date_create("2008-08-08")), -25200);
  return Count(true);
}

bool TestExtDatetime::test_date_parse() {
  VS(f_print_r(f_date_parse("2006-12-12 10:00:00.5"), true),
     "Array\n"
     "(\n"
     "    [year] => 2006\n"
     "    [month] => 12\n"
     "    [day] => 12\n"
     "    [hour] => 10\n"
     "    [minute] => 0\n"
     "    [second] => 0\n"
     "    [fraction] => 0.5\n"
     "    [warning_count] => 0\n"
     "    [warnings] => \n"
     "    [error_count] => 0\n"
     "    [errors] => \n"
     "    [is_localtime] => \n"
     ")\n");
  return Count(true);
}

bool TestExtDatetime::test_date_sun_info() {
  VS(f_print_r(f_date_sun_info(f_strtotime("2006-12-12"), 31.7667, 35.2333),
               true),
     "Array\n"
     "(\n"
     "    [sunrise] => 1165897795\n"
     "    [sunset] => 1165934173\n"
     "    [transit] => 1165915984\n"
     "    [civil_twilight_begin] => 1165896189\n"
     "    [civil_twilight_end] => 1165935779\n"
     "    [nautical_twilight_begin] => 1165894366\n"
     "    [nautical_twilight_end] => 1165937603\n"
     "    [astronomical_twilight_begin] => 1165892582\n"
     "    [astronomical_twilight_end] => 1165939386\n"
     ")\n");
  return Count(true);
}

bool TestExtDatetime::test_date_sunrise() {
  /*
   * calculate the sunrise time for Lisbon, Portugal
   * Latitude: 38.4 North
   * Longitude: 9 West
   * Zenith ~= 90
   * offset: +1 GMT
   */
  VS(f_date_sunrise(f_strtotime("2004-12-20"), k_SUNFUNCS_RET_STRING,
                    38.4, -9, 90, 1), "08:52");
  return Count(true);
}

bool TestExtDatetime::test_date_sunset() {
  /*
   * calculate the sunset time for Lisbon, Portugal
   * Latitude: 38.4 North
   * Longitude: 9 West
   * Zenith ~= 90
   * offset: +1 GMT
   */
  VS(f_date_sunset(f_strtotime("2004-12-20"), k_SUNFUNCS_RET_STRING,
                   38.4, -9, 90, 1), "18:15");
  return Count(true);
}

bool TestExtDatetime::test_date_time_set() {
  Object dt = f_date_create("2006-12-12 12:34:56");
  f_date_time_set(dt, 23, 45, 12);
  VDT(dt, "2006-12-12 23:45:12");
  return Count(true);
}

bool TestExtDatetime::test_date_timezone_get() {
  Object dt = f_date_create("2008-08-08 12:34:56");
  VS(f_timezone_name_get(f_date_timezone_get(dt)), "America/Los_Angeles");
  return Count(true);
}

bool TestExtDatetime::test_date_timezone_set() {
  Object dt = f_date_create("2008-08-08 12:34:56");
  f_date_timezone_set(dt, f_timezone_open("Asia/Shanghai"));
  VS(f_timezone_name_get(f_date_timezone_get(dt)), "Asia/Shanghai");
  VDT(dt, "2008-08-09 03:34:56");
  return Count(true);
}

bool TestExtDatetime::test_date() {
  int d = f_strtotime("2008-09-10 12:34:56");

  VS(f_date("l", d), "Wednesday");

  VS(f_date("l jS \\of F Y h:i:s A", d),
     "Wednesday 10th of September 2008 12:34:56 PM");

  VS(f_date("l", f_mktime(0, 0, 0, 7, 1, 2000)), "Saturday");

  VS(f_date(k_DATE_RFC822, d), "Wed, 10 Sep 08 12:34:56 -0700");

  VS(f_date(k_DATE_ATOM, f_mktime(0, 0, 0, 7, 1, 2000)),
     "2000-07-01T00:00:00-07:00");

  VS(f_date("l \\t\\h\\e jS", d), "Wednesday the 10th");

  int tomorrow = f_mktime(0,0,0,
                          f_date("m", d).toInt32(),
                          f_date("d", d).toInt32() + 1,
                          f_date("Y", d).toInt32());
  VS(tomorrow, 1221116400);

  int lastmonth = f_mktime(0,0,0,
                           f_date("m", d).toInt32() - 1,
                           f_date("d", d).toInt32(),
                           f_date("Y", d).toInt32());
  VS(lastmonth, 1218351600);

  int nextyear = f_mktime(0,0,0,
                          f_date("m", d).toInt32(),
                          f_date("d", d).toInt32(),
                          f_date("Y", d).toInt32() + 1);
  VS(nextyear, 1252566000);

  d = f_strtotime("2001-03-10 05:16:18");
  VS(f_date("F j, Y, g:i a", d), "March 10, 2001, 5:16 am");
  VS(f_date("m.d.y", d), "03.10.01");
  VS(f_date("j, n, Y", d), "10, 3, 2001");
  VS(f_date("Ymd", d), "20010310");
  VS(f_date("h-i-s, j-m-y, it is w Day z ", d),
     "05-16-18, 10-03-01, 1631 1618 6 Satam01 68 ");
  VS(f_date("\\i\\t \\i\\s \\t\\h\\e jS \\d\\a\\y.", d),
     "it is the 10th day.");
  VS(f_date("D M j G:i:s T Y", d), "Sat Mar 10 5:16:18 PDT 2001");
  VS(f_date("H:m:s \\m \\i\\s\\ \\m\\o\\n\\t\\h", d), "05:03:18 m is month");
  VS(f_date("H:i:s", d), "05:16:18");

  d = f_strtotime("1955-03-10 05:16:18");
  VS(f_date("Ymd", d), "19550310");

  VS(f_date("r", -5000000000), "Tue, 23 Jul 1811 07:06:40 -0800");

  VS(f_mktime(0, 0, 0, 2, 26 - 91, 2010), 1259308800);

  return Count(true);
}

bool TestExtDatetime::test_getdate() {
  int d = f_strtotime("2008-09-10 12:34:56");

  Array today = f_getdate(d);
  VS(f_print_r(today, true),
     "Array\n"
     "(\n"
     "    [seconds] => 56\n"
     "    [minutes] => 34\n"
     "    [hours] => 12\n"
     "    [mday] => 10\n"
     "    [wday] => 3\n"
     "    [mon] => 9\n"
     "    [year] => 2008\n"
     "    [yday] => 253\n"
     "    [weekday] => Wednesday\n"
     "    [month] => September\n"
     "    [0] => 1221075296\n"
     ")\n");

  return Count(true);
}

bool TestExtDatetime::test_gettimeofday() {
  Array ret = f_gettimeofday();
  VS(ret.size(), 4);
  VERIFY(more(ret["sec"], 1073504408));

  VERIFY(more(f_gettimeofday(true), 1073504408.23910));
  return Count(true);
}

bool TestExtDatetime::test_gmdate() {
  int d = f_mktime(0, 0, 0, 1, 1, 1998);
  VS(f_date("M d Y H:i:s",   d), "Jan 01 1998 00:00:00");
  VS(f_gmdate("M d Y H:i:s", d), "Jan 01 1998 08:00:00");
  return Count(true);
}

bool TestExtDatetime::test_gmmktime() {
  int d = f_gmmktime(0, 0, 0, 1, 1, 1998);
  VS(f_date("M d Y H:i:s",   d), "Dec 31 1997 16:00:00");
  VS(f_gmdate("M d Y H:i:s", d), "Jan 01 1998 00:00:00");
  return Count(true);
}

bool TestExtDatetime::test_gmstrftime() {
  f_setlocale(2, k_LC_TIME, "en_US.utf8");
  int d = f_mktime(20, 0, 0, 12, 31, 98);
  VS(f_strftime("%b %d %Y %H:%M:%S", d),   "Dec 31 1998 20:00:00");
  VS(f_gmstrftime("%b %d %Y %H:%M:%S", d), "Jan 01 1999 04:00:00");
  int t = f_mktime(0,0,0, 6, 27, 2006);
  VS(f_strftime("%a %A %b %B %c %C %d %D %e %g %G %h %H %I %j %m %M %n %p "
                "%r %R %S %t %T %u %U %V %W %w %x %X %y %Y %Z %z %%", t),
                "Tue Tuesday Jun June Tue 27 Jun 2006 12:00:00 AM PDT 20 27 "
                "06/27/06 27 06 2006 Jun 00 12 178 06 00 \n AM 12:00:00 AM "
                "00:00 00 \t 00:00:00 2 26 26 26 2 06/27/2006 12:00:00 AM "
                "06 2006 PDT -0700 %");
  return Count(true);
}

bool TestExtDatetime::test_idate() {
  int timestamp = f_strtotime("1st January 2004"); //1072915200

  // this prints the year in a two digit format
  // however, as this would start with a "0", it
  // only prints "4"
  VS(f_idate("y", timestamp), 4);
  return Count(true);
}

bool TestExtDatetime::test_localtime() {
  int d = f_strtotime("2008-09-10 12:34:56");

  Array localtime = f_localtime(d);
  Array localtime_assoc = f_localtime(d, true);
  VS(f_print_r(localtime, true),
     "Array\n"
     "(\n"
     "    [0] => 56\n"
     "    [1] => 34\n"
     "    [2] => 12\n"
     "    [3] => 10\n"
     "    [4] => 8\n"
     "    [5] => 108\n"
     "    [6] => 3\n"
     "    [7] => 253\n"
     "    [8] => 1\n"
     ")\n");

  VS(f_print_r(localtime_assoc, true),
     "Array\n"
     "(\n"
     "    [tm_sec] => 56\n"
     "    [tm_min] => 34\n"
     "    [tm_hour] => 12\n"
     "    [tm_mday] => 10\n"
     "    [tm_mon] => 8\n"
     "    [tm_year] => 108\n"
     "    [tm_wday] => 3\n"
     "    [tm_yday] => 253\n"
     "    [tm_isdst] => 1\n"
     ")\n");

  return Count(true);
}

bool TestExtDatetime::test_microtime() {
  int time_start = f_microtime(true);
  VERIFY(time_start > 0);
  return Count(true);
}

bool TestExtDatetime::test_mktime() {
  int lastday = f_mktime(0, 0, 0, 3, 0, 2000);
  VS(f_strftime("Last day in Feb 2000 is: %d", lastday),
     "Last day in Feb 2000 is: 29");

  /**
   * We are not supporting negative parameters
   * lastday = f_mktime(0, 0, 0, 4, -31, 2000);
   * VS(f_strftime("Last day in Feb 2000 is: %d", lastday),
   *    "Last day in Feb 2000 is: 29");
   */

  VS(f_date("M-d-Y", f_mktime(0, 0, 0, 12, 32, 1997)), "Jan-01-1998");
  VS(f_date("M-d-Y", f_mktime(0, 0, 0, 13, 1, 1997)),  "Jan-01-1998");
  VS(f_date("M-d-Y", f_mktime(0, 0, 0, 1, 1, 1998)),   "Jan-01-1998");
  VS(f_date("M-d-Y", f_mktime(0, 0, 0, 1, 1, 98)),     "Jan-01-1998");

  VS(f_mktime(), time(NULL));

  return Count(true);
}

bool TestExtDatetime::test_strftime() {
  int ts = f_mktime(0, 0, 0, 8, 5, 1998);

  f_setlocale(2, k_LC_TIME, "C");
  VS(f_strftime("%A", ts), "Wednesday");

  if (f_setlocale(2, k_LC_TIME, "fi_FI")) {
    VS(f_strftime(" in Finnish is %A,", ts), " in Finnish is keskiviikko,");
  } else {
    SKIP("setlocale() failed");
  }

  if (f_setlocale(2, k_LC_TIME, "fr_FR")) {
    VS(f_strftime(" in French %A and", ts), " in French mercredi and");
  } else {
    SKIP("setlocale() failed");
  }

  if (f_setlocale(2, k_LC_TIME, "de_DE")) {
    VS(f_strftime(" in German %A.", ts), " in German Mittwoch.");
  } else {
    SKIP("setlocale() failed");
  }

  f_setlocale(2, k_LC_TIME, "C");

/*
  December 2002 / January 2003
  ISOWk  M   Tu  W   Thu F   Sa  Su
  ----- ----------------------------
  51     16  17  18  19  20  21  22
  52     23  24  25  26  27  28  29
  1      30  31   1   2   3   4   5
  2       6   7   8   9  10  11  12
  3      13  14  15  16  17  18  19
*/
  VS(f_strftime("%V,%G,%Y", f_strtotime("12/28/2002")), "52,2002,2002");
  VS(f_strftime("%V,%G,%Y", f_strtotime("12/30/2002")), "01,2003,2002");
  VS(f_strftime("%V,%G,%Y", f_strtotime("1/3/2003")),   "01,2003,2003");
  VS(f_strftime("%V,%G,%Y", f_strtotime("1/10/2003")),  "02,2003,2003");

/*
  December 2004 / January 2005
  ISOWk  M   Tu  W   Thu F   Sa  Su
  ----- ----------------------------
  51     13  14  15  16  17  18  19
  52     20  21  22  23  24  25  26
  53     27  28  29  30  31   1   2
  1       3   4   5   6   7   8   9
  2      10  11  12  13  14  15  16
*/
  VS(f_strftime("%V,%G,%Y", f_strtotime("12/23/2004")), "52,2004,2004");
  VS(f_strftime("%V,%G,%Y", f_strtotime("12/31/2004")), "53,2004,2004");
  VS(f_strftime("%V,%G,%Y", f_strtotime("1/2/2005")),   "53,2004,2005");
  VS(f_strftime("%V,%G,%Y", f_strtotime("1/3/2005")),   "01,2005,2005");

  return Count(true);
}

bool TestExtDatetime::test_strptime() {
  String format = "%d/%m/%Y %H:%M:%S";
  String strf = f_strftime(format, f_strtotime("10/03/2004 15:54:19"));
  VS(strf, "03/10/2004 15:54:19");
  VS(f_print_r(f_strptime(strf, format), true),
     "Array\n"
     "(\n"
     "    [tm_sec] => 19\n"
     "    [tm_min] => 54\n"
     "    [tm_hour] => 15\n"
     "    [tm_mday] => 3\n"
     "    [tm_mon] => 9\n"
     "    [tm_year] => 104\n"
     "    [tm_wday] => 0\n"
     "    [tm_yday] => 276\n"
     "    [unparsed] => \n"
     ")\n");

  return Count(true);
}

bool TestExtDatetime::test_strtotime() {
  VERIFY(more(f_strtotime("now"), 0));
  VS(f_strtotime("10 September 2000"), 968569200);
  VS(f_strtotime("+1 day", 968569200), 968655600);
  VS(f_strtotime("+1 week", 968569200), 969174000);
  VS(f_strtotime("+1 week 2 days 4 hours 2 seconds", 968569200), 969361202);
  VS(f_strtotime("next Thursday", 968569200), 968914800);
  VS(f_strtotime("last Monday", 968569200), 968050800);

  String str = "Not Good";
  Variant timestamp = f_strtotime(str);
  VERIFY(same(timestamp, false));
  VERIFY(same(f_strtotime(""), false));
  return Count(true);
}

bool TestExtDatetime::test_time() {
  int nextWeek = f_time() + (7 * 24 * 60 * 60);
  VS(f_date("Y-m-d", nextWeek),
     f_date("Y-m-d", f_strtotime("+1 week")));
  return Count(true);
}

bool TestExtDatetime::test_timezone_abbreviations_list() {
  //f_var_dump(TimeZone::GetAbbreviations());
  VERIFY(!TimeZone::GetAbbreviations().empty());
  return Count(true);
}

bool TestExtDatetime::test_timezone_identifiers_list() {
  //f_var_dump(TimeZone::GetNames());
  VERIFY(!TimeZone::GetNames().empty());
  return Count(true);
}

bool TestExtDatetime::test_timezone_name_from_abbr() {
  VS(f_timezone_name_from_abbr("CET"), "Europe/Berlin");
  VS(f_timezone_name_from_abbr("", 3600, 0), "Europe/Paris");
  return Count(true);
}

bool TestExtDatetime::test_timezone_name_get() {
  Object tz = f_timezone_open("Asia/Shanghai");
  VS(f_timezone_name_get(tz), "Asia/Shanghai");
  return Count(true);
}

bool TestExtDatetime::test_timezone_offset_get() {
  // Create two timezone objects, one for Taipei (Taiwan) and one for
  // Tokyo (Japan)
  Object dateTimeZoneTaipei = f_timezone_open("Asia/Taipei");
  Object dateTimeZoneJapan = f_timezone_open("Asia/Tokyo");

  // Create two DateTime objects that will contain the same Unix timestamp, but
  // have different timezones attached to them.
  Object dateTimeTaipei = f_date_create("2008-08-08", dateTimeZoneTaipei);
  Object dateTimeJapan = f_date_create("2008-08-08", dateTimeZoneJapan);

  VS(f_date_offset_get(dateTimeTaipei), 28800);
  VS(f_date_offset_get(dateTimeJapan), 32400);

  return Count(true);
}

bool TestExtDatetime::test_timezone_open() {
  Object tz = f_timezone_open("Asia/Shanghai");
  VS(f_timezone_name_get(tz), "Asia/Shanghai");
  return Count(true);
}

bool TestExtDatetime::test_timezone_transitions_get() {
  Object timezone = f_timezone_open("CET");
  Array transitions = f_timezone_transitions_get(timezone);
  VS(transitions[0]["ts"], -1693706400);
  VS(transitions[0]["offset"], 7200);
  VS(transitions[0]["isdst"], true);
  VS(transitions[0]["abbr"], "CEST");
  return Count(true);
}
