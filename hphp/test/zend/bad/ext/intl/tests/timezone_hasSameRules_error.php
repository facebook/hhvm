<?php
ini_set("intl.error_level", E_WARNING);

function error_handler($errno, $errstr, $errfile, $errline)
{
	var_dump($errno, $errstr);
	return true;
}
set_error_handler("error_handler");

$tz = IntlTimeZone::createTimeZone('Europe/Lisbon');
var_dump($tz->hasSameRules('foo'));

var_dump(intltz_has_same_rules(null, $tz));
