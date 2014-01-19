<?php

date_default_timezone_set('America/New_York');
$tz = new DateTimeZone('America/New_York');
$date_format = 'Y-m-d H:i:s T e';
$interval_format = 'P%dDT%hH';

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