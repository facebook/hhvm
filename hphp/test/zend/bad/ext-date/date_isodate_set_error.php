<?php

/* Prototype  : DateTime date_isodate_set  ( DateTime $object  , int $year  , int $week  [, int $day  ] )
 * Description: Set a date according to the ISO 8601 standard - using weeks and day offsets rather than specific dates. 
 * Source code: ext/date/php_date.c
 * Alias to functions: DateTime::setISODate
 */
 
 //Set the default time zone 
date_default_timezone_set("Europe/London");

echo "*** Testing date_isodate_set() : error conditions ***\n";

echo "\n-- Testing date_isodate_set() function with zero arguments --\n";
var_dump( date_isodate_set() );

$datetime = date_create("2009-01-30 19:34:10");
echo "\n-- Testing date_isodate_set() function with less than expected no. of arguments --\n";
var_dump( date_isodate_set($datetime) );

echo "\n-- Testing date_isodate_set() function with more than expected no. of arguments --\n";
$year = 2009;
$week = 30; 
$day = 7;
$extra_arg = 30;
var_dump( date_isodate_set($datetime, $year, $week, $day, $extra_arg) );

echo "\n-- Testing date_isodate_set() function with an invalid values for \$object argument --\n";
$invalid_obj = new stdClass();
var_dump( date_isodate_set($invalid_obj, $year, $week, $day) );  
$invalid_obj = 10;
var_dump( date_isodate_set($invalid_obj, $year, $week, $day) );
$invalid_obj = null;
var_dump( date_isodate_set($invalid_obj, $year, $week, $day) );  
?>
===DONE===