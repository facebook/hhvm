<?php
ini_set('intl.error_level', E_WARNING);
var_dump(
	datefmt_create('', IntlDateFormatter::NONE, IntlDateFormatter::NONE, "\xFF",
		IntlDateFormatter::GREGORIAN, 'a'));
var_dump(
	new IntlDateFormatter('', IntlDateFormatter::NONE, IntlDateFormatter::NONE, "Europe/Lisbon",
		IntlDateFormatter::GREGORIAN, "\x80"));