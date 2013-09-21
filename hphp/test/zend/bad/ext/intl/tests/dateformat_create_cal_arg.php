<?php
ini_set("intl.error_level", E_WARNING);
ini_set("intl.default_locale", "pt_PT");
ini_set("date.timezone", 'Atlantic/Azores');

$ts = strtotime('2012-01-01 00:00:00 UTC');

$cal = new IntlGregorianCalendar('UTC', NULL);
$df = new IntlDateFormatter('es_ES', 0, 0, NULL, $cal);
echo $df->format($ts), "\n";

$cal = IntlCalendar::createInstance('UTC', 'en@calendar=islamic');
$df = new IntlDateFormatter('es_ES', 0, 0, NULL, $cal);
echo $df->format($ts), "\n";

//override calendar's timezone
$cal = new IntlGregorianCalendar('UTC', NULL);
$df = new IntlDateFormatter('es_ES', 0, 0, 'Europe/Madrid', $cal);
echo $df->format($ts), "\n";

//default calendar is gregorian
$df = new IntlDateFormatter('es_ES@calendar=islamic', 0, 0);
echo $df->format($ts), "\n";

//try now with traditional
$df = new IntlDateFormatter('es_ES@calendar=islamic', 0, 0, NULL, IntlDateFormatter::TRADITIONAL);
echo $df->format($ts), "\n";

//the timezone can be overridden when not specifying a calendar
$df = new IntlDateFormatter('es_ES@calendar=islamic', 0, 0, 'UTC', IntlDateFormatter::TRADITIONAL);
echo $df->format($ts), "\n";

$df = new IntlDateFormatter('es_ES', 0, 0, 'UTC', 0);
echo $df->format($ts), "\n";

?>
==DONE==