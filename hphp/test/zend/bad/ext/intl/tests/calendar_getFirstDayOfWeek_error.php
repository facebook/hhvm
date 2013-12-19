<?php
ini_set("intl.error_level", E_WARNING);

$c = new IntlGregorianCalendar(NULL, 'pt_PT');

var_dump($c->getFirstDayOfWeek(1));

var_dump(intlcal_get_first_day_of_week($c, 1));
var_dump(intlcal_get_first_day_of_week(1));
