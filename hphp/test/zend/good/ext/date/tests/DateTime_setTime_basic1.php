<?php
/* Prototype  : public DateTime DateTime::setTime  ( int $hour  , int $minute  [, int $second  ] )
 * Description: Resets the current time of the DateTime object to a different time. 
 * Source code: ext/date/php_date.c
 * Alias to functions: date_time_set
 */
 
 //Set the default time zone 
date_default_timezone_set("Europe/London");

echo "*** Testing DateTime::setTime() : basic functionality ***\n";

// Create a DateTime object
$datetime = new DateTime("2009-01-31 15:14:10");

echo "Initial date: " . $datetime ->format(DATE_RFC2822) . "\n";

$datetime->setTime(17, 20);
echo "After modification1 " . $datetime ->format(DATE_RFC2822) . "\n";

$datetime->setTime(19, 05, 59);
echo "After modification2 " . $datetime ->format(DATE_RFC2822) . "\n";

$datetime->setTime(24, 10);
echo "After modification3 " . $datetime ->format(DATE_RFC2822) . "\n";

$datetime->setTime(47, 35, 47);
echo "After modification4 " . $datetime ->format(DATE_RFC2822) . "\n";

$datetime->setTime(54, 25);
echo "After modification5 " . $datetime ->format(DATE_RFC2822) . "\n";

?>
===DONE===