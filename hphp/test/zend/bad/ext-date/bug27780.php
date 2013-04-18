<?php
$timezones = array (
	"America/Chicago", "Europe/Amsterdam", "Asia/Jerusalem",
	"Asia/Singapore", "America/Sao_Paulo"
);

$timestrings = array (
	"2004-04-07 00:00:00 -2 months +7 days +23 hours +59 minutes +59 seconds",
	"2004-04-07 00:00:00 -2 months +7 days +23 hours +59 minutes +60 seconds",
	"2004-04-07 00:00:00 -2 months +7 days +23 hours +59 minutes +61 seconds",
	"2004-04-07 00:00:00 -21 days",
	"2004-04-07 00:00:00 11 days ago",
	"2004-04-07 00:00:00 -10 day +2 hours",
	"2004-04-07 00:00:00 -1 day",
	"2004-04-07 00:00:00",
	"2004-04-07 00:00:00 +1 hour",
	"2004-04-07 00:00:00 +2 hour",
	"2004-04-07 00:00:00 +1 day",
	"2004-04-07 00:00:00 1 day",
	"2004-04-07 00:00:00 +21 days",
);

foreach ($timezones as $timezone) {
	date_default_timezone_set($timezone);
	echo $timezone, "\n";

	foreach ($timestrings as $timestring) {
		$time = strtotime($timestring);

		echo $time, strftime(" [%Y-%m-%d %H:%M:%S %Z]", $time), " [$timestring]\n";
	}

	echo "\n";
}
?>