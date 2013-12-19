<?php
ini_set("intl.error_level", E_WARNING);

$c = new IntlGregorianCalendar(NULL, 'pt_PT');

var_dump($c->clear(1, 2));
var_dump($c->clear(-1));

var_dump(intlcal_clear($c, -1));
var_dump(intlcal_clear(1, 2));