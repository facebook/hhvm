<?php
ini_set("intl.error_level", E_WARNING);

$tz = IntlTimeZone::createTimeZone('Europe/Lisbon');
var_dump($tz->getRawOffset('foo'));

intltz_get_raw_offset(null);
