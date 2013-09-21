<?php
ini_set("intl.error_level", E_WARNING);
ini_set("intl.default_locale", "pt_PT");
ini_set("date.timezone", 'Atlantic/Azores');

$df = new IntlDateFormatter(NULL, 0, 0);

var_dump($df->getTimeZone(9));
var_dump(datefmt_get_timezone($df, 9));
var_dump(datefmt_get_timezone($df, 9));
var_dump(datefmt_get_timezone(new stdclass));

?>
==DONE==