<?php
ini_set("intl.error_level", E_WARNING);

$tz = IntlTimeZone::createTimeZone('Europe/Lisbon');
var_dump($tz->getDSTSavings(array()));

var_dump(intltz_get_dst_savings(null));
