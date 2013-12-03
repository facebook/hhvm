<?php

date_default_timezone_set('America/New_York');
$date_format = 'Y-m-d H:i:s T e';
$interval_format = 'P%dDT%hH';

/*
 * For backward transitions, must create objects with zone type 2
 * where specifying Daylight or Standard time is required
 * then converting them to zone type 3.
 */

$tz = new DateTimeZone('America/New_York');

/*
 * Backward Transitions, diff().
 */

$end   = new DateTime('2010-11-07 05:30:00');
$end->setTimeZone($tz);
$start = new DateTime('2010-11-06 04:30:59');
echo 'bd0 ' . $end->format($date_format) . ' - ' . $start->format($date_format)
	. ' = ' . $start->diff($end)->format('P%dDT%hH%iM%sS') . "\n";

$end   = new DateTime('2010-11-07 01:30:00 EST');
$end->setTimeZone($tz);
$start = new DateTime('2010-11-06 04:30:00');
echo 'bd5 ' . $end->format($date_format) . ' - ' . $start->format($date_format)
	. ' = ' . $start->diff($end)->format($interval_format) . "\n";

$end   = new DateTime('2010-11-07 01:30:00 EDT');
$end->setTimeZone($tz);
$start = new DateTime('2010-11-06 04:30:00');
echo 'bd6 ' . $end->format($date_format) . ' - ' . $start->format($date_format)
	. ' = ' . $start->diff($end)->format($interval_format) . "\n";

$end   = new DateTime('2010-11-07 01:30:00 EST');
$end->setTimeZone($tz);
$start = new DateTime('2010-11-06 01:30:00');
echo 'bd8 ' . $end->format($date_format) . ' - ' . $start->format($date_format)
	. ' = ' . $start->diff($end)->format($interval_format) . "\n";

echo "\n";
?>