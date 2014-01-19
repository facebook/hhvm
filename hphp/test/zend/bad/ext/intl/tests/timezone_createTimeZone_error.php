<?php
ini_set("intl.error_level", E_WARNING);

var_dump(IntlTimeZone::createTimeZone());
var_dump(IntlTimeZone::createTimeZone(new stdClass));
var_dump(IntlTimeZone::createTimeZone("foo bar", 4));
var_dump(IntlTimeZone::createTimeZone("foo\x80"));
