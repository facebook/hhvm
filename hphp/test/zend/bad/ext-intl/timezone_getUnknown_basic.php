<?php
ini_set("intl.error_level", E_WARNING);
ini_set("intl.default_locale", "nl");

$tz = IntlTimeZone::getUnknown();
print_r($tz);
$tz = intltz_get_unknown();
print_r($tz);
?>
==DONE==