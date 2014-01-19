<?php
ini_set("intl.error_level", E_WARNING);

$c = new IntlGregorianCalendar(NULL, 'pt_PT');

var_dump($c->isWeekend(1, 2));
var_dump($c->isWeekend("jhhk"));

var_dump(intlcal_is_weekend($c, "jj"));
var_dump(intlcal_is_weekend(1));
