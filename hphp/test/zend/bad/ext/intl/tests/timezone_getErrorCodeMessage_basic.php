<?php
ini_set("intl.error_level", E_WARNING);

$lsb = IntlTimeZone::createTimeZone('Europe/Lisbon');

var_dump($lsb->getErrorCode());
var_dump($lsb->getErrorMessage());

var_dump($lsb->getOffset(INF, 1, $a, $b));

var_dump($lsb->getErrorCode());
var_dump($lsb->getErrorMessage());

?>
==DONE==