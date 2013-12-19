<?php
ini_set("intl.error_level", E_WARNING);

$c = new IntlGregorianCalendar(NULL, 'pt_PT');

var_dump($c->inDaylightTime(1));

var_dump(intlcal_in_daylight_time($c, 1));
var_dump(intlcal_in_daylight_time(1));
