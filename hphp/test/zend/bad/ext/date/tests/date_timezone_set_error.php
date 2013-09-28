<?php
/* Prototype  : DateTime date_timezone_set  ( DateTime $object  , DateTimeZone $timezone  )
 * Description: Sets the time zone for the DateTime object
 * Source code: ext/date/php_date.c
 * Alias to functions: DateTime::setTimezone
 */
 
date_default_timezone_set("UTC");

echo "*** Testing date_timezone_set() : error conditions ***\n";

echo "\n-- Testing date_timezone_set() function with zero arguments --\n";
var_dump( date_timezone_set() ); 

echo "\n-- Testing date_timezone_set() function with less than expected no. of arguments --\n";
$datetime = date_create("2009-01-30 17:57:32");
var_dump( date_timezone_set($datetime) ); 

echo "\n-- Testing date_timezone_set() function with more than expected no. of arguments --\n";
$timezone  = timezone_open("GMT");
$extra_arg = 99;
var_dump( date_timezone_set($datetime, $timezone, $extra_arg) );

echo "\n-- Testing date_timezone_set() function with an invalid values for \$object argument --\n";
$invalid_obj = new stdClass();
var_dump( date_timezone_set($invalid_obj, $timezone) );  
$invalid_obj = 10;
var_dump( date_timezone_set($invalid_obj, $timezone) );
$invalid_obj = null;
var_dump( date_timezone_set($invalid_obj, $timezone) );  
?>
===DONE===