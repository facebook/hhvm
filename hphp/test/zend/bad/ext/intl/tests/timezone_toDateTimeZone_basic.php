<?php
ini_set("intl.error_level", E_WARNING);
ini_set("intl.default_locale", "nl");
date_default_timezone_set('Europe/Lisbon');

function do_test(IntlTimeZone $tz, $proc = false) {
	var_dump($tz->getID(), $tz->getRawOffset());
	if (!$proc)
		$dtz = $tz->toDateTimeZone();
	else
		$dtz = intltz_to_date_time_zone($tz);
	var_dump($dtz->getName(), $dtz->getOffset(new DateTime('2012-01-01 00:00:00')));
}

do_test(IntlTimeZone::createTimeZone('CET'));
do_test(IntlTimeZone::createTimeZone('Europe/Amsterdam'));
do_test(IntlTimeZone::createTimeZone('GMT+0405'), true);
