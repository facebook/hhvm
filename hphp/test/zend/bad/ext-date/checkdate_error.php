<?php
/* Prototype  : bool checkdate  ( int $month  , int $day  , int $year  )
 * Description: Validate a Gregorian date
 * Source code: ext/date/php_date.c
 * Alias to functions: 
 */

echo "*** Testing checkdate() : error conditions ***\n";

//Set the default time zone 
date_default_timezone_set("America/Chicago");

$arg_0 = 1;
$arg_1 = 1;
$arg_2 = 1;
$extra_arg = 1;

echo "\n-- Testing checkdate() function with more than expected no. of arguments --\n";
var_dump (checkdate($arg_0, $arg_1, $arg_2, $extra_arg));

echo "\n-- Testing checkdate() function with less than expected no. of arguments --\n";
var_dump (checkdate());
var_dump (checkdate($arg_0));
var_dump (checkdate($arg_0, $arg_1));

?>
===DONE=== 