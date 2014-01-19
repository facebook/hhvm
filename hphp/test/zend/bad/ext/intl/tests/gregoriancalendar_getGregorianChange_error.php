<?php
ini_set("intl.error_level", E_WARNING);

$c = new IntlGregorianCalendar(NULL, 'pt_PT');
var_dump($c->getGregorianChange(1));

var_dump(intlgregcal_get_gregorian_change($c, 1));
var_dump(intlgregcal_get_gregorian_change(1));