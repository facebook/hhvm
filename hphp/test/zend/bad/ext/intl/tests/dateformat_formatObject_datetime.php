<?php
ini_set("intl.error_level", E_WARNING);
ini_set("intl.default_locale", "pt_PT");
ini_set("date.timezone", "Europe/Lisbon");

$dt = new DateTime('2012-01-01 00:00:00'); //Europe/Lisbon
echo IntlDateFormatter::formatObject($dt), "\n";
echo IntlDateFormatter::formatObject($dt, IntlDateFormatter::FULL), "\n";
echo IntlDateFormatter::formatObject($dt, null, "en-US"), "\n";
echo IntlDateFormatter::formatObject($dt, array(IntlDateFormatter::SHORT, IntlDateFormatter::FULL), "en-US"), "\n";
echo IntlDateFormatter::formatObject($dt, 'E y-MM-d HH,mm,ss.SSS v', "en-US"), "\n";

$dt = new DateTime('2012-01-01 05:00:00+03:00');
echo IntlDateFormatter::formatObject($dt, IntlDateFormatter::FULL), "\n";

?>
==DONE==
