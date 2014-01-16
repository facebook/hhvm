<?php

date_default_timezone_set('America/New_York');
$date_format = 'Y-m-d H:i:s T e';
$interval_format = 'P%dDT%hH';

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
?>