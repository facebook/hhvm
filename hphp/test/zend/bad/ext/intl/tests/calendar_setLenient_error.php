<?php
ini_set("intl.error_level", E_WARNING);

$c = new IntlGregorianCalendar(NULL, 'pt_PT');

var_dump($c->setLenient());
var_dump($c->setLenient(array()));
var_dump($c->setLenient(1, 2));

var_dump(intlcal_set_lenient($c, array()));
var_dump(intlcal_set_lenient(1, false));
