<?php
ini_set("intl.error_level", E_WARNING);

$c = new IntlGregorianCalendar(NULL, 'pt_PT');

var_dump($c->isLenient(1));

var_dump(intlcal_is_lenient($c, 1));
var_dump(intlcal_is_lenient(1));
