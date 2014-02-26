<?php
ini_set("intl.error_level", E_WARNING);
ini_set("intl.default_locale", "nl");

$intlcal1 = IntlCalendar::createInstance('Europe/Amsterdam');
$intlcal2 = IntlCalendar::createInstance('Europe/Lisbon');
$intlcal3 = IntlCalendar::createInstance('Europe/Amsterdam', "nl_NL@calendar=islamic");
$intlcal4 = IntlCalendar::createInstance('Europe/Amsterdam');
$intlcal4->roll(IntlCalendar::FIELD_MONTH, true);

var_dump(
		"1 - 1",
		$intlcal1->isEquivalentTo($intlcal1),
		"1 - 2",
		$intlcal1->isEquivalentTo($intlcal2),
		"1 - 3",
		$intlcal1->isEquivalentTo($intlcal3),
		"1 - 4",
		$intlcal1->isEquivalentTo($intlcal4)
);

?>
==DONE==