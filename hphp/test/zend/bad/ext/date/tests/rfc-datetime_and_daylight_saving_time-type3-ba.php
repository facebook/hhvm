<?php

date_default_timezone_set('America/New_York');
$date_format = 'Y-m-d H:i:s T e';
$interval_format = 'P%dDT%hH';

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

?>