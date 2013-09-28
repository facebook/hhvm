<?php
ini_set("intl.error_level", E_WARNING);

var_dump(IntlTimeZone::getEquivalentID('foo'));
var_dump(IntlTimeZone::getEquivalentID('foo', 'bar'));
var_dump(IntlTimeZone::getEquivalentID('Europe/Lisbon', 0, 1));
var_dump(IntlTimeZone::getEquivalentID("foo\x80", 0));
