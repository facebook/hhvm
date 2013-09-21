<?php
ini_set("intl.error_level", E_WARNING);
ini_set("intl.default_locale", "pt_PT");
ini_set("date.timezone", 'Atlantic/Azores');

$df = new IntlDateFormatter('pt_PT', 0, 0, 'Europe/Minsk');

$df->setTimeZoneId('Europe/Madrid');

?>
==DONE==