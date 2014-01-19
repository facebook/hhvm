<?php
date_default_timezone_set("UTC");

$tests = array(
	'12am', '1am', '1pm',
	'12a.m.', '1a.m.', '1p.m.',
	'12:00am', '1:00am', '1:00pm',
	'12:00a.m.', '1:00a.m.', '1:00p.m.'
);

foreach ($tests as $test) {
	$t = strtotime("2005-12-22 ". $test);
	printf("%-10s => %s\n", $test, date(DATE_ISO8601, $t));
}

?>