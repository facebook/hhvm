<?php
date_default_timezone_set('UTC');

/* Format: YYYYMMDDHHMMSS */
$d[] = '19970523091528';
$d[] = '20001231185859';
$d[] = '20800410101010'; // overflow..

foreach($d as $date) {
	$time = strtotime($date);

	if (is_integer($time)) {
		var_dump(date('r', $time));
	} else {
		var_dump($time);
	}
}
?>