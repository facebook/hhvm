<?php
ini_set("intl.error_level", E_WARNING);
ini_set("intl.default_locale", "nl");
date_default_timezone_set('Europe/Lisbon');

var_dump(IntlCalendar::fromDateTime());
var_dump(IntlCalendar::fromDateTime(0,1,2));

try {
IntlCalendar::fromDateTime("foobar");
} catch (Exception $e) {
	echo "threw exception, OK";
}
class A extends DateTime {
function __construct() {}
}

var_dump(IntlCalendar::fromDateTime(new A));

$date = new DateTime('2012-01-01 00:00:00 +24:00');
var_dump(IntlCalendar::fromDateTime($date));

$date = new DateTime('2012-01-01 00:00:00 WEST');
var_dump(IntlCalendar::fromDateTime($date));

var_dump(intlcal_from_date_time());
