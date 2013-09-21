<?php
ini_set("intl.error_level", E_WARNING);

class A {
function __tostring() { return 'aaa'; }
}

$bi = IntlBreakIterator::createWordInstance('pt');
var_dump($bi->setText('foo bar'));
var_dump($bi->getText());
var_dump($bi->setText(1));
var_dump($bi->getText());
var_dump($bi->setText(new A));
var_dump($bi->getText());

/* setText resets the pointer */
var_dump($bi->next());
var_dump($bi->setText('foo bar'));
var_dump($bi->current());