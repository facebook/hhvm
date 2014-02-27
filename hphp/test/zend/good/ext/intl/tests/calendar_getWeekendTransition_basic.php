<?php
ini_set("intl.error_level", E_WARNING);
ini_set("intl.default_locale", "nl");

$intlcal = IntlCalendar::createInstance();
var_dump($intlcal->getWeekendTransition(IntlCalendar::DOW_SUNDAY));
var_dump(intlcal_get_weekend_transition($intlcal, IntlCalendar::DOW_SUNDAY));
?>
==DONE==