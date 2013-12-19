<?php
ini_set("intl.error_level", E_WARNING);

$c = new IntlGregorianCalendar(NULL, 'pt_PT');

var_dump($c->getLocale());
var_dump($c->getLocale(2));
var_dump($c->getLocale(2, 3));

var_dump(intlcal_get_locale($c));
var_dump(intlcal_get_locale(1));
