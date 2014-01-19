<?php
ini_set("intl.error_level", E_WARNING);

$c = new IntlGregorianCalendar();
var_dump($c->setGregorianChange());
var_dump($c->setGregorianChange(1, 2));
var_dump($c->setGregorianChange("sdfds"));

var_dump(intlgregcal_set_gregorian_change($c));
var_dump(intlgregcal_set_gregorian_change(1, 4.));