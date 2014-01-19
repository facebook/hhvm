<?php
ini_set("intl.error_level", E_WARNING);

$c = new IntlGregorianCalendar(NULL, 'pt_PT');
var_dump($c->isLeapYear(2000, 2011));
var_dump($c->isLeapYear());
var_dump($c->isLeapYear("fgdf"));

var_dump(intlgregcal_is_leap_year($c, 1, 2));
var_dump(intlgregcal_is_leap_year($c));
var_dump(intlgregcal_is_leap_year(1, 2));