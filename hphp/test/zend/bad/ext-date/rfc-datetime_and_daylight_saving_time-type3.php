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

echo "\n";

/*
 * Forward Transitions, add().
 */

$start = new DateTime('2010-03-14 01:59:59');
$interval_spec = 'PT1S';
$interval = new DateInterval($interval_spec);
echo 'fa1 ' . $start->format($date_format) . " + $interval_spec = "
	. $start->add($interval)->format($date_format) . "\n";

$start = new DateTime('2010-03-13 04:30:00');
$interval_spec = 'P1D';
$interval = new DateInterval($interval_spec);
echo 'fa2 ' . $start->format($date_format) . " + $interval_spec = "
	. $start->add($interval)->format($date_format) . "\n";

$start = new DateTime('2010-03-13 04:30:00');
$interval_spec = 'PT22H';
$interval = new DateInterval($interval_spec);
echo 'fa3 ' . $start->format($date_format) . " + $interval_spec = "
	. $start->add($interval)->format($date_format) . "\n";

$start = new DateTime('2010-03-13 04:30:00');
$interval_spec = 'PT21H';
$interval = new DateInterval($interval_spec);
echo 'fa4 ' . $start->format($date_format) . " + $interval_spec = "
	. $start->add($interval)->format($date_format) . "\n";

$start = new DateTime('2010-03-13 01:30:00');
$interval_spec = 'P1D';
$interval = new DateInterval($interval_spec);
echo 'fa5 ' . $start->format($date_format) . " + $interval_spec = "
	. $start->add($interval)->format($date_format) . "\n";

$start = new DateTime('2010-03-13 02:30:00');
$interval_spec = 'P1D';
$interval = new DateInterval($interval_spec);
echo 'fa6 ' . $start->format($date_format) . " + $interval_spec = "
	. $start->add($interval)->format($date_format) . "\n";

echo "\n";

/*
 * Forward Transitions, sub().
 */

$end   = new DateTime('2010-03-14 03:00:00');
$interval_spec = 'PT1S';
$interval = new DateInterval($interval_spec);
echo 'fs1 ' . $end->format($date_format) . " - $interval_spec = "
	. $end->sub($interval)->format($date_format) . "\n";

$end   = new DateTime('2010-03-14 04:30:00');
$interval_spec = 'P1D';
$interval = new DateInterval($interval_spec);
echo 'fs2 ' . $end->format($date_format) . " - $interval_spec = "
	. $end->sub($interval)->format($date_format) . "\n";

$end   = new DateTime('2010-03-14 03:30:00');
$interval_spec = 'PT22H';
$interval = new DateInterval($interval_spec);
echo 'fs3 ' . $end->format($date_format) . " - $interval_spec = "
	. $end->sub($interval)->format($date_format) . "\n";

$end   = new DateTime('2010-03-14 01:30:00');
$interval_spec = 'PT21H';
$interval = new DateInterval($interval_spec);
echo 'fs4 ' . $end->format($date_format) . " - $interval_spec = "
	. $end->sub($interval)->format($date_format) . "\n";

$end   = new DateTime('2010-03-14 01:30:00');
$interval_spec = 'P1D';
$interval = new DateInterval($interval_spec);
echo 'fs5 ' . $end->format($date_format) . " - $interval_spec = "
	. $end->sub($interval)->format($date_format) . "\n";

$end   = new DateTime('2010-03-15 03:30:00');
$interval_spec = 'P1D';
$interval = new DateInterval($interval_spec);
echo 'fs6 ' . $end->format($date_format) . " - $interval_spec = "
	. $end->sub($interval)->format($date_format) . "\n";

$end   = new DateTime('2010-03-15 02:30:00');
$interval_spec = 'P1D';
$interval = new DateInterval($interval_spec);
echo 'fs7 ' . $end->format($date_format) . " - $interval_spec = "
	. $end->sub($interval)->format($date_format) . "\n";

echo "\n";


/*
 * For backward transitions, must create objects with zone type 2
 * where specifying Daylight or Standard time is required
 * then converting them to zone type 3.
 */

$tz = new DateTimeZone('America/New_York');

/*
 * Backward Transitions, diff().
 */

$end   = new DateTime('2010-11-07 01:00:00 EST');
$end->setTimeZone($tz);
$start = new DateTime('2010-11-07 01:59:59');
echo 'bd1 ' . $end->format($date_format) . ' - ' . $start->format($date_format)
	. ' = ' . $start->diff($end)->format('PT%hH%iM%sS') . "\n";

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

$end   = new DateTime('2010-11-07 01:30:00');
$start = new DateTime('2010-11-06 01:30:00');
echo 'bd7 ' . $end->format($date_format) . ' - ' . $start->format($date_format)
	. ' = ' . $start->diff($end)->format($interval_format) . "\n";

$end   = new DateTime('2010-11-07 01:30:00 EST');
$end->setTimeZone($tz);
$start = new DateTime('2010-11-06 01:30:00');
echo 'bd8 ' . $end->format($date_format) . ' - ' . $start->format($date_format)
	. ' = ' . $start->diff($end)->format($interval_format) . "\n";

echo "\n";

/*
 * Backward Transitions, add().
 */

$start = new DateTime('2010-11-07 01:59:59');
$interval_spec = 'PT1S';
$interval = new DateInterval($interval_spec);
echo 'ba1 ' . $start->format($date_format) . " + $interval_spec = "
	. $start->add($interval)->format($date_format) . "\n";

$start = new DateTime('2010-11-06 04:30:00');
$interval_spec = 'P1D';
$interval = new DateInterval($interval_spec);
echo 'ba2 ' . $start->format($date_format) . " + $interval_spec = "
	. $start->add($interval)->format($date_format) . "\n";

$start = new DateTime('2010-11-06 04:30:00');
$interval_spec = 'PT24H';
$interval = new DateInterval($interval_spec);
echo 'ba3 ' . $start->format($date_format) . " + $interval_spec = "
	. $start->add($interval)->format($date_format) . "\n";

$start = new DateTime('2010-11-06 04:30:00');
$interval_spec = 'PT23H';
$interval = new DateInterval($interval_spec);
echo 'ba4 ' . $start->format($date_format) . " + $interval_spec = "
	. $start->add($interval)->format($date_format) . "\n";

$start = new DateTime('2010-11-06 04:30:00');
$interval_spec = 'PT22H';
$interval = new DateInterval($interval_spec);
echo 'ba5 ' . $start->format($date_format) . " + $interval_spec = "
	. $start->add($interval)->format($date_format) . "\n";

$start = new DateTime('2010-11-06 04:30:00');
$interval_spec = 'PT21H';
$interval = new DateInterval($interval_spec);
echo 'ba6 ' . $start->format($date_format) . " + $interval_spec = "
	. $start->add($interval)->format($date_format) . "\n";

$start = new DateTime('2010-11-06 01:30:00');
$interval_spec = 'P1D';
$interval = new DateInterval($interval_spec);
echo 'ba7 ' . $start->format($date_format) . " + $interval_spec = "
	. $start->add($interval)->format($date_format) . "\n";

$start = new DateTime('2010-11-06 01:30:00');
$interval_spec = 'P1DT1H';
$interval = new DateInterval($interval_spec);
echo 'ba8 ' . $start->format($date_format) . " + $interval_spec = "
	. $start->add($interval)->format($date_format) . "\n";

$start = new DateTime('2010-11-06 04:30:00');
$interval_spec = 'PT25H';
$interval = new DateInterval($interval_spec);
echo 'ba9 ' . $start->format($date_format) . " + $interval_spec = "
	. $start->add($interval)->format($date_format) . "\n";

$start = new DateTime('2010-11-06 03:30:00');
$interval_spec = 'P1D';
$interval = new DateInterval($interval_spec);
echo 'ba10 ' . $start->format($date_format) . " + $interval_spec = "
	. $start->add($interval)->format($date_format) . "\n";

$start = new DateTime('2010-11-06 02:30:00');
$interval_spec = 'P1D';
$interval = new DateInterval($interval_spec);
echo 'ba11 ' . $start->format($date_format) . " + $interval_spec = "
	. $start->add($interval)->format($date_format) . "\n";

echo "\n";

/*
 * Backward Transitions, sub().
 */

$end   = new DateTime('2010-11-07 01:00:00 EST');
$end->setTimeZone($tz);
$interval_spec = 'PT1S';
$interval = new DateInterval($interval_spec);
echo 'bs1 ' . $end->format($date_format) . " - $interval_spec = "
	. $end->sub($interval)->format($date_format) . "\n";

$end   = new DateTime('2010-11-07 04:30:00');
$interval_spec = 'P1D';
$interval = new DateInterval($interval_spec);
echo 'bs2 ' . $end->format($date_format) . " - $interval_spec = "
	. $end->sub($interval)->format($date_format) . "\n";

$end   = new DateTime('2010-11-07 03:30:00');
$interval_spec = 'PT24H';
$interval = new DateInterval($interval_spec);
echo 'bs3 ' . $end->format($date_format) . " - $interval_spec = "
	. $end->sub($interval)->format($date_format) . "\n";

$end   = new DateTime('2010-11-07 02:30:00');
$interval_spec = 'PT23H';
$interval = new DateInterval($interval_spec);
echo 'bs4 ' . $end->format($date_format) . " - $interval_spec = "
	. $end->sub($interval)->format($date_format) . "\n";

$end   = new DateTime('2010-11-07 01:30:00 EST');
$end->setTimeZone($tz);
$interval_spec = 'PT22H';
$interval = new DateInterval($interval_spec);
echo 'bs5 ' . $end->format($date_format) . " - $interval_spec = "
	. $end->sub($interval)->format($date_format) . "\n";

$end   = new DateTime('2010-11-07 01:30:00 EDT');
$end->setTimeZone($tz);
$interval_spec = 'PT21H';
$interval = new DateInterval($interval_spec);
echo 'bs6 ' . $end->format($date_format) . " - $interval_spec = "
	. $end->sub($interval)->format($date_format) . "\n";

$end   = new DateTime('2010-11-07 01:30:00');
$interval_spec = 'P1D';
$interval = new DateInterval($interval_spec);
echo 'bs7 ' . $end->format($date_format) . " - $interval_spec = "
	. $end->sub($interval)->format($date_format) . "\n";

$end   = new DateTime('2010-11-07 01:30:00 EST');
$end->setTimeZone($tz);
$interval_spec = 'P1DT1H';
$interval = new DateInterval($interval_spec);
echo 'bs8 ' . $end->format($date_format) . " - $interval_spec = "
	. $end->sub($interval)->format($date_format) . "\n";

$end   = new DateTime('2010-11-07 03:30:00');
$interval_spec = 'P1D';
$interval = new DateInterval($interval_spec);
echo 'bs9 ' . $end->format($date_format) . " - $interval_spec = "
	. $end->sub($interval)->format($date_format) . "\n";

$end   = new DateTime('2010-11-07 02:30:00');
$interval_spec = 'P1D';
$interval = new DateInterval($interval_spec);
echo 'bs10 ' . $end->format($date_format) . " - $interval_spec = "
	. $end->sub($interval)->format($date_format) . "\n";

?>