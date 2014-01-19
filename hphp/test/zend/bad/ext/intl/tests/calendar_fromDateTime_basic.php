<?php
ini_set("intl.error_level", E_WARNING);
ini_set("intl.default_locale", "nl_NL");
date_default_timezone_set('Europe/Lisbon');

$cal = IntlCalendar::fromDateTime('2012-01-01 00:00:00 Europe/Rome');
var_dump(
	$cal->getTime(),
	strtotime('2012-01-01 00:00:00 Europe/Rome') * 1000.,
	$cal->getTimeZone()->getID(),
	$cal->getLocale(1)
);
echo "\n";

$cal = IntlCalendar::fromDateTime(new DateTime('2012-01-01 00:00:00 PST'), "pt_PT");
var_dump(
	$cal->getTime(),
	strtotime('2012-01-01 00:00:00 PST') * 1000.,
	$cal->getTimeZone()->getID(),
	$cal->getLocale(1)
);

echo "\n";

$cal = intlcal_from_date_time(new DateTime('2012-01-01 00:00:00 +03:40'));
var_dump(
	$cal->getTime(),
	strtotime('2012-01-01 00:00:00 +03:40') * 1000.,
	$cal->getTimeZone()->getID()
);
