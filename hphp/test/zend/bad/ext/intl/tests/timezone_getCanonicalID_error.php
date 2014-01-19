<?php
ini_set("intl.error_level", E_WARNING);

var_dump(IntlTimeZone::getCanonicalID());
var_dump(IntlTimeZone::getCanonicalID(array()));
var_dump(IntlTimeZone::getCanonicalID("foo\x81"));
var_dump(IntlTimeZone::getCanonicalID('foobar', null));

