<?php
ini_set("intl.error_level", E_WARNING);
ini_set("intl.default_locale", "pt_PT");
ini_set("date.timezone", 'Atlantic/Azores');

$df = new IntlDateFormatter(NULL, 0, 0);

var_dump($df->setTimeZone());
var_dump(datefmt_set_timezone());
var_dump($df->setTimeZone(array()));
var_dump($df->setTimeZone(1, 2));
var_dump($df->setTimeZone('non existing timezone'));
var_dump(datefmt_set_timezone(new stdclass, 'UTC'));

?>
==DONE==