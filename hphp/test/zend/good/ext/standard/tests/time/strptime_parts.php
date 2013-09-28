<?php
/* Prototype  : array strptime  ( string $date  , string $format  )
 * Description:  Parse a time/date generated with strftime()
 * Source code: ext/standard/datetime.c
*/

$orig = setlocale(LC_ALL, 'C');
date_default_timezone_set("GMT"); 
putenv("TZ=GMT");

echo "*** Testing strptime() : basic functionality ***\n";

$input = "10:01:20 AM July 2 1963";
$tstamp = strtotime($input);
 
$str = strftime("%r %B%e %Y %Z", $tstamp);
$res = strptime($str, '%H:%M:%S %p %B %d %Y %Z');
var_dump($res["tm_sec"]);
var_dump($res["tm_min"]);
var_dump($res["tm_hour"]);
var_dump($res["tm_mday"]);
var_dump($res["tm_mon"]);
var_dump($res["tm_year"]);

$str = strftime("%T %D", $tstamp);
$res = strptime($str, '%H:%M:%S %m/%d/%y');
var_dump($res["tm_sec"]);
var_dump($res["tm_min"]);
var_dump($res["tm_hour"]);
var_dump($res["tm_mday"]);
var_dump($res["tm_mon"]);
var_dump($res["tm_year"]);

$str = strftime("%A %B %e %R", $tstamp);
$res = strptime($str, '%A %B %e %R');
var_dump($res["tm_sec"]);
var_dump($res["tm_min"]);
var_dump($res["tm_hour"]);
var_dump($res["tm_mday"]);
var_dump($res["tm_mon"]);
var_dump($res["tm_year"]);

setlocale(LC_ALL, $orig);
?>
===DONE===