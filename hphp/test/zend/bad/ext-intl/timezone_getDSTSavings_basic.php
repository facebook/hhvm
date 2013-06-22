<?php
ini_set("intl.error_level", E_WARNING);

$lsb = IntlTimeZone::createTimeZone('Europe/Lisbon');
var_dump($lsb->getDSTSavings());

var_dump(intltz_get_dst_savings($lsb));

?>
==DONE==