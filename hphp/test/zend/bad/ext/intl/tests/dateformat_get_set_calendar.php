<?php
ini_set("intl.error_level", E_WARNING);
ini_set("intl.default_locale", "pt_PT");
ini_set("date.timezone", 'Atlantic/Azores');

$ts = strtotime('2012-01-01 00:00:00 UTC');

function d(IntlDateFormatter $df) {
global $ts;
echo $df->format($ts), "\n";
var_dump($df->getCalendar(),
$df->getCalendarObject()->getType(),
$df->getCalendarObject()->getTimeZone()->getId());
echo "\n";
}

$df = new IntlDateFormatter('fr@calendar=islamic', 0, 0, 'Europe/Minsk');
d($df);


//changing the calendar with a cal type should not change tz
$df->setCalendar(IntlDateFormatter::TRADITIONAL);
d($df);

//but changing with an actual calendar should
$cal = IntlCalendar::createInstance("UTC");
$df->setCalendar($cal);
d($df);

?>
==DONE==