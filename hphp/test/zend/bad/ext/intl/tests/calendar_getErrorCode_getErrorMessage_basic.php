<?php
ini_set("intl.error_level", E_WARNING);
ini_set("intl.default_locale", "nl");

$intlcal = new IntlGregorianCalendar(2012, 1, 29);
var_dump(
		$intlcal->getErrorCode(),
		intlcal_get_error_code($intlcal),
		$intlcal->getErrorMessage(),
		intlcal_get_error_message($intlcal)
);
$intlcal->add(IntlCalendar::FIELD_SECOND, 2147483647);
$intlcal->fieldDifference(-PHP_INT_MAX, IntlCalendar::FIELD_SECOND);

var_dump(
		$intlcal->getErrorCode(),
		intlcal_get_error_code($intlcal),
		$intlcal->getErrorMessage(),
		intlcal_get_error_message($intlcal)
);
?>
==DONE==