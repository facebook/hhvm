<?php

$tests = array(
	array("Europe/Andorra",     17, 17, 17, 1, 24764, 1970),
	array("Asia/Dubai",         17, 17, 17, 1, 1, 1970),
	array("Asia/Kabul",         17, 17, 17, 1, 1, 1970),
	array("America/Antigua",    17, 17, 17, 1, 1, 1970),
	array("America/Anguilla",   17, 17, 17, 1, 1, 1970),
	array("Europe/Tirane",      17, 17, 17, 1, 4849, 1970),
	array("Asia/Yerevan",       17, 17, 17, 1, 24764, 1970),
	array("America/Curacao",    17, 17, 17, 1, 1, 1970),
	array("Africa/Luanda",      17, 17, 17, 1, 1, 1970),
	array("Antarctica/McMurdo", 17, 17, 17, 1, 24743, 1970),
	array("Australia/Adelaide", 17, 17, 17, 1, 1, 1971),
	array("Australia/Darwin",   17, 17, 17, 1, 88, 1971),
	array("Australia/Perth",    17, 17, 17, 1, 1, 1971),
	array("America/Aruba",      17, 17, 17, 1, 88, 1971),
	array("Asia/Baku",          17, 17, 17, 1, 1, 1971),
	array("Europe/Sarajevo",    17, 17, 17, 1, 1, 1971),
	array("America/Barbados",   17, 17, 17, 1, 1, 1971),
	array("Asia/Dacca",         17, 17, 17, 1, 1, 1971),
	array("Europe/Brussels",    17, 17, 17, 1, 1, 1971),
	array("Africa/Ouagadougou", 17, 17, 17, 1, 88, 1971),
	array("Europe/Tirane",      17, 17, 17, 1, 4849, 1970),
	array("America/Buenos_Aires", 17, 17, 17, 1, 1734, 1970),
	array("America/Rosario",    17, 17, 17, 1, 1734, 1970),
	array("Europe/Vienna",      17, 17, 17, 1, 3743, 1970),
	array("Asia/Baku",          17, 17, 17, 1, 9490, 1970),
);

foreach ($tests as $test) {
	date_default_timezone_set($test[0]);
	print "{$test[0]}\n";
	array_shift($test);
	$timestamp = call_user_func_array('mktime', $test);

	print "ts     = ". date("l Y-m-d H:i:s T", $timestamp). "\n";
	$strtotime_tstamp = strtotime("first monday", $timestamp);
	print "result = ".date("l Y-m-d H:i:s T", $strtotime_tstamp)."\n";
	print "wanted = Monday            00:00:00\n\n";
}
?>