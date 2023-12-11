<?hh
<<__EntryPoint>> function main(): void {
date_default_timezone_set("Europe/Oslo");
$time = time();

$constants = vec[
	'DATE_ATOM',
	'DATE_COOKIE',
	'DATE_ISO8601',
	'DATE_RFC822',
	'DATE_RFC850',
	'DATE_RFC1036',
	'DATE_RFC1123',
	'DATE_RFC2822',
	'DATE_RFC3339',
	'DATE_RSS',
	'DATE_W3C'
];


foreach ($constants as $const) {
	echo "$const:\t";
	echo ((strtotime(date(constant($const), $time)) === $time) ? "OK" : "FAIL") . "\n";
}
}
