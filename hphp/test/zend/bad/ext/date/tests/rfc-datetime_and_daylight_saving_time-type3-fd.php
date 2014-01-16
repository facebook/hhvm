<?php

date_default_timezone_set('America/New_York');
$date_format = 'Y-m-d H:i:s T e';
$interval_format = 'P%dDT%hH';

/*
 * Forward Transitions, diff().
 */

$end   = new DateTime('2010-03-14 03:00:00');
$start = new DateTime('2010-03-14 01:59:59');
echo 'fd1 ' . $end->format($date_format) . ' - ' . $start->format($date_format)
	. ' = ' . $start->diff($end)->format('PT%hH%iM%sS') . "\n";

$end   = new DateTime('2010-03-14 04:30:00');
$start = new DateTime('2010-03-13 04:30:00');
echo 'fd2 ' . $end->format($date_format) . ' - ' . $start->format($date_format)
	. ' = ' . $start->diff($end)->format($interval_format) . "\n";

$end   = new DateTime('2010-03-14 03:30:00');
$start = new DateTime('2010-03-13 04:30:00');
echo 'fd3 ' . $end->format($date_format) . ' - ' . $start->format($date_format)
	. ' = ' . $start->diff($end)->format($interval_format) . "\n";

$end   = new DateTime('2010-03-14 01:30:00');
$start = new DateTime('2010-03-13 04:30:00');
echo 'fd4 ' . $end->format($date_format) . ' - ' . $start->format($date_format)
	. ' = ' . $start->diff($end)->format($interval_format) . "\n";

$end   = new DateTime('2010-03-14 01:30:00');
$start = new DateTime('2010-03-13 01:30:00');
echo 'fd5 ' . $end->format($date_format) . ' - ' . $start->format($date_format)
	. ' = ' . $start->diff($end)->format($interval_format) . "\n";

$end   = new DateTime('2010-03-14 03:30:00');
$start = new DateTime('2010-03-13 03:30:00');
echo 'fd6 ' . $end->format($date_format) . ' - ' . $start->format($date_format)
	. ' = ' . $start->diff($end)->format($interval_format) . "\n";

$end   = new DateTime('2010-03-14 03:30:00');
$start = new DateTime('2010-03-13 02:30:00');
echo 'fd7 ' . $end->format($date_format) . ' - ' . $start->format($date_format)
	. ' = ' . $start->diff($end)->format($interval_format) . "\n";
?>