<?php
ini_set("intl.error_level", E_WARNING);

var_dump(IntlTimeZone::getRegion());
var_dump(IntlTimeZone::getRegion(array()));
var_dump(IntlTimeZone::getRegion('Europe/Lisbon', 4));
var_dump(IntlTimeZone::getRegion("foo\x81"));
var_dump(IntlTimeZone::getRegion("foo"));


