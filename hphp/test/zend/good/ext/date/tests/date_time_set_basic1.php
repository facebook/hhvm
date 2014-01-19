<?php
/* Prototype  : DateTime date_time_set  ( DateTime $object  , int $hour  , int $minute  [, int $second  ] )
 * Description: Resets the current time of the DateTime object to a different time. 
 * Source code: ext/date/php_date.c
 * Alias to functions: DateTime::setTime
 */
 
 //Set the default time zone 
date_default_timezone_set("Europe/London");

echo "*** Testing date_time_set() : basic functionality ***\n";

// Create a DateTime object
$datetime = date_create("2009-01-31 15:14:10");

echo "Initial date: " . date_format($datetime, DATE_RFC2822) . "\n";

date_time_set($datetime, 17, 20);
echo "After modification1 " . date_format($datetime, DATE_RFC2822) . "\n";

date_time_set($datetime, 19, 05, 59);
echo "After modification2 " . date_format($datetime, DATE_RFC2822) . "\n";

date_time_set($datetime, 24, 10);
echo "After modification3 " . date_format($datetime, DATE_RFC2822) . "\n";

date_time_set($datetime, 47, 35, 47);
echo "After modification4 " . date_format($datetime, DATE_RFC2822) . "\n";

date_time_set($datetime, 54, 25);
echo "After modification5 " . date_format($datetime, DATE_RFC2822) . "\n";

?>
===DONE===