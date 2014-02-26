<?php
ini_set("intl.error_level", E_WARNING);
$tz = IntlTimeZone::createEnumeration();
var_dump(get_class($tz));
$count = count(iterator_to_array($tz));
var_dump($count > 300);

$tz = intltz_create_enumeration();
var_dump(get_class($tz));
$count2 = count(iterator_to_array($tz));
var_dump($count == $count2);
?>
==DONE==