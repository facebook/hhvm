<?php
ini_set("intl.error_level", E_WARNING);

$c = new IntlGregorianCalendar(NULL, 'pt_PT');

var_dump($c->getLeastMaximum());
var_dump($c->getMaximum());
var_dump($c->getGreatestMinimum());
var_dump($c->getMinimum());

var_dump($c->getLeastMaximum(-1));
var_dump($c->getMaximum(-1));
var_dump($c->getGreatestMinimum(-1));
var_dump($c->getMinimum(-1));

var_dump(intlcal_get_least_maximum($c, -1));
var_dump(intlcal_get_maximum($c, -1));
var_dump(intlcal_get_greatest_minimum($c, -1));
var_dump(intlcal_get_minimum($c, -1));

function eh($errno, $errstr) {
echo "error: $errno, $errstr\n";
}
set_error_handler('eh');

var_dump(intlcal_get_least_maximum(1, 1));
var_dump(intlcal_get_maximum(1, 1));
var_dump(intlcal_get_greatest_minimum(1, -1));
var_dump(intlcal_get_minimum(1, -1));
