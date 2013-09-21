<?php
ini_set("intl.error_level", E_WARNING);

$tz = IntlTimeZone::createTimeZone('Europe/Lisbon');
var_dump($tz->getDisplayName(array()));
var_dump($tz->getDisplayName(false, array()));
var_dump($tz->getDisplayName(false, -1));
var_dump($tz->getDisplayName(false, IntlTimeZone::DISPLAY_SHORT, array()));
var_dump($tz->getDisplayName(false, IntlTimeZone::DISPLAY_SHORT, NULL, NULL));

var_dump(intltz_get_display_name(null, IntlTimeZone::DISPLAY_SHORT, false, 'pt_PT'));
