<?php
/* Prototype  : DateTime date_time_set  ( DateTime $object  , int $hour  , int $minute  [, int $second  ] )
 * Description: Resets the current time of the DateTime object to a different time. 
 * Source code: ext/date/php_date.c
 * Alias to functions: DateTime::setTime
 */
 
date_default_timezone_set("Europe/London");

echo "*** Testing date_time_set() : error conditions ***\n";

echo "\n-- Testing date_time_set() function with zero arguments --\n";
var_dump( date_time_set() );

echo "\n-- Testing date_time_set() function with less than expected no. of arguments --\n";
$datetime = date_create("2009-01-31 15:34:10");
$hour = 18;
var_dump( date_time_set($datetime) );
var_dump( date_time_set($datetime, $hour) );

echo "\n-- Testing date_time_set() function with more than expected no. of arguments --\n";
$min = 15;
$sec = 30;
$extra_arg = 10;
var_dump( date_time_set($datetime, $hour, $min, $sec, $extra_arg) );

echo "\n-- Testing date_time_set() function with an invalid values for \$object argument --\n";
$invalid_obj = new stdClass();
var_dump( date_time_set($invalid_obj, $hour, $min, $sec) );  
$invalid_obj = 10;
var_dump( date_time_set($invalid_obj, $hour, $min, $sec) );
$invalid_obj = null;
var_dump( date_time_set($invalid_obj, $hour, $min, $sec) ); 
?>
===DONE===