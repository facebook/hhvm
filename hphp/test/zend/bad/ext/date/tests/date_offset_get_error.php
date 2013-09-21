<?php

/* Prototype  : int date_offset_get  ( DateTimeInterface $object  )
 * Description: Returns the daylight saving time offset
 * Source code: ext/date/php_date.c
 * Alias to functions:  DateTimeInterface::getOffset
 */
 
 //Set the default time zone 
date_default_timezone_set("Europe/London");

echo "*** Testing date_offset_get() : error conditions ***\n";

echo "\n-- Testing date_offset_get() function with zero arguments --\n";
var_dump( date_offset_get() );

echo "\n-- Testing date_offset_get() function with more than expected no. of arguments --\n";
$datetime = date_create("2009-01-30 19:34:10");
$extra_arg = 30;
var_dump( date_offset_get($datetime, $extra_arg) );

echo "\n-- Testing date_offset_get() function with an invalid values for \$object argument --\n";
$invalid_obj = new stdClass();
var_dump( date_offset_get($invalid_obj) );  
$invalid_obj = 10;
var_dump( date_offset_get($invalid_obj) );
$invalid_obj = null;
var_dump( date_offset_get($invalid_obj) ); 
?>
===DONE===