<?php
ini_set("intl.error_level", E_WARNING);

$tz = IntlTimeZone::createTimeZone('Europe/Lisbon');
var_dump($tz->useDaylightTime('foo'));
intltz_use_daylight_time(null);
