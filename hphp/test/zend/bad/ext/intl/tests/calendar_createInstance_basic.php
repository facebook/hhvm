<?php
ini_set("intl.error_level", E_WARNING);
ini_set("intl.default_locale", "nl");

date_default_timezone_set('Europe/Amsterdam');

$cal = IntlCalendar::createInstance();
print_R($cal->getTimeZone());
print_R($cal->getLocale(Locale::ACTUAL_LOCALE));
echo "\n";
print_R($cal->getType());
echo "\n";

$timeMillis = $cal->getTime();
$time = time();

var_dump(abs($timeMillis - $time * 1000) < 1000);

?>
==DONE==
