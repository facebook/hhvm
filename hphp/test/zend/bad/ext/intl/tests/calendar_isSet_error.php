<?php
ini_set("intl.error_level", E_WARNING);

$c = new IntlGregorianCalendar(NULL, 'pt_PT');

var_dump($c->isSet());
var_dump($c->isSet(1, 2));
var_dump($c->isSet(-1));

var_dump(intlcal_is_set($c));
var_dump(intlcal_is_set(1, 2));
