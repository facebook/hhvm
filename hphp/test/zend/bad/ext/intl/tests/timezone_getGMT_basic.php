<?php
ini_set("intl.error_level", E_WARNING);
$tz = IntlTimeZone::getGMT();
print_r($tz);
$tz = intltz_get_gmt();
print_r($tz);
?>
==DONE==