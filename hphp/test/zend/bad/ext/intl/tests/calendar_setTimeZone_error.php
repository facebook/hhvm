<?php
ini_set("intl.error_level", E_WARNING);

$c = new IntlGregorianCalendar(NULL, 'pt_PT');

$gmt = IntlTimeZone::getGMT();

function eh($errno, $errstr) {
echo "error: $errno, $errstr\n";
}
set_error_handler('eh');

var_dump($c->setTimeZone($gmt, 2));
var_dump($c->setTimeZone());

var_dump(intlcal_set_time_zone($c, 1, 2));
var_dump(intlcal_set_time_zone(1, $gmt));
