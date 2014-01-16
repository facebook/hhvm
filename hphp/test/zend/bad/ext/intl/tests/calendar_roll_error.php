<?php
ini_set("intl.error_level", E_WARNING);

$c = new IntlGregorianCalendar(NULL, 'pt_PT');

var_dump($c->roll(1, 2, 3));
var_dump($c->roll(-1, 2));
var_dump($c->roll(1));

var_dump(intlcal_roll($c, 1, 2, 3));
var_dump(intlcal_roll(1, 2, 3));