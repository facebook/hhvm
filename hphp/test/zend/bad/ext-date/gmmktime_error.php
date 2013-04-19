<?php
/* Prototype  : int gmmktime([int hour [, int min [, int sec [, int mon [, int day [, int year]]]]]])
 * Description: Get UNIX timestamp for a GMT date 
 * Source code: ext/date/php_date.c
 * Alias to functions: 
 */

echo "*** Testing gmmktime() : error conditions ***\n";

//Test gmmktime with one more than the expected number of arguments
echo "\n-- Testing gmmktime() function with more than expected no. of arguments --\n";
$hour = 8;
$min = 8;
$sec = 8;
$mon = 8;
$day = 8;
$year = 2008;
$extra_arg1 = 10;
$extra_arg2 = 10;

var_dump( gmmktime($hour, $min, $sec, $mon, $day, $year, $extra_arg1) );

var_dump( gmmktime($hour, $min, $sec, $mon, $day, $year, $extra_arg1, $extra_arg2) );
?>
===DONE===