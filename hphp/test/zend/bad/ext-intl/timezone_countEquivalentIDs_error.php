<?php
ini_set("intl.error_level", E_WARNING);

var_dump(IntlTimeZone::countEquivalentIDs());
var_dump(IntlTimeZone::countEquivalentIDs(array()));
var_dump(IntlTimeZone::countEquivalentIDs("foo\x80"));
var_dump(IntlTimeZone::countEquivalentIDs("foo bar", 7));

