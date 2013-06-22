<?php
ini_set("intl.error_level", E_WARNING);
ini_set("intl.default_locale", "nl");
date_default_timezone_set('Europe/Amsterdam');

$intlcal = new IntlGregorianCalendar();

$pstdate = new DateTime('2012-01-01 00:00:00 WEST');
$intlcal->setTimeZone($pstdate->getTimeZone());
var_dump($intlcal->getTimeZone()->getID());

$pstdate = new DateTime('2012-01-01 00:00:00 +24:00');
$intlcal->setTimeZone($pstdate->getTimeZone());
var_dump($intlcal->getTimeZone()->getID());
