<?php
/* Prototype  : public DateTime DateTime::setDate  ( int $year  , int $month  , int $day  )
 * Description: Resets the current date of the DateTime object to a different date. 
 * Source code: ext/date/php_date.c
 * Alias to functions: date_date_set()
 */
 
date_default_timezone_set("Europe/London");

echo "*** Testing DateTime::setDate() : error conditions ***\n";

$datetime = new DateTime("2009-01-30 19:34:10");

echo "\n-- Testing DateTime::setDate() function with zero arguments --\n";
var_dump( $datetime->setDate() );

echo "\n-- Testing DateTime::setDate() function with less than expected no. of arguments --\n";
$year = 2009;
$month = 1;
$day = 30;
var_dump( $datetime->setDate($year) );
var_dump( $datetime->setDate($year, $month) );

echo "\n-- Testing DateTime::setDate() function with more than expected no. of arguments --\n";
$extra_arg = 10;
var_dump( $datetime->setDate($year, $month, $day, $extra_arg) );

?>
===DONE===
