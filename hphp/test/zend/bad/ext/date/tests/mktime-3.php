<?php
$tzs = array("America/Toronto", "Europe/Oslo");
$years = array(0, 69, 70, 71, 99, 100, 105, 1900, 1901, 1902, 1999, 2000, 2001);

foreach ($tzs as $tz) {
	echo $tz, "\n";
	date_default_timezone_set($tz);
	foreach ($years as $year) {
		printf("Y: %4d - ", $year);
		$ret = mktime(1, 1, 1, 1, 1, $year);
		if ($ret == FALSE) {
			echo "out of range\n";
		} else {
			echo date("F ".DATE_ISO8601, $ret), "\n";
		}
	}
	echo "\n";
}
?>
===Done===