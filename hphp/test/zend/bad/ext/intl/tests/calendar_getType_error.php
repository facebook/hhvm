<?php
ini_set("intl.error_level", E_WARNING);

$c = new IntlGregorianCalendar(NULL, 'pt_PT');

var_dump($c->getType(1));

var_dump(intlcal_get_type($c, 1));
var_dump(intlcal_get_type(1));
