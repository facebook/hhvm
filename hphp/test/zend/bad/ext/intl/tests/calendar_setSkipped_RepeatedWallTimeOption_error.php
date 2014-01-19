<?php
ini_set("intl.error_level", E_WARNING);

$c = new IntlGregorianCalendar(NULL, 'pt_PT');

var_dump($c->setSkippedWallTimeOption());
var_dump($c->setRepeatedWallTimeOption());

var_dump($c->setSkippedWallTimeOption(1, 2));
var_dump($c->setRepeatedWallTimeOption(1, 2));

var_dump($c->setSkippedWallTimeOption(array()));
var_dump($c->setRepeatedWallTimeOption(array()));

var_dump($c->setSkippedWallTimeOption(3));
var_dump($c->setRepeatedWallTimeOption(2));

var_dump(intlcal_set_skipped_wall_time_option($c));
var_dump(intlcal_set_repeated_wall_time_option($c));

var_dump(intlcal_set_repeated_wall_time_option(1, 1));
