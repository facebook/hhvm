<?php
/* Prototype  : int timezone_offset_get  ( DateTimeZone $object  , DateTime $datetime  )
 * Description: Returns the timezone offset from GMT
 * Source code: ext/date/php_date.c
 * Alias to functions: DateTimeZone::getOffset
 */
 
//Set the default time zone 
date_default_timezone_set("GMT");
$tz = timezone_open("Europe/London");
$date = date_create("GMT");
 
echo "*** Testing timezone_offset_get() : error conditions ***\n";

echo "\n-- Testing timezone_offset_get() function with zero arguments --\n";
var_dump( timezone_offset_get() );

echo "\n-- Testing timezone_offset_get() function with less than expected no. of arguments --\n";
var_dump( timezone_offset_get($tz) );

echo "\n-- Testing timezone_offset_get() function with more than expected no. of arguments --\n";
$extra_arg = 99;
var_dump( timezone_offset_get($tz, $date, $extra_arg) );

echo "\n-- Testing timezone_offset_get() function with an invalid values for \$object argument --\n";
$invalid_obj = new stdClass();
var_dump( timezone_offset_get($invalid_obj, $date) );  
$invalid_obj = 10;
var_dump( timezone_offset_get($invalid_obj, $date) );
$invalid_obj = null;
var_dump( timezone_offset_get($invalid_obj, $date) );

echo "\n-- Testing timezone_offset_get() function with an invalid values for \$datetime argument --\n";
$invalid_obj = new stdClass();
var_dump( timezone_offset_get($tz, $invalid_obj) );  
$invalid_obj = 10;
var_dump( timezone_offset_get($tz, $invalid_obj) );
$invalid_obj = null;
var_dump( timezone_offset_get($tz, $invalid_obj) );  
?>
===DONE===