<?php
/* Prototype  : array timezone_transitions_get  ( DateTimeZone $object, [ int $timestamp_begin  [, int $timestamp_end  ]]  )
 * Description: Returns all transitions for the timezone
 * Source code: ext/date/php_date.c
 * Alias to functions: DateTimeZone::getTransitions()
 */
 
//Set the default time zone 
date_default_timezone_set("GMT");
$tz = timezone_open("Europe/London");
 
echo "*** Testing timezone_transitions_get() : error conditions ***\n";

echo "\n-- Testing timezone_transitions_get() function with zero arguments --\n";
var_dump( timezone_transitions_get() );

echo "\n-- Testing timezone_transitions_get() function with more than expected no. of arguments --\n";
$timestamp_begin = mktime(0, 0, 0, 1, 1, 1972);
$timestamp_end = mktime(0, 0, 0, 1, 1, 1975);
$extra_arg = 99;
var_dump( timezone_transitions_get($tz, $timestamp_begin, $timestamp_end, $extra_arg) );

echo "\n-- Testing timezone_transitions_get() function with an invalid values for \$object argument --\n";
$invalid_obj = new stdClass();
var_dump( timezone_transitions_get($invalid_obj) );  
$invalid_obj = 10;
var_dump( timezone_transitions_get($invalid_obj) );
$invalid_obj = null;
var_dump( timezone_transitions_get($invalid_obj) ); 
?>
===DONE===