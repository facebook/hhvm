<?php
/* Prototype  : array date_parse  ( string $date  ) 
 * Description: Returns associative array with detailed info about given date.
 * Source code: ext/date/php_date.c
 */
 
//Set the default time zone 
date_default_timezone_set("Europe/London");

echo "*** Testing date_parse() : error conditions ***\n";

echo "\n-- Testing date_parse() function with zero arguments --\n";
var_dump( date_parse() );

echo "\n-- Testing date_parse() function with more than expected no. of arguments --\n";
$date = "2009-02-27 10:00:00.5";
$extra_arg = 10;
var_dump( date_parse($date, $extra_arg) );

echo "\n-- Testing date_parse() function with unexpected characters in \$date argument --\n";
$invalid_date = "2OO9-02--27 10:00?00.5";
var_dump( date_parse($invalid_date) );  

?>
===DONE===