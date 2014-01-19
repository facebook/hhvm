<?php
ini_set("intl.error_level", E_WARNING);

$fmt1 = new IntlDateFormatter('en_US',
	IntlDateFormatter::FULL,
	IntlDateFormatter::FULL,
	'GMT+05:12',
	IntlDateFormatter::TRADITIONAL);
$fmt2 = new IntlDateFormatter('en_US',
	IntlDateFormatter::FULL,
	IntlDateFormatter::FULL,
	'GMT+05:12',
	IntlDateFormatter::GREGORIAN);
$fmt3 = new IntlDateFormatter('en_US@calendar=hebrew',
	IntlDateFormatter::FULL,
	IntlDateFormatter::FULL,
	'GMT+05:12',
	IntlDateFormatter::TRADITIONAL);
var_dump($fmt1->format(strtotime('2012-01-01 00:00:00 +0000')));
var_dump($fmt2->format(strtotime('2012-01-01 00:00:00 +0000')));
var_dump($fmt3->format(strtotime('2012-01-01 00:00:00 +0000')));

new IntlDateFormatter('en_US@calendar=hebrew',
	IntlDateFormatter::FULL,
	IntlDateFormatter::FULL,
	'GMT+05:12',
	-1);
?>
==DONE==