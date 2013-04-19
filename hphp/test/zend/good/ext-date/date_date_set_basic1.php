<?php
/* Prototype  : DateTime date_date_set  ( DateTime $object  , int $year  , int $month  , int $day  )
 * Description: Resets the current date of the DateTime object to a different date. 
 * Source code: ext/date/php_date.c
 * Alias to functions: DateTime::setDate
 */
 
 //Set the default time zone 
date_default_timezone_set("Europe/London");

echo "*** Testing date_date_set() : basic functionality ***\n";

$datetime = date_create("2009-01-30 19:34:10");

echo date_format($datetime, DATE_RFC2822) . "\n";

date_date_set($datetime, 2008, 02, 01);

echo date_format($datetime, DATE_RFC2822) . "\n";

?>
===DONE===