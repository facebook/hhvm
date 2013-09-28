<?php
ini_set("intl.error_level", E_WARNING);

$tz = IntlTimeZone::createTimeZone('Europe/Lisbon');
var_dump($tz->getErrorCode(array()));

var_dump(intltz_get_error_code(null));
