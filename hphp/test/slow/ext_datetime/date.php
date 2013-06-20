<?php

date_default_timezone_set("America/Los_Angeles");

function format($dt) {
  var_dump(date_format($dt, "Y-m-d H:i:s"));
}

format(date_create("2006-12-12"), "2006-12-12 00:00:00");
format(date_create("@1170288001"), "2007-02-01 00:00:01");

$dt = date_create("2006-12-12 12:34:56");
date_date_set($dt, 2007, 11, 23);
format($dt);


$dt = date_create("2008-08-08 00:00:00");
date_isodate_set($dt, 2007, 35, 3);
format($dt);

$dt = date_create("2006-12-12 00:00:00");
date_modify($dt, "+1 day");
format($dt);

var_dump(date_offset_get(date_create("2006-12-12")));
var_dump(date_offset_get(date_create("2008-08-08")));

$dt = date_create("2006-12-12 12:34:56");
date_time_set($dt, 23, 45, 12);
format($dt);


$d = strtotime("2008-09-10 12:34:56");

var_dump(date("l", $d));

var_dump(date("l jS \\of F Y h:i:s A", $d));

var_dump(date("l", mktime(0, 0, 0, 7, 1, 2000)));

var_dump(date(DATE_RFC822, $d));

var_dump(date(DATE_ATOM, mktime(0, 0, 0, 7, 1, 2000)));

var_dump(date("l \\t\\h\\e jS", $d));

$tomorrow = mktime(0,0,0,
                        (int)date("m", $d),
                        (int)date("d", $d) + 1,
                        (int)date("Y", $d));
var_dump($tomorrow);

$lastmonth = mktime(0,0,0,
                         (int)date("m", $d) - 1,
                         (int)date("d", $d),
                         (int)date("Y", $d));
var_dump($lastmonth);

$nextyear = mktime(0,0,0,
                        (int)date("m", $d),
                        (int)date("d", $d),
                        (int)date("Y", $d) + 1);
var_dump($nextyear);

$d = strtotime("2001-03-10 05:16:18");
var_dump(date("F j, Y, g:i a", $d));
var_dump(date("m.d.y", $d));
var_dump(date("j, n, Y", $d));
var_dump(date("Ymd", $d));
var_dump(date("h-i-s, j-m-y, it is w Day z ", $d));
var_dump(date("\\i\\t \\i\\s \\t\\h\\e jS \\d\\a\\y.", $d));
var_dump(date("D M j G:i:s T Y", $d));
var_dump(date("H:m:s \\m \\i\\s\\ \\m\\o\\n\\t\\h", $d));
var_dump(date("H:i:s", $d));

$d = strtotime("1955-03-10 05:16:18");
var_dump(date("Ymd", $d));

var_dump(date("r", -5000000000));

var_dump(mktime(0, 0, 0, 2, 26 - 91, 2010));

$d = strtotime("2008-09-10 12:34:56");
$today = getdate($d);
var_dump($today);

$tod = gettimeofday();
var_dump(count($tod));
var_dump($tod['sec'] > 1073504408);
var_dump(gettimeofday(true) > 1073504408.23910);

$timestamp = strtotime("1st January 2004"); //1072915200

// this prints the year in a two digit format
// however, as this would start with a "0", it
// only prints "4"
var_dump(idate("y", $timestamp));
