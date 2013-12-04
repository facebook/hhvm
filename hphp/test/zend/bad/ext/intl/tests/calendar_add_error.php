<?php
ini_set("intl.error_level", E_WARNING);

$c = new IntlGregorianCalendar(NULL, 'pt_PT');

var_dump($c->add(1, 2, 3));
var_dump($c->add(-1, 2));
var_dump($c->add(1));

var_dump(intlcal_add($c, 1, 2, 3));
var_dump(intlcal_add(1, 2, 3));