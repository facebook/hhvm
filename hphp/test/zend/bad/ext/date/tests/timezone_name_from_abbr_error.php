<?php
/* Prototype  : string timezone_name_from_abbr  ( string $abbr  [, int $gmtOffset= -1  [, int $isdst= -1  ]] )
 * Description: Returns the timezone name from abbrevation
 * Source code: ext/date/php_date.c
 * Alias to functions: 
 */

error_reporting(E_ALL | E_STRICT);

//Set the default time zone 
date_default_timezone_set("Europe/London");

echo "*** Testing timezone_name_from_abbr() : error conditions ***\n";

echo "\n-- Testing timezone_name_from_abbr() function with Zero arguments --\n";
var_dump( timezone_name_from_abbr() );

echo "\n-- Testing timezone_name_from_abbr() function with more than expected no. of arguments --\n";
$abbr = 10;
$gmtOffset = 30;
$isdst = 45;
$extra_arg = 10;
var_dump( timezone_name_from_abbr($abbr, $gmtOffset, $isdst, $extra_arg) );

?>
===DONE===