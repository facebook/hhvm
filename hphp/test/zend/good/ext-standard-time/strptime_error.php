<?php
/* Prototype  : array strptime  ( string $date  , string $format  )
 * Description: Parse a time/date generated with strftime()
 * Source code: ext/standard/datetime.c
 * Alias to functions: 
 */

//Set the default time zone 
date_default_timezone_set("Europe/London");

echo "*** Testing strptime() : error conditions ***\n";

echo "\n-- Testing strptime() function with Zero arguments --\n";
var_dump( strptime() );

echo "\n-- Testing strptime() function with less than expected no. of arguments --\n";
$format = '%b %d %Y %H:%M:%S';
$timestamp = mktime(8, 8, 8, 8, 8, 2008);
$date = strftime($format, $timestamp);
var_dump( strptime($date) );

echo "\n-- Testing strptime() function with more than expected no. of arguments --\n";
$extra_arg = 10;
var_dump( strptime($date, $format, $extra_arg) );

?>
===DONE===