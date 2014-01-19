<?php
ini_set("intl.error_level", E_WARNING);

$c = new IntlGregorianCalendar(NULL, 'pt_PT');

var_dump($c->getDayOfWeekType(1, 2));
var_dump($c->getDayOfWeekType(0));
var_dump($c->getDayOfWeekType());

var_dump(intlcal_get_day_of_week_type($c, "foo"));
var_dump(intlcal_get_day_of_week_type(1, 1));
