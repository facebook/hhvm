<?php
ini_set("intl.error_level", E_WARNING);

$bi = new IntlRuleBasedBreakIterator('[\p{Letter}\uFFFD]+;[:number:]+;');
$bi->setText("\x80sdfé\x90d888 dfsa9");

var_dump($bi->following(1, 2));
var_dump($bi->following(array()));
var_dump($bi->preceding(1, 2));
var_dump($bi->preceding(array()));
var_dump($bi->isBoundary(1, 2));
var_dump($bi->isBoundary(array()));
