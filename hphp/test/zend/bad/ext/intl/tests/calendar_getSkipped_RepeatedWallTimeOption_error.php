<?php
ini_set("intl.error_level", E_WARNING);

$c = new IntlGregorianCalendar(NULL, 'pt_PT');

var_dump($c->getSkippedWallTimeOption(1));
var_dump($c->getRepeatedWallTimeOption(1));

var_dump(intlcal_get_skipped_wall_time_option($c, 1));
var_dump(intlcal_get_repeated_wall_time_option($c, 1));

var_dump(intlcal_get_skipped_wall_time_option(1));
