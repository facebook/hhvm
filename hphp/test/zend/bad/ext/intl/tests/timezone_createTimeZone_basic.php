<?php
ini_set("intl.error_level", E_WARNING);
$tz = IntlTimeZone::createTimeZone('GMT+01:00');
print_r($tz);
$tz = intltz_create_time_zone('GMT+01:00');
print_r($tz);
?>
==DONE==