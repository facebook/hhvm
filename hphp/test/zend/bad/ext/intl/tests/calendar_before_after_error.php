<?php
ini_set("intl.error_level", E_WARNING);

$c = new IntlGregorianCalendar(NULL, 'pt_PT');

function eh($errno, $errstr) {
echo "error: $errno, $errstr\n";
}
set_error_handler('eh');

var_dump($c->after());
var_dump($c->before());

var_dump($c->after(1));
var_dump($c->before(1));

var_dump($c->after($c, 1));
var_dump($c->before($c, 1));

var_dump(intlcal_after($c));
var_dump(intlcal_before($c));