<?php
ini_set("intl.error_level", E_WARNING);

$c = new IntlGregorianCalendar(NULL, 'pt_PT');

var_dump($c->getTime(1));

var_dump(intlcal_get_time($c, 1));
var_dump(intlcal_get_time(1));