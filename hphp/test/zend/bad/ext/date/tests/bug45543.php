<?php
$test_dates = array(
	'2008-01-01 12:00:00 PDT',
	'2008-01-01 12:00:00 +02:00',
);

foreach ($test_dates as $test_date)
{
	$d1 = new DateTime($test_date);
	$d2 = new DateTime('2008-01-01 12:00:00 UTC');
	echo $d1->format(DATE_ISO8601), PHP_EOL;
	echo $d2->format(DATE_ISO8601), PHP_EOL;
	$tz = $d1->getTimeZone();
	$d2->setTimeZone($tz);
	echo $d1->format(DATE_ISO8601), PHP_EOL;
	echo $d2->format(DATE_ISO8601), PHP_EOL;
	echo PHP_EOL;
}
