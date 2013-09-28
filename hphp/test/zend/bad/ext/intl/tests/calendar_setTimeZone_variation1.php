<?php
ini_set("intl.error_level", E_WARNING);
ini_set("intl.default_locale", "nl");

$intlcal = IntlCalendar::createInstance('Europe/Amsterdam');
print_r($intlcal->getTimeZone()->getID());
echo "\n";
var_dump($intlcal->get(IntlCalendar::FIELD_ZONE_OFFSET));

/* passing NULL has no effect */
$intlcal->setTimeZone(null);
print_r($intlcal->getTimeZone()->getID());
echo "\n";
var_dump($intlcal->get(IntlCalendar::FIELD_ZONE_OFFSET));

?>
==DONE==