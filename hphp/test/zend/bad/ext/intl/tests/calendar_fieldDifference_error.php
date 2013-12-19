<?php
ini_set("intl.error_level", E_WARNING);

$c = new IntlGregorianCalendar(NULL, 'pt_PT');

var_dump($c->fieldDifference($c, 2, 3));
var_dump($c->fieldDifference(INF, 2));
var_dump($c->fieldDifference(1));

var_dump(intlcal_field_difference($c, 0, 1, 2));
var_dump(intlcal_field_difference(1, 0, 1));
