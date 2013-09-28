<?php
ini_set("intl.error_level", E_WARNING);

$tz = IntlTimeZone::createTimeZone('Europe/Lisbon');
var_dump($tz->getOffset(INF, true, $a, $a));
var_dump($tz->getOffset(time()*1000, true, $a));
var_dump($tz->getOffset(time()*1000, true, $a, $a, $a));

intltz_get_offset(null, time()*1000, false, $a, $a);
