<?php
/* Prototype  : DateTime date_create  ([ string $time  [, DateTimeZone $timezone  ]] )
 * Description: Returns new DateTime object
 * Source code: ext/date/php_date.c
 * Alias to functions: DateTime::__construct
 */

echo "*** Testing date_create() : error conditions ***\n";

echo "\n-- Testing date_create() function with more than expected no. of arguments --\n";
$time = "GMT";
$timezone  = timezone_open("GMT");
$extra_arg = 99;
var_dump( date_create($time, $timezone, $extra_arg) );

?>
===DONE===