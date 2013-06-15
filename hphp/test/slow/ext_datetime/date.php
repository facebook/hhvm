<?php

function VDT($dt, $what) {
  var_dump(date_format($dt, "Y-m-d H:i:s") == $what);
}

VDT(date_create("2006-12-12"), "2006-12-12 00:00:00");
VDT(date_create("@1170288001"), "2007-02-01 00:00:01");

$dt = date_create("2006-12-12 12:34:56");
date_date_set($dt, 2007, 11, 23);
VDT($dt, "2007-11-23 12:34:56");


$dt = date_create("2008-08-08 00:00:00");
date_isodate_set($dt, 2007, 35, 3);
VDT($dt, "2007-08-29 00:00:00");

$dt = date_create("2006-12-12 00:00:00");
date_modify($dt, "+1 day");
VDT($dt, "2006-12-13 00:00:00");

var_dump(date_offset_get(date_create("2006-12-12")) === -28800);
var_dump(date_offset_get(date_create("2008-08-08")) === -25200);

$dt = date_create("2006-12-12 12:34:56");
date_time_set($dt, 23, 45, 12);
VDT($dt, "2006-12-12 23:45:12");


function VS($x, $y) {
  var_dump($x === $y);
  if ($x !== $y) { echo "Failed: $y\n"; }
}

$d = strtotime("2008-09-10 12:34:56");

VS(date("l", $d), "Wednesday");

VS(date("l jS \\of F Y h:i:s A", $d),
   "Wednesday 10th of September 2008 12:34:56 PM");

VS(date("l", mktime(0, 0, 0, 7, 1, 2000)), "Saturday");

VS(date(DATE_RFC822, $d), "Wed, 10 Sep 08 12:34:56 -0700");

VS(date(DATE_ATOM, mktime(0, 0, 0, 7, 1, 2000)),
   "2000-07-01T00:00:00-07:00");

VS(date("l \\t\\h\\e jS", $d), "Wednesday the 10th");

$tomorrow = mktime(0,0,0,
                        (int)date("m", $d),
                        (int)date("d", $d) + 1,
                        (int)date("Y", $d));
VS($tomorrow, 1221116400);

$lastmonth = mktime(0,0,0,
                         (int)date("m", $d) - 1,
                         (int)date("d", $d),
                         (int)date("Y", $d));
VS($lastmonth, 1218351600);

$nextyear = mktime(0,0,0,
                        (int)date("m", $d),
                        (int)date("d", $d),
                        (int)date("Y", $d) + 1);
VS($nextyear, 1252566000);

$d = strtotime("2001-03-10 05:16:18");
VS(date("F j, Y, g:i a", $d), "March 10, 2001, 5:16 am");
VS(date("m.d.y", $d), "03.10.01");
VS(date("j, n, Y", $d), "10, 3, 2001");
VS(date("Ymd", $d), "20010310");
VS(date("h-i-s, j-m-y, it is w Day z ", $d),
   "05-16-18, 10-03-01, 1631 1618 6 Satam01 68 ");
VS(date("\\i\\t \\i\\s \\t\\h\\e jS \\d\\a\\y.", $d),
   "it is the 10th day.");
VS(date("D M j G:i:s T Y", $d), "Sat Mar 10 5:16:18 PST 2001");
VS(date("H:m:s \\m \\i\\s\\ \\m\\o\\n\\t\\h", $d), "05:03:18 m is month");
VS(date("H:i:s", $d), "05:16:18");

$d = strtotime("1955-03-10 05:16:18");
VS(date("Ymd", $d), "19550310");

VS(date("r", -5000000000), "Tue, 23 Jul 1811 07:06:40 -0800");

VS(mktime(0, 0, 0, 2, 26 - 91, 2010), 1259308800);

$d = strtotime("2008-09-10 12:34:56");
$today = getdate($d);
var_dump($today);

$tod = gettimeofday();
VS(count($tod), 4);
var_dump($tod['sec'] > 1073504408);
var_dump(gettimeofday(true) > 1073504408.23910);

$timestamp = strtotime("1st January 2004"); //1072915200

// this prints the year in a two digit format
// however, as this would start with a "0", it
// only prints "4"
VS(idate("y", $timestamp), 4);
