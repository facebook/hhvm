<?php
ini_set("intl.error_level", E_WARNING);

$c = new IntlGregorianCalendar(NULL, 'pt_PT');

function eh($errno, $errstr) {
echo "error: $errno, $errstr\n";
}
set_error_handler('eh');

var_dump(intlcal_get($c));
var_dump(intlcal_get_actual_maximum($c));
var_dump(intlcal_get_actual_minimum($c));

var_dump(intlcal_get($c, -1));
var_dump(intlcal_get_actual_maximum($c, -1));
var_dump(intlcal_get_actual_minimum($c, -1));

var_dump(intlcal_get($c, "s"));
var_dump(intlcal_get_actual_maximum($c, "s"));
var_dump(intlcal_get_actual_minimum($c, "s"));

var_dump(intlcal_get(1));
var_dump(intlcal_get_actual_maximum(1));
var_dump(intlcal_get_actual_minimum(1));