<?php
ini_set("intl.error_level", E_WARNING);

$c = new IntlGregorianCalendar(NULL, 'pt_PT');

function eh($errno, $errstr) {
echo "error: $errno, $errstr\n";
}
set_error_handler('eh');

var_dump($c->isEquivalentTo(0));
var_dump($c->isEquivalentTo($c, 1));
var_dump($c->isEquivalentTo(1));

var_dump(intlcal_is_equivalent_to($c));
var_dump(intlcal_is_equivalent_to($c, 1));
var_dump(intlcal_is_equivalent_to(1, $c));
