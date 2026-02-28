<?hh
<<__EntryPoint>> function main(): void {
date_default_timezone_set("UTC");
$tz = vec["UTC", "Asia/Jerusalem", "America/Chicago", "Europe/London"];
$t = mktime(0, 0, 0, 6, 27, 2006);

foreach ($tz as $zone) {
	date_default_timezone_set($zone);

	var_dump(date("w", $t));
	var_dump(date("z", $t));
	var_dump(date("n", $t));
	var_dump(date("t", $t));
	var_dump(date("L", $t));
	var_dump(date("a", $t));
	var_dump(date("B", $t));
	var_dump(date("g", $t));
	var_dump(date("G", $t));
	var_dump(date("Z", $t));
	var_dump(date("U", $t));
}

echo "Done\n";
}
