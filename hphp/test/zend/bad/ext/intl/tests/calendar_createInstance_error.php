<?php
ini_set("intl.error_level", E_WARNING);

class X extends IntlTimeZone {
function __construct() {}
}

var_dump(IntlCalendar::createInstance(1, 2, 3));
var_dump(intlcal_create_instance(1, 2, 3));
var_dump(intlcal_create_instance(new X, NULL));
var_dump(intlcal_create_instance(NULL, array()));
