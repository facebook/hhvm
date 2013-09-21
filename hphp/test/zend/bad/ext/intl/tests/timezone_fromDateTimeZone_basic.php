<?php
ini_set("intl.error_level", E_WARNING);
ini_set("intl.default_locale", "nl");
date_default_timezone_set('Europe/Lisbon');

$tz = IntlTimeZone::fromDateTimeZone(new DateTimeZone('Europe/Amsterdam'));
var_dump($tz->getID(), $tz->getRawOffset());


$dt = new DateTime('2012-01-01 00:00:00 CET');
$dtz = $dt->getTimeZone();
/* this is different from new DateTimeZone('CET'),
 * which gives a Europe/Berlin timezone */
var_dump($dtz->getName());
$tz = IntlTimeZone::fromDateTimeZone($dtz);
var_dump($tz->getID(), $tz->getRawOffset());


$dt = new DateTime('2012-01-01 00:00:00 +0340');
$dtz = $dt->getTimeZone();
/* I don't think this timezone can be generated without a DateTime object */
var_dump($dtz->getName());
$tz = IntlTimeZone::fromDateTimeZone($dtz);
var_dump($tz->getID(), $tz->getRawOffset() /* (3*60+40)*60000 */);
