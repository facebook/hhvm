<?hh
<<__EntryPoint>> function main(): void {
date_default_timezone_set('UTC');

$base_time = 1204200000; // 28 Feb 2008 12:00:00

$offsets = vec[
	// offset around a day
	'80412 seconds',
	'86399 seconds',
	'86400 seconds',
	'86401 seconds',
	'112913 seconds',

	// offset around 7 days
	'134 hours',
	'167 hours',
	'168 hours',
	'169 hours',
	'183 hours',

	// offset around 6 months
	'178 days',
	'179 days',
	'180 days',
	'183 days',
	'184 days',

	// offset around 10 years
	'115 months',
	'119 months',
	'120 months',
	'121 months',
	'128 months',

	// offset around 25 years (can't do much more reliably with strtotime)
	'24 years',
	'25 years',
	'26 years'
];

foreach ($offsets as $offset) {
	foreach (vec['+', '-'] as $direction) {
		echo "$direction$offset: " . date(DATE_ISO8601, strtotime("$direction$offset", $base_time)) . "\n";
	}
}
}
