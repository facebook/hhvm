<?php
ini_set("intl.error_level", E_WARNING);

$tz = IntlTimeZone::createTimeZone('Etc/Unknown');

var_dump($tz->toDateTimeZone(''));
try {
	var_dump($tz->toDateTimeZone());
} catch (Exception $e) {
	var_dump($e->getMessage());
}

var_dump(intltz_to_date_time_zone());
var_dump(intltz_to_date_time_zone(1));
