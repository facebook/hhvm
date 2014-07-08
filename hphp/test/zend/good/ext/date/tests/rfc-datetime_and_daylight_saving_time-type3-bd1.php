<?php

date_default_timezone_set('America/New_York');
$date_format = 'Y-m-d H:i:s T e';
$interval_format = 'P%dDT%hH';

/*
 * Backward Transitions, diff().
 */

$end   = new DateTime('2010-11-07 05:30:00');
$start = new DateTime('2010-11-06 04:30:00');
echo 'bd1 ' . $end->format($date_format) . ' - ' . $start->format($date_format)
	. ' = ' . $start->diff($end)->format($interval_format) . "\n";

$end   = new DateTime('2010-11-07 04:30:00');
$start = new DateTime('2010-11-06 04:30:00');
echo 'bd2 ' . $end->format($date_format) . ' - ' . $start->format($date_format)
	. ' = ' . $start->diff($end)->format($interval_format) . "\n";

$end   = new DateTime('2010-11-07 03:30:00');
$start = new DateTime('2010-11-06 04:30:00');
echo 'bd3 ' . $end->format($date_format) . ' - ' . $start->format($date_format)
	. ' = ' . $start->diff($end)->format($interval_format) . "\n";

$end   = new DateTime('2010-11-07 02:30:00');
$start = new DateTime('2010-11-06 04:30:00');
echo 'bd4 ' . $end->format($date_format) . ' - ' . $start->format($date_format)
	. ' = ' . $start->diff($end)->format($interval_format) . "\n";

$end   = new DateTime('2010-11-07 01:30:00');
$start = new DateTime('2010-11-06 01:30:00');
echo 'bd7 ' . $end->format($date_format) . ' - ' . $start->format($date_format)
	. ' = ' . $start->diff($end)->format($interval_format) . "\n";

echo "\n";
?>