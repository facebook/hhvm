<?php
ini_set("intl.error_level", E_WARNING);
ini_set("intl.default_locale", "pt_PT");
ini_set("date.timezone", 'Atlantic/Azores');

$ts = strtotime('2012-01-01 00:00:00 UTC');

function d(IntlDateFormatter $df) {
global $ts;
echo $df->format($ts), "\n";
var_dump(
$df->getTimeZoneID(),
$df->getTimeZone()->getID());
echo "\n";
}

$df = new IntlDateFormatter('pt_PT', 0, 0, 'Europe/Minsk');
d($df);

$df->setTimeZone(NULL);
d($df);

$df->setTimeZone('Europe/Madrid');
d($df);

$df->setTimeZone(IntlTimeZone::createTimeZone('Europe/Paris'));
d($df);

$df->setTimeZone(new DateTimeZone('Europe/Amsterdam'));
d($df);

?>
==DONE==