<?php
/* Prototype  : DateTimeZone timezone_open  ( string $timezone  )
 * Description: Returns new DateTimeZone object
 * Source code: ext/date/php_date.c
 * Alias to functions: DateTime::__construct()
 */
 
echo "*** Testing timezone_open() : error conditions ***\n";

echo "\n-- Testing timezone_open() function with zero arguments --\n";
var_dump( timezone_open() );

echo "\n-- Testing timezone_open() function with more than expected no. of arguments --\n";
$time = "GMT";
$extra_arg = 99;
var_dump( timezone_open($time, $extra_arg) );

?>
===DONE===