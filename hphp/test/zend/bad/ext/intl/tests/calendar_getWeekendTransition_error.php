<?php
ini_set("intl.error_level", E_WARNING);

$c = new IntlGregorianCalendar(NULL, 'pt_PT');

var_dump($c->getWeekendTransition());
var_dump($c->getWeekendTransition(1, 2));
var_dump($c->getWeekendTransition(0));

var_dump(intlcal_get_weekend_transition($c));
var_dump(intlcal_get_weekend_transition(1, 1));
