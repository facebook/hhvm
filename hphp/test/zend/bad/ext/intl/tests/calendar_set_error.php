<?php
ini_set("intl.error_level", E_WARNING);

$c = new IntlGregorianCalendar(NULL, 'pt_PT');

var_dump($c->set(1));
var_dump($c->set(1, 2, 3, 4));
var_dump($c->set(1, 2, 3, 4, 5, 6, 7));
var_dump($c->set(-1, 2));

var_dump(intlcal_set($c, -1, 2));
var_dump(intlcal_set(1, 2, 3));