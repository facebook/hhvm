<?php
	putenv("TZ=");
	$date1 = strtotime("2005-01-12 08:00:00");
	$date2 = strtotime("2005-07-12 08:00:00");
	date_default_timezone_set("America/Indiana/Knox");
	$date3 = strtotime("2005-01-12 08:00:00");
	$date4 = strtotime("2005-07-12 08:00:00");

	echo date_default_timezone_get(), "\n";
	echo date(DATE_ISO8601, $date1), "\n";
	echo date(DATE_ISO8601, $date2), "\n";
	echo date(DATE_ISO8601, $date3), "\n";
	echo date(DATE_ISO8601, $date4), "\n";
?>