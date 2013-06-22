<?php
ini_set("intl.error_level", E_WARNING);
$tz = IntlTimeZone::createDefault();
print_r($tz);
$tz = intltz_create_default();
print_r($tz);
?>
==DONE==