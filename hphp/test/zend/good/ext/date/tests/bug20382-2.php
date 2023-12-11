<?hh
<<__EntryPoint>> function main(): void {
$tests = vec[
	vec["Europe/Andorra",     17, 17, 17, 1, 24764, 1970],
	vec["Asia/Dubai",         17, 17, 17, 1, 1, 1970],
	vec["Asia/Kabul",         17, 17, 17, 1, 1, 1970],
	vec["America/Antigua",    17, 17, 17, 1, 1, 1970],
	vec["America/Anguilla",   17, 17, 17, 1, 1, 1970],
	vec["Europe/Tirane",      17, 17, 17, 1, 4849, 1970],
	vec["Asia/Yerevan",       17, 17, 17, 1, 24764, 1970],
	vec["America/Curacao",    17, 17, 17, 1, 1, 1970],
	vec["Africa/Luanda",      17, 17, 17, 1, 1, 1970],
	vec["Antarctica/McMurdo", 17, 17, 17, 1, 24743, 1970],
	vec["Australia/Adelaide", 17, 17, 17, 1, 1, 1971],
	vec["Australia/Darwin",   17, 17, 17, 1, 88, 1971],
	vec["Australia/Perth",    17, 17, 17, 1, 1, 1971],
	vec["America/Aruba",      17, 17, 17, 1, 88, 1971],
	vec["Asia/Baku",          17, 17, 17, 1, 1, 1971],
	vec["Europe/Sarajevo",    17, 17, 17, 1, 1, 1971],
	vec["America/Barbados",   17, 17, 17, 1, 1, 1971],
	vec["Asia/Dacca",         17, 17, 17, 1, 1, 1971],
	vec["Europe/Brussels",    17, 17, 17, 1, 1, 1971],
	vec["Africa/Ouagadougou", 17, 17, 17, 1, 88, 1971],
	vec["Europe/Tirane",      17, 17, 17, 1, 4849, 1970],
	vec["America/Buenos_Aires", 17, 17, 17, 1, 1734, 1970],
	vec["America/Rosario",    17, 17, 17, 1, 1734, 1970],
	vec["Europe/Vienna",      17, 17, 17, 1, 3743, 1970],
	vec["Asia/Baku",          17, 17, 17, 1, 9490, 1970],
];

foreach ($tests as $test) {
	date_default_timezone_set($test[0]);
	print "{$test[0]}\n";
	array_shift(inout $test);
	$timestamp = call_user_func_array(mktime<>, $test);

	print "ts     = ". date("l Y-m-d H:i:s T", $timestamp). "\n";
	$strtotime_tstamp = strtotime("first monday", $timestamp);
	print "result = ".date("l Y-m-d H:i:s T", $strtotime_tstamp)."\n";
	print "wanted = Monday            00:00:00\n\n";
}
}
