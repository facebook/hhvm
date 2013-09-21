<?php
ini_set("intl.error_level", E_WARNING);

$tz = IntlTimeZone::createTimeZone('Europe/Lisbon');
var_dump($tz->getErrorMessage(array()));

var_dump(intltz_get_error_message(null));
