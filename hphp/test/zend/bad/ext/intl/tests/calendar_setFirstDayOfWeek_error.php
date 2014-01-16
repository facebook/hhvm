<?php
ini_set("intl.error_level", E_WARNING);

$c = new IntlGregorianCalendar(NULL, 'pt_PT');

var_dump($c->setFirstDayOfWeek());
var_dump($c->setFirstDayOfWeek(1, 2));
var_dump($c->setFirstDayOfWeek(0));

var_dump(intlcal_set_first_day_of_week($c, 0));
var_dump(intlcal_set_first_day_of_week(1, 2));
