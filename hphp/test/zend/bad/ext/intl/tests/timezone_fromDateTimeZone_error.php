<?php
ini_set("intl.error_level", E_WARNING);

var_dump(IntlTimeZone::fromDateTimeZone());
var_dump(IntlTimeZone::fromDateTimeZone(1,2));
var_dump(IntlTimeZone::fromDateTimeZone('sdfds'));
var_dump(IntlTimeZone::fromDateTimeZone(new stdclass));
$dt = new DateTime('2012-08-01 00:00:00 WEST');
var_dump(IntlTimeZone::fromDateTimeZone($dt->getTimeZone()));

var_dump(intltz_from_date_time_zone());
