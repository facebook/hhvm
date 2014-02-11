<?php
ini_set("intl.error_level", E_WARNING);
ini_set("intl.default_locale", "nl");

$intlcal = IntlCalendar::createInstance('UTC');
$intlcal->clear();
var_dump($intlcal->set(2012, 1, 29));
var_dump($intlcal->getTime(),
		strtotime('2012-02-29 00:00:00 +0000') * 1000.);
		
//two minutes to midnight!
var_dump($intlcal->set(2012, 1, 29, 23, 58));
var_dump($intlcal->getTime(),
		strtotime('2012-02-29 23:58:00 +0000') * 1000.);

var_dump($intlcal->set(2012, 1, 29, 23, 58, 31));
var_dump($intlcal->getTime(),
		strtotime('2012-02-29 23:58:31 +0000') * 1000.);

?>
==DONE==