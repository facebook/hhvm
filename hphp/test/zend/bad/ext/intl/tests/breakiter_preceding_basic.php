<?php
ini_set("intl.error_level", E_WARNING);
ini_set("intl.default_locale", "pt_PT");

$bi = IntlBreakIterator::createWordInstance('pt');
$bi->setText('foo bar trans zoo bee');

var_dump($bi->preceding(5));
var_dump($bi->preceding(50));
var_dump($bi->preceding(-1));
?>
==DONE==