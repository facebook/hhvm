
<?php
ini_set("intl.error_level", E_WARNING);
ini_set("intl.default_locale", "nl");

date_default_timezone_set('Europe/Amsterdam');

//25 March 2012, transition to DST
$intlcal = new IntlGregorianCalendar(2012, 2, 25, 0, 0, 0);
var_dump($intlcal->getSkippedWallTimeOption());
$intlcal->set(IntlCalendar::FIELD_HOUR_OF_DAY, 2);
$intlcal->set(IntlCalendar::FIELD_MINUTE, 30);
echo "Should be 3h30\n";
var_dump(
	$intlcal->get(IntlCalendar::FIELD_HOUR_OF_DAY),
	$intlcal->get(IntlCalendar::FIELD_MINUTE)
);

var_dump($intlcal->setSkippedWallTimeOption(IntlCalendar::WALLTIME_FIRST));
var_dump(intlcal_get_skipped_wall_time_option($intlcal));
$intlcal->set(IntlCalendar::FIELD_HOUR_OF_DAY, 2);
$intlcal->set(IntlCalendar::FIELD_MINUTE, 30);
echo "Should be 1h30\n";
var_dump(
	$intlcal->get(IntlCalendar::FIELD_HOUR_OF_DAY),
	$intlcal->get(IntlCalendar::FIELD_MINUTE)
);

var_dump(intlcal_set_skipped_wall_time_option($intlcal, IntlCalendar::WALLTIME_NEXT_VALID));
var_dump($intlcal->getSkippedWallTimeOption());
$intlcal->set(IntlCalendar::FIELD_HOUR_OF_DAY, 2);
$intlcal->set(IntlCalendar::FIELD_MINUTE, 30);
echo "Should be 3h00\n";
var_dump(
	$intlcal->get(IntlCalendar::FIELD_HOUR_OF_DAY),
	$intlcal->get(IntlCalendar::FIELD_MINUTE)
);


?>
==DONE==