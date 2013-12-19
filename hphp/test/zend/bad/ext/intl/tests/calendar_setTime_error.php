<?php
ini_set("intl.error_level", E_WARNING);

$c = new IntlGregorianCalendar(NULL, 'pt_PT');

var_dump($c->setTime(1, 2));
var_dump($c->setTime("jjj"));

var_dump(intlcal_set_time($c, 1, 2));
var_dump(intlcal_set_time(1));