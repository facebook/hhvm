<?php
ini_set("intl.error_level", E_WARNING);
ini_set("intl.default_locale", "nl");

date_default_timezone_set('Europe/Amsterdam');

$cal = intlcal_create_instance('Europe/Amsterdam');
print_R($cal->getTimeZone());
print_R($cal->getLocale(Locale::ACTUAL_LOCALE));
echo "\n";

$cal = intlcal_create_instance('Europe/Lisbon', null);
print_R($cal->getTimeZone());
print_R($cal->getLocale(Locale::ACTUAL_LOCALE));
echo "\n";

$cal = intlcal_create_instance(IntlTimeZone::createTimeZone('Europe/Lisbon'));
print_R($cal->getTimeZone());
print_R($cal->getLocale(Locale::ACTUAL_LOCALE));
echo "\n";

$cal = intlcal_create_instance(null, "pt");
print_R($cal->getTimeZone());
print_R($cal->getLocale(Locale::ACTUAL_LOCALE));
echo "\n";

$cal = intlcal_create_instance("Europe/Lisbon", "pt");
print_R($cal->getTimeZone());
print_R($cal->getLocale(Locale::ACTUAL_LOCALE));
echo "\n";

?>
==DONE==