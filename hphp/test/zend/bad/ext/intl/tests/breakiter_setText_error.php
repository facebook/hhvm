<?php
ini_set("intl.error_level", E_WARNING);

$bi = new IntlRuleBasedBreakIterator('[\p{Letter}]+;');
var_dump($bi->setText());
var_dump($bi->setText(array()));
var_dump($bi->setText(1,2));

class A {
function __destruct() { var_dump('destructed'); throw new Exception('e'); }
function __tostring() { return 'foo'; }
}

try {
var_dump($bi->setText(new A));
} catch (Exception $e) {
var_dump($e->getMessage());
}
