<?php
ini_set("intl.error_level", E_WARNING);

$bi = new IntlRuleBasedBreakIterator('[\p{Letter}\uFFFD]+;[:number:]+;');
$bi->setText("\x80sdfé\x90d888 dfsa9");

var_dump($bi->getLocale(1, 2));
var_dump($bi->getLocale(array()));
var_dump($bi->getLocale());
