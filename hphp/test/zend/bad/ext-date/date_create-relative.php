<?php

date_default_timezone_set('UTC');

if (!defined('PHP_INT_MIN')) {
	define('PHP_INT_MIN', intval(-PHP_INT_MAX - 1));
}

$base_time = '28 Feb 2008 12:00:00'; 

// Most offsets tested in strtotime-relative.phpt. These are tests for dates outside the 32-bit range.
$offsets = array(
	// around 10 leap year periods (4000 years) in days
	'1460000 days',
	'1460969 days',
	'1460970 days',
	'1460971 days',
	'1462970 days',
	
	// around 1 leap year period in years
	'398 years',
	'399 years',
	'400 years',
	'401 years',
	
	// around 40000 years
	'39755 years',
	'39999 years',
	'40000 years',
	'40001 years',
	'41010 years',
	
	// bigger than int (32-bit)
	'10000000000 seconds',
	'10000000000 minutes',
	'10000000000 hours',
	'10000000000 days',
	'10000000000 months',
	'10000000000 years',
);

foreach ($offsets AS $offset) {
	foreach (array('+', '-') AS $direction) {
		$dt = date_create("$base_time $direction$offset");
		echo "$direction$offset: " . date_format($dt, DATE_ISO8601) . "\n";
	}
}

?>