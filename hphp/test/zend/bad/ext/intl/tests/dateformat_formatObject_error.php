<?php
ini_set("intl.error_level", E_WARNING);
ini_set("intl.default_locale", "pt_PT");
ini_set("date.timezone", "Europe/Lisbon");

var_dump(IntlDateFormatter::formatObject());
var_dump(IntlDateFormatter::formatObject(1));
var_dump(IntlDateFormatter::formatObject(new stdclass));

class A extends IntlCalendar {function __construct(){}}
var_dump(IntlDateFormatter::formatObject(new A));
class B extends DateTime {function __construct(){}}
var_dump(IntlDateFormatter::formatObject(new B));

$cal = IntlCalendar::createInstance();
var_dump(IntlDateFormatter::formatObject($cal, -2));
var_dump(IntlDateFormatter::formatObject($cal, array()));
var_dump(IntlDateFormatter::formatObject($cal, array(1,2,3)));
var_dump(IntlDateFormatter::formatObject($cal, array(array(), 1)));
var_dump(IntlDateFormatter::formatObject($cal, array(1, -2)));
var_dump(IntlDateFormatter::formatObject($cal, ""));
var_dump(IntlDateFormatter::formatObject($cal, "YYYY", array()));

?>
==DONE==
