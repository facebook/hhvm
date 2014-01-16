<?php
ini_set("intl.error_level", E_WARNING);

$c = new IntlGregorianCalendar(NULL, 'pt_PT');

var_dump($c->setMinimalDaysInFirstWeek());
var_dump($c->setMinimalDaysInFirstWeek(1, 2));
var_dump($c->setMinimalDaysInFirstWeek(0));

var_dump(intlcal_set_minimal_days_in_first_week($c, 0));
var_dump(intlcal_set_minimal_days_in_first_week(1, 2));
