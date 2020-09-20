<?hh
<<__EntryPoint>> function main(): void {
$tests = varray[
	varray["Europe/Andorra",     17, 17, 17, 1, 24764, 1970],
	varray["Asia/Dubai",         17, 17, 17, 1, 1, 1970],
	varray["Asia/Kabul",         17, 17, 17, 1, 1, 1970],
	varray["America/Antigua",    17, 17, 17, 1, 1, 1970],
	varray["America/Anguilla",   17, 17, 17, 1, 1, 1970],
	varray["Europe/Tirane",      17, 17, 17, 1, 4849, 1970],
	varray["Asia/Yerevan",       17, 17, 17, 1, 24764, 1970],
	varray["America/Curacao",    17, 17, 17, 1, 1, 1970],
	varray["Africa/Luanda",      17, 17, 17, 1, 1, 1970],
	varray["Antarctica/McMurdo", 17, 17, 17, 1, 24743, 1970],
	varray["Australia/Adelaide", 17, 17, 17, 1, 1, 1971],
	varray["Australia/Darwin",   17, 17, 17, 1, 88, 1971],
	varray["Australia/Perth",    17, 17, 17, 1, 1, 1971],
	varray["America/Aruba",      17, 17, 17, 1, 88, 1971],
	varray["Asia/Baku",          17, 17, 17, 1, 1, 1971],
	varray["Europe/Sarajevo",    17, 17, 17, 1, 1, 1971],
	varray["America/Barbados",   17, 17, 17, 1, 1, 1971],
	varray["Asia/Dacca",         17, 17, 17, 1, 1, 1971],
	varray["Europe/Brussels",    17, 17, 17, 1, 1, 1971],
	varray["Africa/Ouagadougou", 17, 17, 17, 1, 88, 1971],
	varray["Europe/Tirane",      17, 17, 17, 1, 4849, 1970],
	varray["America/Buenos_Aires", 17, 17, 17, 1, 1734, 1970],
	varray["America/Rosario",    17, 17, 17, 1, 1734, 1970],
	varray["Europe/Vienna",      17, 17, 17, 1, 3743, 1970],
	varray["Asia/Baku",          17, 17, 17, 1, 9490, 1970],
];

foreach ($tests as $test) {
	date_default_timezone_set($test[0]);
	print "{$test[0]}\n";
	array_shift(inout $test);
	$timestamp = call_user_func_array('mktime', $test);

	print "ts     = ". date("l Y-m-d H:i:s T", $timestamp). "\n";
	$strtotime_tstamp = strtotime("first monday", $timestamp);
	print "result = ".date("l Y-m-d H:i:s T", $strtotime_tstamp)."\n";
	print "wanted = Monday            00:00:00\n\n";
}
}
