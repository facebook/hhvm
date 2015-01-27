<?php
ini_set("intl.error_level", E_WARNING);
ini_set("intl.default_locale", "pt_PT");
ini_set("date.timezone", "Europe/Lisbon");

$cal = IntlCalendar::fromDateTime('2012-01-01 00:00:00'); //Europe/Lisbon
echo IntlDateFormatter::formatObject($cal), "\n";
echo IntlDateFormatter::formatObject($cal, IntlDateFormatter::FULL), "\n";
echo IntlDateFormatter::formatObject($cal, null, "en-US"), "\n";
echo IntlDateFormatter::formatObject($cal, array(IntlDateFormatter::SHORT, IntlDateFormatter::FULL), "en-US"), "\n";
echo IntlDateFormatter::formatObject($cal, 'E y-MM-d HH,mm,ss.SSS v', "en-US"), "\n";

$cal = IntlCalendar::fromDateTime('2012-01-01 05:00:00+03:00');
echo datefmt_format_object($cal, IntlDateFormatter::FULL), "\n";

$cal = IntlCalendar::createInstance(null,'en-US@calendar=islamic-civil');
$cal->setTime(strtotime('2012-01-01 00:00:00')*1000.);
echo IntlDateFormatter::formatObject($cal), "\n";
echo IntlDateFormatter::formatObject($cal, IntlDateFormatter::FULL, "en-US"), "\n";

?>
==DONE==
