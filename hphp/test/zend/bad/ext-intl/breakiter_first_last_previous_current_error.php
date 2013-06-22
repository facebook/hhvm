<?php
ini_set("intl.error_level", E_WARNING);

$bi = new IntlRuleBasedBreakIterator('[\p{Letter}\uFFFD]+;[:number:]+;');
$bi->setText("\x80sdfé\x90d888 dfsa9");

var_dump($bi->first(1));
var_dump($bi->last(1));
var_dump($bi->previous(1));
var_dump($bi->current(1));
