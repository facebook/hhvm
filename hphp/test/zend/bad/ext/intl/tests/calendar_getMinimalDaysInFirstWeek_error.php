<?php
ini_set("intl.error_level", E_WARNING);

$c = new IntlGregorianCalendar(NULL, 'pt_PT');

var_dump($c->getMinimalDaysInFirstWeek(1));

var_dump(intlcal_get_minimal_days_in_first_week($c, 1));
var_dump(intlcal_get_minimal_days_in_first_week(1));
