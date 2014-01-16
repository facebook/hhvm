<?php
ini_set("intl.error_level", E_WARNING);

$c = new IntlGregorianCalendar(NULL, 'pt_PT');

function eh($errno, $errstr) {
echo "error: $errno, $errstr\n";
}
set_error_handler('eh');

var_dump($c->equals());
var_dump($c->equals(new stdclass));
var_dump($c->equals(1, 2));

var_dump(intlcal_equals($c, array()));
var_dump(intlcal_equals(1, $c));
