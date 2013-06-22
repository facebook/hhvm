<?php
ini_set("intl.error_level", E_WARNING);
ini_set('date.timezone', 'Europe/Lisbon');

$cal = new IntlGregorianCalendar();
var_dump($cal->toDateTime(3));

var_dump(intlcal_to_date_time($cal, 3));

$cal = new IntlGregorianCalendar("Etc/Unknown");
try {
var_dump($cal->toDateTime());
} catch (Exception $e) {
var_dump("exception: {$e->getMessage()}");
}

var_dump(intlcal_to_date_time(3));
