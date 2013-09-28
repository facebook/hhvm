<?php
ini_set("intl.error_level", E_WARNING);

$tz1 = IntlTimeZone::createTimeZone('Europe/Amsterdam');
print_r($tz1);
print_r(clone $tz1);

//clone non-owned object
$gmt = IntlTimeZone::getGMT();
print_r($gmt);
print_r(clone $gmt);

?>
==DONE==