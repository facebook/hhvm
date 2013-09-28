<?php
ini_set("intl.error_level", E_WARNING);
ini_set("intl.default_locale", "pt_PT");

$bi = IntlBreakIterator::createWordInstance('pt');
$bi->setText('foo bar trans zoo bee');

var_dump($bi->isBoundary(0));
var_dump($bi->isBoundary(7));
var_dump($bi->isBoundary(-1));
var_dump($bi->isBoundary(1));
var_dump($bi->isBoundary(50));
?>
==DONE==